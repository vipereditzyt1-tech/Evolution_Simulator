#include <gtest/gtest.h>

#include <random>

#include "simulation/systems.hpp"

namespace {

evo::sim::Creature make_creature(std::uint64_t id, evo::sim::ElementId element) {
  evo::sim::Creature creature;
  creature.id = id;
  creature.genome.id = id;
  creature.energy_pool = 5.0;
  creature.position = evo::sim::SpatialPosition {static_cast<double>(id), 0.0, 0.0};

  evo::sim::Block block;
  block.id = id;
  block.mass = 2.0;
  block.durability = 3.0;
  block.maintenance_energy_cost = 2.0;
  block.action_energy_cost = 2.0;
  block.repair_energy_cost = 0.1;
  block.composition.components = {{element, 1.0}};
  block.composition.normalize();
  creature.blocks.push_back(block);

  evo::sim::Gene gene;
  gene.id = id;
  gene.block_type = evo::sim::BlockType::Structural;
  gene.composition = block.composition;
  creature.genome.genes.push_back(gene);
  return creature;
}

} // namespace

TEST(IntegrationEcosystem, ReproductionAndResourceLoop) {
  evo::sim::SimulationEnvironment env(33);
  std::vector<evo::sim::Creature> creatures {make_creature(1, 8), make_creature(2, 26)};

  evo::sim::DeterministicIds ids {.next_creature_id = 10, .next_genome_id = 20, .next_gene_id = 30, .next_block_id = 40};
  std::mt19937 rng(7);
  creatures.push_back(evo::sim::reproduce_sexual(creatures[0], creatures[1], ids, rng, 0.05));

  for (std::uint64_t t = 0; t < 8; ++t) {
    evo::sim::run_tick(creatures, env, evo::sim::TickContext {.tick_index = t, .deterministic_seed = 33});
  }

  EXPECT_FALSE(env.recycled_materials.empty());
  const auto neighbors = env.world.query_neighbors(creatures.front().position, 8.0);
  EXPECT_FALSE(neighbors.empty());
}
