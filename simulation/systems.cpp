#include "systems.hpp"

#include <algorithm>
#include <cmath>

namespace evo::sim {
namespace {

void mutate_gene(Gene& gene, std::mt19937& rng, const double mutation_rate) {
  std::uniform_real_distribution<double> p(0.0, 1.0);
  std::normal_distribution<double> delta(0.0, 0.1);
  if (p(rng) < mutation_rate) {
    gene.priority = static_cast<std::uint32_t>(std::max(0.0, static_cast<double>(gene.priority) + delta(rng) * 10.0));
  }
  if (p(rng) < mutation_rate) {
    gene.activation_conditions.min_energy = std::max(0.0, gene.activation_conditions.min_energy + delta(rng));
  }
  if (p(rng) < mutation_rate) {
    gene.activation_conditions.max_damage = std::clamp(gene.activation_conditions.max_damage + delta(rng), 0.0, 1.0);
  }
}

void mutate_block(Block& block, std::mt19937& rng, const double mutation_rate) {
  std::uniform_real_distribution<double> p(0.0, 1.0);
  std::normal_distribution<double> delta(0.0, 0.2);
  if (p(rng) < mutation_rate) {
    block.mass = std::max(0.1, block.mass + delta(rng));
  }
  if (p(rng) < mutation_rate) {
    block.action_energy_cost = std::max(0.0, block.action_energy_cost + delta(rng));
  }
}

Creature copy_creature(const Creature& src, DeterministicIds& ids) {
  Creature child = src;
  child.id = ids.next_creature_id++;
  child.genome.id = ids.next_genome_id++;
  child.age_ticks = 0;
  child.history.events.clear();
  child.history.events.push_back("born");

  for (auto& gene : child.genome.genes) {
    gene.id = ids.next_gene_id++;
  }
  for (auto& block : child.blocks) {
    block.id = ids.next_block_id++;
    block.damage_state = DamageState {};
  }
  return child;
}

} // namespace

Creature reproduce_asexual(const Creature& parent, DeterministicIds& ids, std::mt19937& rng,
                           const double mutation_rate) {
  Creature child = copy_creature(parent, ids);
  for (auto& gene : child.genome.genes) {
    mutate_gene(gene, rng, mutation_rate);
  }
  for (auto& block : child.blocks) {
    mutate_block(block, rng, mutation_rate);
  }
  child.history.events.push_back("asexual_reproduction");
  return child;
}

Creature reproduce_sexual(const Creature& a, const Creature& b, DeterministicIds& ids, std::mt19937& rng,
                         const double mutation_rate) {
  Creature child = copy_creature(a, ids);
  child.genome.genes.clear();
  child.blocks.clear();

  std::vector<Gene> combined = a.genome.genes;
  combined.insert(combined.end(), b.genome.genes.begin(), b.genome.genes.end());
  std::sort(combined.begin(), combined.end(), [](const Gene& lhs, const Gene& rhs) {
    if (lhs.build_order == rhs.build_order) {
      return lhs.id < rhs.id;
    }
    return lhs.build_order < rhs.build_order;
  });

  for (std::size_t i = 0; i < combined.size(); ++i) {
    if (i % 2 == 0) {
      Gene g = combined[i];
      g.id = ids.next_gene_id++;
      mutate_gene(g, rng, mutation_rate);
      child.genome.genes.push_back(g);

      Block block;
      block.id = ids.next_block_id++;
      block.type = g.block_type;
      block.composition = g.composition;
      block.mass = 1.0;
      block.durability = 100.0;
      mutate_block(block, rng, mutation_rate);
      child.blocks.push_back(block);
    }
  }

  child.history.events.push_back("sexual_reproduction");
  return child;
}

void run_tick(std::vector<Creature>& creatures, SimulationEnvironment& environment,
              const TickContext& context) {
  std::sort(creatures.begin(), creatures.end(), [](const Creature& lhs, const Creature& rhs) {
    return lhs.id < rhs.id;
  });

  for (auto& creature : creatures) {
    creature.age_ticks += 1;
    auto ordered = creature.sorted_blocks();
    for (Block* block : ordered) {
      const double upkeep = block->maintenance_energy_cost + block->action_energy_cost;
      if (creature.energy_pool >= upkeep) {
        creature.energy_pool -= upkeep;
        block->damage_state.wear = std::max(0.0, block->damage_state.wear - block->repair_energy_cost * 0.1);
      } else {
        block->damage_state.wear += (upkeep - creature.energy_pool) + static_cast<double>(context.tick_index % 3);
        creature.energy_pool = 0.0;
      }

      if (block->damage_state.wear >= block->durability) {
        block->damage_state.broken = true;
        for (const auto& component : block->composition.components) {
          environment.recycled_materials.push_back(InventoryItem {component.element_id, component.weight * block->mass});
        }
      }
    }

    creature.blocks.erase(
      std::remove_if(creature.blocks.begin(), creature.blocks.end(), [](const Block& b) { return b.damage_state.broken; }),
      creature.blocks.end());
  }
}

} // namespace evo::sim
