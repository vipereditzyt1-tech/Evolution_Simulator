#include <gtest/gtest.h>

#include <random>

#include "simulation/elements.hpp"
#include "simulation/systems.hpp"

namespace {

evo::sim::Creature make_creature(const std::uint64_t id, const std::uint64_t block_id, const evo::sim::ElementId element) {
  evo::sim::Creature creature;
  creature.id = id;
  creature.genome.id = id;
  creature.energy_pool = 10.0;

  evo::sim::Block block;
  block.id = block_id;
  block.type = evo::sim::BlockType::Structural;
  block.mass = 2.0;
  block.durability = 4.0;
  block.maintenance_energy_cost = 2.0;
  block.action_energy_cost = 1.0;
  block.repair_energy_cost = 1.0;
  block.composition.components = {{element, 1.0}};
  block.composition.normalize();

  creature.blocks.push_back(block);
  return creature;
}

} // namespace

TEST(Simulation, ElementRegistryHas50BaseElements) {
  evo::sim::ElementRegistry registry;
  EXPECT_EQ(registry.all().size(), evo::sim::ElementRegistry::kBaseElementCount);
  EXPECT_EQ(registry.get(0).name, "Element_1");
  EXPECT_EQ(registry.get(49).name, "Element_50");
}

TEST(Simulation, TickUsesDeterministicOrderingAndRecyclesBrokenBlocks) {
  evo::sim::SimulationEnvironment env;
  std::vector<evo::sim::Creature> creatures;
  creatures.push_back(make_creature(2, 20, 3));
  creatures.push_back(make_creature(1, 10, 5));

  evo::sim::TickContext ctx {.tick_index = 1, .deterministic_seed = 42};
  for (int i = 0; i < 4; ++i) {
    evo::sim::run_tick(creatures, env, ctx);
  }

  ASSERT_EQ(creatures.size(), 2);
  EXPECT_EQ(creatures[0].id, 1);
  EXPECT_EQ(creatures[1].id, 2);
  EXPECT_TRUE(creatures[0].blocks.empty());
  EXPECT_TRUE(creatures[1].blocks.empty());
  EXPECT_EQ(env.recycled_materials.size(), 2);
}

TEST(Simulation, ReproductionAssignsDeterministicIds) {
  evo::sim::DeterministicIds ids {.next_creature_id = 100, .next_genome_id = 200, .next_gene_id = 300, .next_block_id = 400};
  std::mt19937 rng(7);

  evo::sim::Creature parent = make_creature(1, 1, 0);
  evo::sim::Gene g;
  g.id = 10;
  g.build_order = 0;
  g.block_type = evo::sim::BlockType::Compute;
  g.composition.components = {{0, 1.0}};
  parent.genome.genes.push_back(g);

  evo::sim::Creature clone = evo::sim::reproduce_asexual(parent, ids, rng, 0.0);
  EXPECT_EQ(clone.id, 100);
  ASSERT_FALSE(clone.genome.genes.empty());
  EXPECT_EQ(clone.genome.genes[0].id, 300);
  ASSERT_FALSE(clone.blocks.empty());
  EXPECT_EQ(clone.blocks[0].id, 400);
}
