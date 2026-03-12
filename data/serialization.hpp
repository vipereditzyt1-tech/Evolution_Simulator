#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "simulation/creatures/creature.hpp"
#include "ui/panels.hpp"

namespace evo::data {

constexpr std::uint32_t kCurrentSchemaVersion = 1;

struct TerrainCellRecord {
  int x {0};
  int y {0};
  int z {0};
  double terrain_height {0.0};
};

struct EnvironmentalBlockRecord {
  int x {0};
  int y {0};
  int z {0};
  double temperature {0.0};
  double pressure {0.0};
  double energy_density {0.0};
  double resource_density {0.0};
};

struct WorldStatistics {
  std::size_t population {0};
  double diversity {0.0};
  std::size_t extinctions {0};
  double average_genome_size {0.0};
  std::vector<ui::StatisticsPoint> timeline;
};

struct SaveData {
  std::uint32_t schema_version {kCurrentSchemaVersion};
  std::vector<TerrainCellRecord> terrain;
  std::vector<EnvironmentalBlockRecord> environmental_blocks;
  std::vector<sim::Creature> creatures;
  WorldStatistics statistics;
};

enum class DeserializeError {
  None,
  ParseError,
  UnsupportedVersion,
  CorruptPayload,
};

struct DeserializeResult {
  DeserializeError error {DeserializeError::None};
  std::string message;
  std::optional<SaveData> payload;
};

std::string serialize_to_text(const SaveData& data);
DeserializeResult deserialize_with_migration(const std::string& source_text);

} // namespace evo::data
