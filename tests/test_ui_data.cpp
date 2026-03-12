#include <gtest/gtest.h>

#include "data/serialization.hpp"
#include "engine/render_loop.hpp"
#include "ui/panels.hpp"

namespace {

evo::sim::Creature make_creature() {
  evo::sim::Creature creature;
  creature.id = 42;
  creature.genome.id = 9;
  creature.energy_pool = 55.0;
  creature.age_ticks = 10;
  creature.position = evo::sim::SpatialPosition {2.0, 3.0, 4.0};
  creature.history.events = {"born", "asexual_reproduction"};

  evo::sim::Block block;
  block.id = 77;
  block.composition.components = {{6, 1.0}};
  block.composition.normalize();
  creature.blocks.push_back(block);

  evo::sim::Gene gene;
  gene.id = 1;
  gene.block_type = evo::sim::BlockType::Compute;
  gene.composition.components = {{6, 1.0}};
  gene.parameters = {0.1, 0.2};
  creature.genome.genes.push_back(gene);

  creature.neural_network_state.neurons = {0.5, 0.7};
  creature.neural_network_state.activations = {0.4};

  return creature;
}

} // namespace

TEST(RenderLoop, SelectionAndColorizedBlocks) {
  evo::engine::RenderLoop loop;
  evo::engine::OrbitCamera camera;
  std::vector<evo::sim::Creature> creatures {make_creature()};

  auto selected = loop.pick_creature(creatures, evo::sim::SpatialPosition {2.0, 3.0, 4.0}, 0.5);
  ASSERT_TRUE(selected.has_value());
  loop.set_selected_creature(selected);

  const auto frame = loop.build_frame(creatures, camera);
  ASSERT_EQ(frame.blocks.size(), 1);
  EXPECT_TRUE(frame.blocks[0].selected);
  EXPECT_GT(frame.blocks[0].color.r + frame.blocks[0].color.g + frame.blocks[0].color.b, 0.0F);
}

TEST(UiPanels, InspectorAndStatisticsUpdate) {
  evo::ui::UiState state;
  const auto creature = make_creature();

  state.controls.pause();
  EXPECT_TRUE(state.controls.paused);
  state.controls.resume();
  EXPECT_FALSE(state.controls.paused);

  state.inspector.update_from(creature);
  EXPECT_TRUE(state.inspector.focused_creature.has_value());
  EXPECT_EQ(state.inspector.reproduction_history.size(), 2);

  state.statistics.append_timeline_point(evo::ui::StatisticsPoint {
    .tick = 1,
    .population = 5,
    .diversity = 2.5,
    .extinctions = 1,
    .average_genome_size = 3.0,
  });
  EXPECT_EQ(state.statistics.population, 5U);
}

TEST(DataSerialization, RoundTripAndMigrationAndCorruptionHandling) {
  evo::data::SaveData save;
  save.terrain.push_back({.x = 0, .y = 1, .z = 2, .terrain_height = 3.0});
  save.environmental_blocks.push_back({.x = 1, .y = 2, .z = 3, .temperature = 290.0, .pressure = 1.2, .energy_density = 0.4, .resource_density = 0.8});
  save.creatures.push_back(make_creature());
  save.statistics.population = 1;

  const auto serialized = evo::data::serialize_to_text(save);
  const auto loaded = evo::data::deserialize_with_migration(serialized);
  ASSERT_EQ(loaded.error, evo::data::DeserializeError::None);
  ASSERT_TRUE(loaded.payload.has_value());
  EXPECT_EQ(loaded.payload->creatures.size(), 1U);

  std::string legacy = serialized;
  legacy.replace(0, std::string("schema_version=1").size(), "version=0");
  const auto migrated = evo::data::deserialize_with_migration(legacy);
  EXPECT_EQ(migrated.error, evo::data::DeserializeError::None);

  const auto corrupted = evo::data::deserialize_with_migration("schema_version=999\n");
  EXPECT_EQ(corrupted.error, evo::data::DeserializeError::UnsupportedVersion);

  const auto malformed = evo::data::deserialize_with_migration("schema_version=1\nterrain=nan,1,2,3\n");
  EXPECT_EQ(malformed.error, evo::data::DeserializeError::ParseError);
}
