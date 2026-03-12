#include <gtest/gtest.h>

#include <vector>

#include "simulation/systems.hpp"

namespace {

evo::sim::Creature seed_creature(std::uint64_t id) {
  evo::sim::Creature c;
  c.id = id;
  c.genome.id = id;
  c.energy_pool = 20.0;
  c.position = evo::sim::SpatialPosition {static_cast<double>(id), 1.0, 2.0};
  evo::sim::Block block;
  block.id = id;
  block.durability = 6.0;
  block.maintenance_energy_cost = 1.0;
  block.action_energy_cost = 1.0;
  block.composition.components = {{8, 1.0}};
  block.composition.normalize();
  c.blocks.push_back(block);
  return c;
}

std::vector<evo::sim::Creature> run_sim(std::uint32_t seed, int ticks) {
  evo::sim::SimulationEnvironment env(seed);
  std::vector<evo::sim::Creature> creatures {seed_creature(1), seed_creature(2), seed_creature(3)};
  for (int t = 0; t < ticks; ++t) {
    evo::sim::run_tick(creatures, env, evo::sim::TickContext {.tick_index = static_cast<std::uint64_t>(t), .deterministic_seed = seed});
  }
  return creatures;
}

} // namespace

TEST(DeterministicReplay, FixedSeedProducesStableResults) {
  const auto a = run_sim(99, 20);
  const auto b = run_sim(99, 20);

  ASSERT_EQ(a.size(), b.size());
  for (std::size_t i = 0; i < a.size(); ++i) {
    EXPECT_EQ(a[i].id, b[i].id);
    EXPECT_NEAR(a[i].energy_pool, b[i].energy_pool, 1e-9);
    EXPECT_EQ(a[i].age_ticks, b[i].age_ticks);
    EXPECT_EQ(a[i].blocks.size(), b[i].blocks.size());
  }
}
