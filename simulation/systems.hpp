#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "creatures/creature.hpp"
#include "environment/world.hpp"

namespace evo::sim {

struct SimulationEnvironment {
  std::vector<InventoryItem> recycled_materials;
  EnvironmentWorld world;
};

struct DeterministicIds {
  std::uint64_t next_creature_id {1};
  std::uint64_t next_genome_id {1};
  std::uint64_t next_gene_id {1};
  std::uint64_t next_block_id {1};
};

struct TickContext {
  std::uint64_t tick_index {0};
  std::uint32_t deterministic_seed {0};
};

Creature reproduce_asexual(const Creature& parent, DeterministicIds& ids, std::mt19937& rng,
                           double mutation_rate);

Creature reproduce_sexual(const Creature& a, const Creature& b, DeterministicIds& ids, std::mt19937& rng,
                         double mutation_rate);

void run_tick(std::vector<Creature>& creatures, SimulationEnvironment& environment,
              const TickContext& context);

} // namespace evo::sim
