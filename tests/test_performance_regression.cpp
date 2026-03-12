#include <gtest/gtest.h>

#include <chrono>
#include <vector>

#include "simulation/systems.hpp"

namespace {

evo::sim::Creature perf_creature(std::uint64_t id) {
  evo::sim::Creature c;
  c.id = id;
  c.genome.id = id;
  c.energy_pool = 50.0;
  c.position = evo::sim::SpatialPosition {static_cast<double>(id % 64), static_cast<double>((id / 64) % 64), 0.0};

  evo::sim::Block block;
  block.id = id;
  block.durability = 10.0;
  block.maintenance_energy_cost = 0.5;
  block.action_energy_cost = 0.5;
  block.repair_energy_cost = 0.2;
  block.composition.components = {{8, 0.5}, {26, 0.5}};
  block.composition.normalize();
  c.blocks.push_back(block);
  return c;
}

void run_budget_case(std::size_t population, double tick_budget_ms) {
  evo::sim::SimulationEnvironment env(123);
  std::vector<evo::sim::Creature> creatures;
  creatures.reserve(population);
  for (std::size_t i = 0; i < population; ++i) {
    creatures.push_back(perf_creature(static_cast<std::uint64_t>(i + 1)));
  }

  constexpr int ticks = 20;
  auto start = std::chrono::steady_clock::now();
  for (int t = 0; t < ticks; ++t) {
    evo::sim::run_tick(creatures, env, evo::sim::TickContext {.tick_index = static_cast<std::uint64_t>(t), .deterministic_seed = 123});
  }
  auto end = std::chrono::steady_clock::now();

  const double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
  const double avg_tick_ms = elapsed_ms / static_cast<double>(ticks);
  EXPECT_LE(avg_tick_ms, tick_budget_ms);
}

} // namespace

TEST(PerformanceRegression, Population500WithinBudget) {
  run_budget_case(500, 8.0);
}

TEST(PerformanceRegression, Population1000WithinBudget) {
  run_budget_case(1000, 12.0);
}

TEST(PerformanceRegression, Population5000WithinBudget) {
  run_budget_case(5000, 35.0);
}
