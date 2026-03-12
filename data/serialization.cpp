#include "serialization.hpp"

#include <sstream>
#include <stdexcept>

namespace evo::data {
namespace {

std::string creature_line(const sim::Creature& creature) {
  std::ostringstream out;
  out << creature.id << '|' << creature.genome.id << '|' << creature.energy_pool << '|' << creature.age_ticks << '|'
      << creature.position.x << ',' << creature.position.y << ',' << creature.position.z << '|';

  for (std::size_t i = 0; i < creature.blocks.size(); ++i) {
    const auto& b = creature.blocks[i];
    out << b.id << ':' << static_cast<int>(b.type) << ':' << b.mass;
    if (i + 1 < creature.blocks.size()) {
      out << ';';
    }
  }
  return out.str();
}

sim::Creature parse_creature_line(const std::string& line) {
  sim::Creature creature;
  std::stringstream ss(line);
  std::string token;

  std::getline(ss, token, '|');
  creature.id = static_cast<sim::CreatureId>(std::stoull(token));
  std::getline(ss, token, '|');
  creature.genome.id = static_cast<sim::GenomeId>(std::stoull(token));
  std::getline(ss, token, '|');
  creature.energy_pool = std::stod(token);
  std::getline(ss, token, '|');
  creature.age_ticks = static_cast<std::uint64_t>(std::stoull(token));

  std::getline(ss, token, '|');
  {
    std::stringstream xyz(token);
    std::string axis;
    std::getline(xyz, axis, ',');
    creature.position.x = std::stod(axis);
    std::getline(xyz, axis, ',');
    creature.position.y = std::stod(axis);
    std::getline(xyz, axis, ',');
    creature.position.z = std::stod(axis);
  }

  if (std::getline(ss, token, '|') && !token.empty()) {
    std::stringstream blocks(token);
    std::string block_token;
    while (std::getline(blocks, block_token, ';')) {
      std::stringstream bs(block_token);
      std::string field;
      sim::Block block;
      std::getline(bs, field, ':');
      block.id = static_cast<sim::BlockId>(std::stoull(field));
      std::getline(bs, field, ':');
      block.type = static_cast<sim::BlockType>(std::stoi(field));
      std::getline(bs, field, ':');
      block.mass = std::stod(field);
      creature.blocks.push_back(block);
    }
  }

  return creature;
}

std::uint32_t parse_version(const std::string& line) {
  if (line.rfind("schema_version=", 0) == 0) {
    return static_cast<std::uint32_t>(std::stoul(line.substr(15)));
  }
  if (line.rfind("version=", 0) == 0) {
    return 0;
  }
  throw std::runtime_error("missing schema version");
}

} // namespace

std::string serialize_to_text(const SaveData& data) {
  std::ostringstream out;
  out << "schema_version=" << data.schema_version << '\n';
  out << "terrain_count=" << data.terrain.size() << '\n';
  for (const auto& cell : data.terrain) {
    out << "terrain=" << cell.x << ',' << cell.y << ',' << cell.z << ',' << cell.terrain_height << '\n';
  }

  out << "environment_count=" << data.environmental_blocks.size() << '\n';
  for (const auto& block : data.environmental_blocks) {
    out << "env=" << block.x << ',' << block.y << ',' << block.z << ',' << block.temperature << ',' << block.pressure << ','
        << block.energy_density << ',' << block.resource_density << '\n';
  }

  out << "creature_count=" << data.creatures.size() << '\n';
  for (const auto& creature : data.creatures) {
    out << "creature=" << creature_line(creature) << '\n';
  }

  out << "stats=" << data.statistics.population << ',' << data.statistics.diversity << ',' << data.statistics.extinctions << ','
      << data.statistics.average_genome_size << '\n';
  out << "timeline_count=" << data.statistics.timeline.size() << '\n';
  for (const auto& point : data.statistics.timeline) {
    out << "timeline=" << point.tick << ',' << point.population << ',' << point.diversity << ',' << point.extinctions << ','
        << point.average_genome_size << '\n';
  }

  return out.str();
}

DeserializeResult deserialize_with_migration(const std::string& source_text) {
  try {
    std::stringstream input(source_text);
    std::string line;
    if (!std::getline(input, line)) {
      return DeserializeResult {.error = DeserializeError::ParseError, .message = "empty payload"};
    }

    const auto version = parse_version(line);
    if (version > kCurrentSchemaVersion) {
      return DeserializeResult {.error = DeserializeError::UnsupportedVersion, .message = "unsupported schema"};
    }

    SaveData data;
    data.schema_version = kCurrentSchemaVersion;

    while (std::getline(input, line)) {
      if (line.rfind("terrain=", 0) == 0) {
        std::stringstream row(line.substr(8));
        std::string f;
        TerrainCellRecord cell;
        std::getline(row, f, ',');
        cell.x = std::stoi(f);
        std::getline(row, f, ',');
        cell.y = std::stoi(f);
        std::getline(row, f, ',');
        cell.z = std::stoi(f);
        std::getline(row, f, ',');
        cell.terrain_height = std::stod(f);
        data.terrain.push_back(cell);
      } else if (line.rfind("env=", 0) == 0) {
        std::stringstream row(line.substr(4));
        std::string f;
        EnvironmentalBlockRecord block;
        std::getline(row, f, ',');
        block.x = std::stoi(f);
        std::getline(row, f, ',');
        block.y = std::stoi(f);
        std::getline(row, f, ',');
        block.z = std::stoi(f);
        std::getline(row, f, ',');
        block.temperature = std::stod(f);
        std::getline(row, f, ',');
        block.pressure = std::stod(f);
        std::getline(row, f, ',');
        block.energy_density = std::stod(f);
        std::getline(row, f, ',');
        block.resource_density = std::stod(f);
        data.environmental_blocks.push_back(block);
      } else if (line.rfind("creature=", 0) == 0) {
        data.creatures.push_back(parse_creature_line(line.substr(9)));
      } else if (line.rfind("stats=", 0) == 0) {
        std::stringstream row(line.substr(6));
        std::string f;
        std::getline(row, f, ',');
        data.statistics.population = static_cast<std::size_t>(std::stoull(f));
        std::getline(row, f, ',');
        data.statistics.diversity = std::stod(f);
        std::getline(row, f, ',');
        data.statistics.extinctions = static_cast<std::size_t>(std::stoull(f));
        std::getline(row, f, ',');
        data.statistics.average_genome_size = std::stod(f);
      } else if (line.rfind("timeline=", 0) == 0) {
        std::stringstream row(line.substr(9));
        std::string f;
        ui::StatisticsPoint point;
        std::getline(row, f, ',');
        point.tick = std::stoull(f);
        std::getline(row, f, ',');
        point.population = static_cast<std::size_t>(std::stoull(f));
        std::getline(row, f, ',');
        point.diversity = std::stod(f);
        std::getline(row, f, ',');
        point.extinctions = static_cast<std::size_t>(std::stoull(f));
        std::getline(row, f, ',');
        point.average_genome_size = std::stod(f);
        data.statistics.timeline.push_back(point);
      }
    }

    return DeserializeResult {.error = DeserializeError::None, .payload = std::move(data)};
  } catch (const std::invalid_argument& e) {
    return DeserializeResult {.error = DeserializeError::ParseError, .message = e.what()};
  } catch (const std::out_of_range& e) {
    return DeserializeResult {.error = DeserializeError::CorruptPayload, .message = e.what()};
  } catch (const std::exception& e) {
    return DeserializeResult {.error = DeserializeError::CorruptPayload, .message = e.what()};
  }
}

} // namespace evo::data
