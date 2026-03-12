#include <gtest/gtest.h>

#include <random>

#include "simulation/elements.hpp"
#include "simulation/material.hpp"
#include "simulation/systems.hpp"

TEST(UnitMath, MaterialNormalizationAndDerivedColor) {
  evo::sim::MaterialComposition composition;
  composition.components = {{0, 2.0}, {1, 1.0}, {2, -1.0}};
  composition.normalize();

  ASSERT_EQ(composition.components.size(), 3U);
  EXPECT_NEAR(composition.components[0].weight, 2.0 / 3.0, 1e-9);
  EXPECT_NEAR(composition.components[1].weight, 1.0 / 3.0, 1e-9);
  EXPECT_NEAR(composition.components[2].weight, 0.0, 1e-9);

  evo::sim::ElementRegistry registry;
  const auto color = composition.derived_color(registry);
  EXPECT_GT(color.r + color.g + color.b, 0.0F);
}

TEST(UnitMath, BlockDamageLifecycleIntegrity) {
  evo::sim::Block block;
  block.durability = 10.0;
  block.damage_state.wear = 3.0;
  EXPECT_NEAR(block.integrity(), 0.7, 1e-9);

  block.damage_state.wear = 20.0;
  EXPECT_NEAR(block.integrity(), 0.0, 1e-9);
}

TEST(UnitMath, GenomeMutationAndSexualCrossover) {
  evo::sim::Creature a;
  a.id = 1;
  a.genome.id = 1;
  evo::sim::Gene ga;
  ga.id = 10;
  ga.build_order = 0;
  ga.block_type = evo::sim::BlockType::Compute;
  ga.priority = 2;
  ga.composition.components = {{3, 1.0}};
  a.genome.genes.push_back(ga);

  evo::sim::Creature b = a;
  b.id = 2;
  b.genome.id = 2;
  evo::sim::Gene gb = ga;
  gb.id = 11;
  gb.build_order = 1;
  gb.block_type = evo::sim::BlockType::Sensor;
  gb.priority = 4;
  b.genome.genes.push_back(gb);

  evo::sim::DeterministicIds ids {.next_creature_id = 100, .next_genome_id = 200, .next_gene_id = 300, .next_block_id = 400};
  std::mt19937 rng(123);

  const auto child_asexual = evo::sim::reproduce_asexual(a, ids, rng, 1.0);
  ASSERT_FALSE(child_asexual.genome.genes.empty());
  EXPECT_EQ(child_asexual.id, 100U);

  const auto child_sexual = evo::sim::reproduce_sexual(a, b, ids, rng, 0.0);
  EXPECT_EQ(child_sexual.id, 101U);
  EXPECT_FALSE(child_sexual.genome.genes.empty());
  EXPECT_EQ(child_sexual.blocks.size(), child_sexual.genome.genes.size());
}
