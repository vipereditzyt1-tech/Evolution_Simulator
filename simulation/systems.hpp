#pragma once

#include <algorithm>
#include <cstdint>
#include <random>
#include <vector>

#include "creatures/creature.hpp"
#include "job_system.hpp"
#include "environment/world.hpp"

namespace evo::sim {

struct SimulationEnvironment {
  std::vector<InventoryItem> recycled_materials;
  EnvironmentWorld world;
  ThreadPool jobs;

  SimulationEnvironment() : world(1), jobs() {}
  explicit SimulationEnvironment(std::uint32_t seed) : world(seed), jobs() {}
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
  double dt_seconds {1.0 / 60.0};
};

struct FixedTimestepRunner {
  double fixed_dt {1.0 / 60.0};
  double accumulator {0.0};

  template <typename Fn>
  std::size_t consume(double frame_dt, Fn&& tick_fn) {
    accumulator += frame_dt;
    std::size_t ticks = 0;
    while (accumulator >= fixed_dt) {
      tick_fn(fixed_dt);
      accumulator -= fixed_dt;
      ++ticks;
    }
    return ticks;
  }

  [[nodiscard]] double interpolation_alpha() const {
    return fixed_dt <= 0.0 ? 0.0 : std::clamp(accumulator / fixed_dt, 0.0, 1.0);
  }
};

Creature reproduce_asexual(const Creature& parent, DeterministicIds& ids, std::mt19937& rng,
                           double mutation_rate);

Creature reproduce_sexual(const Creature& a, const Creature& b, DeterministicIds& ids, std::mt19937& rng,
                         double mutation_rate);

void run_tick(std::vector<Creature>& creatures, SimulationEnvironment& environment,
              const TickContext& context);

} // namespace evo::sim
