#include "world.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace evo::sim {

namespace {
constexpr double kPi = 3.14159265358979323846;

int floor_div(const int value, const int divisor) {
  const int q = value / divisor;
  const int r = value % divisor;
  return (r != 0 && ((r < 0) != (divisor < 0))) ? q - 1 : q;
}

int floor_mod(const int value, const int mod) {
  const int result = value % mod;
  return result < 0 ? result + mod : result;
}

} // namespace

EnvironmentWorld::Chunk::Chunk() {
  cells.resize(static_cast<std::size_t>(kChunkSize * kChunkSize * kChunkSize));
}

std::size_t EnvironmentWorld::ChunkCoordHash::operator()(const ChunkCoord& coord) const {
  const std::uint64_t x = static_cast<std::uint64_t>(static_cast<std::int64_t>(coord.x) * 73856093LL);
  const std::uint64_t y = static_cast<std::uint64_t>(static_cast<std::int64_t>(coord.y) * 19349663LL);
  const std::uint64_t z = static_cast<std::uint64_t>(static_cast<std::int64_t>(coord.z) * 83492791LL);
  return static_cast<std::size_t>(x ^ y ^ z);
}

EnvironmentWorld::EnvironmentWorld(const std::uint32_t seed) : seed_(seed) {}

void EnvironmentWorld::tick(const std::uint64_t tick_index, const std::vector<Creature>& creatures,
                            std::vector<InventoryItem>& recycled_materials, ThreadPool* jobs) {
  creature_grid_.rebuild(creatures);

  std::unordered_set<ChunkCoord, ChunkCoordHash> active_chunks;
  active_chunks.reserve(creature_grid_.buckets.size() * 8 + 8);

  for (const auto& [coord, _] : creature_grid_.buckets) {
    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dz = -1; dz <= 1; ++dz) {
          active_chunks.insert(ChunkCoord {coord.x + dx, coord.y + dy, coord.z + dz});
        }
      }
    }
  }

  if (active_chunks.empty()) {
    active_chunks.insert(ChunkCoord {0, 0, 0});
  }

  std::vector<ChunkCoord> ordered_chunks(active_chunks.begin(), active_chunks.end());
  std::sort(ordered_chunks.begin(), ordered_chunks.end(), [](const ChunkCoord& a, const ChunkCoord& b) {
    if (a.x != b.x) {
      return a.x < b.x;
    }
    if (a.y != b.y) {
      return a.y < b.y;
    }
    return a.z < b.z;
  });

  if (jobs != nullptr) {
    std::vector<std::vector<InventoryItem>> local_recycled(ordered_chunks.size());
    jobs->parallel_for(0, ordered_chunks.size(), [&](const std::size_t i) {
      Chunk& chunk = ensure_chunk(ordered_chunks[i]);
      apply_rainfall(chunk, tick_index);
      apply_temperature(chunk, tick_index);
      apply_pressure(chunk, tick_index);
      apply_chemical_weathering(chunk, local_recycled[i]);
      apply_geological_compression(chunk);
      apply_resource_diffusion(chunk);
    });
    for (const auto& local : local_recycled) {
      recycled_materials.insert(recycled_materials.end(), local.begin(), local.end());
    }
    return;
  }

  for (const auto& coord : ordered_chunks) {
    Chunk& chunk = ensure_chunk(coord);
    apply_rainfall(chunk, tick_index);
    apply_temperature(chunk, tick_index);
    apply_pressure(chunk, tick_index);
    apply_chemical_weathering(chunk, recycled_materials);
    apply_geological_compression(chunk);
    apply_resource_diffusion(chunk);
  }
}

EnvironmentSensorSample EnvironmentWorld::query_environment(const SpatialPosition& world_position, const ElementId element,
                                                            const int radius) const {
  const ChunkCoord center_chunk = to_chunk_coord(world_position);
  const Int3 local = to_local_index(world_position);

  EnvironmentSensorSample sample;
  double total_rain = 0.0;
  double total_material = 0.0;
  std::size_t count = 0;

  auto lookup_cell = [&](int wx, int wy, int wz) -> const Cell* {
    const ChunkCoord coord {floor_div(wx, kChunkSize), floor_div(wy, kChunkSize), floor_div(wz, kChunkSize)};
    const Chunk* chunk = find_chunk(coord);
    if (chunk == nullptr) {
      return nullptr;
    }
    const int lx = floor_mod(wx, kChunkSize);
    const int ly = floor_mod(wy, kChunkSize);
    const int lz = floor_mod(wz, kChunkSize);
    return &chunk->cells[cell_index(lx, ly, lz)];
  };

  const int world_x = center_chunk.x * kChunkSize + local.x;
  const int world_y = center_chunk.y * kChunkSize + local.y;
  const int world_z = center_chunk.z * kChunkSize + local.z;

  const Cell* center = lookup_cell(world_x, world_y, world_z);
  if (center != nullptr) {
    const Cell* txp = lookup_cell(world_x + 1, world_y, world_z);
    const Cell* txn = lookup_cell(world_x - 1, world_y, world_z);
    const Cell* typ = lookup_cell(world_x, world_y + 1, world_z);
    const Cell* tyn = lookup_cell(world_x, world_y - 1, world_z);
    const Cell* tzp = lookup_cell(world_x, world_y, world_z + 1);
    const Cell* tzn = lookup_cell(world_x, world_y, world_z - 1);

    if (txp != nullptr && txn != nullptr) {
      sample.temperature_gradient.x = (txp->temperature - txn->temperature) * 0.5;
      sample.pressure_gradient.x = (txp->pressure - txn->pressure) * 0.5;
    }
    if (typ != nullptr && tyn != nullptr) {
      sample.temperature_gradient.y = (typ->temperature - tyn->temperature) * 0.5;
      sample.pressure_gradient.y = (typ->pressure - tyn->pressure) * 0.5;
    }
    if (tzp != nullptr && tzn != nullptr) {
      sample.temperature_gradient.z = (tzp->temperature - tzn->temperature) * 0.5;
      sample.pressure_gradient.z = (tzp->pressure - tzn->pressure) * 0.5;
    }
  }

  for (int dx = -radius; dx <= radius; ++dx) {
    for (int dy = -radius; dy <= radius; ++dy) {
      for (int dz = -radius; dz <= radius; ++dz) {
        const Cell* cell = lookup_cell(world_x + dx, world_y + dy, world_z + dz);
        if (cell == nullptr) {
          continue;
        }
        total_rain += cell->rainfall;
        const auto it = cell->materials.find(element);
        if (it != cell->materials.end()) {
          total_material += it->second;
        }
        ++count;
      }
    }
  }

  if (count > 0) {
    sample.local_rainfall_density = total_rain / static_cast<double>(count);
    sample.local_material_density = total_material / static_cast<double>(count);
  }

  return sample;
}

std::size_t EnvironmentWorld::loaded_chunk_count() const {
  return chunks_.size();
}


std::vector<CreatureId> EnvironmentWorld::query_neighbors(const SpatialPosition& world_position, const double radius) const {
  const ChunkCoord center = to_chunk_coord(world_position);
  const int chunk_radius = std::max(1, static_cast<int>(std::ceil(radius / static_cast<double>(kChunkSize))));
  const double radius_sq = radius * radius;
  std::vector<CreatureId> neighbors;

  for (int dx = -chunk_radius; dx <= chunk_radius; ++dx) {
    for (int dy = -chunk_radius; dy <= chunk_radius; ++dy) {
      for (int dz = -chunk_radius; dz <= chunk_radius; ++dz) {
        const ChunkCoord c {center.x + dx, center.y + dy, center.z + dz};
        const auto bucket_it = creature_grid_.buckets.find(c);
        if (bucket_it == creature_grid_.buckets.end()) {
          continue;
        }
        for (const CreatureId id : bucket_it->second) {
          const auto pos_it = creature_grid_.positions.find(id);
          if (pos_it == creature_grid_.positions.end()) {
            continue;
          }
          const auto& p = pos_it->second;
          const double sx = p.x - world_position.x;
          const double sy = p.y - world_position.y;
          const double sz = p.z - world_position.z;
          if ((sx * sx + sy * sy + sz * sz) <= radius_sq) {
            neighbors.push_back(id);
          }
        }
      }
    }
  }

  std::sort(neighbors.begin(), neighbors.end());
  neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());
  return neighbors;
}

std::vector<std::pair<CreatureId, CreatureId>> EnvironmentWorld::broad_phase_pairs(const double radius) const {
  std::vector<std::pair<CreatureId, CreatureId>> pairs;
  const double radius_sq = radius * radius;

  for (const auto& [coord, ids] : creature_grid_.buckets) {
    for (std::size_t i = 0; i < ids.size(); ++i) {
      for (std::size_t j = i + 1; j < ids.size(); ++j) {
        const auto a_it = creature_grid_.positions.find(ids[i]);
        const auto b_it = creature_grid_.positions.find(ids[j]);
        if (a_it == creature_grid_.positions.end() || b_it == creature_grid_.positions.end()) {
          continue;
        }
        const double dx = a_it->second.x - b_it->second.x;
        const double dy = a_it->second.y - b_it->second.y;
        const double dz = a_it->second.z - b_it->second.z;
        if ((dx * dx + dy * dy + dz * dz) <= radius_sq) {
          pairs.emplace_back(std::min(ids[i], ids[j]), std::max(ids[i], ids[j]));
        }
      }

      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          for (int dz = -1; dz <= 1; ++dz) {
            if (dx == 0 && dy == 0 && dz == 0) {
              continue;
            }
            const ChunkCoord n {coord.x + dx, coord.y + dy, coord.z + dz};
            if (n.x < coord.x || (n.x == coord.x && n.y < coord.y) ||
                (n.x == coord.x && n.y == coord.y && n.z <= coord.z)) {
              continue;
            }
            const auto n_it = creature_grid_.buckets.find(n);
            if (n_it == creature_grid_.buckets.end()) {
              continue;
            }
            for (const CreatureId other : n_it->second) {
              const auto a_it = creature_grid_.positions.find(ids[i]);
              const auto b_it = creature_grid_.positions.find(other);
              if (a_it == creature_grid_.positions.end() || b_it == creature_grid_.positions.end()) {
                continue;
              }
              const double ddx = a_it->second.x - b_it->second.x;
              const double ddy = a_it->second.y - b_it->second.y;
              const double ddz = a_it->second.z - b_it->second.z;
              if ((ddx * ddx + ddy * ddy + ddz * ddz) <= radius_sq) {
                pairs.emplace_back(std::min(ids[i], other), std::max(ids[i], other));
              }
            }
          }
        }
      }
    }
  }

  std::sort(pairs.begin(), pairs.end());
  pairs.erase(std::unique(pairs.begin(), pairs.end()), pairs.end());
  return pairs;
}

EnvironmentWorld::ChunkCoord EnvironmentWorld::to_chunk_coord(const SpatialPosition& position) {
  return ChunkCoord {
    floor_div(static_cast<int>(std::floor(position.x)), kChunkSize),
    floor_div(static_cast<int>(std::floor(position.y)), kChunkSize),
    floor_div(static_cast<int>(std::floor(position.z)), kChunkSize),
  };
}

Int3 EnvironmentWorld::to_local_index(const SpatialPosition& position) {
  return Int3 {
    floor_mod(static_cast<int>(std::floor(position.x)), kChunkSize),
    floor_mod(static_cast<int>(std::floor(position.y)), kChunkSize),
    floor_mod(static_cast<int>(std::floor(position.z)), kChunkSize),
  };
}

std::size_t EnvironmentWorld::cell_index(const int x, const int y, const int z) {
  return static_cast<std::size_t>(x + y * kChunkSize + z * kChunkSize * kChunkSize);
}

EnvironmentWorld::Chunk& EnvironmentWorld::ensure_chunk(const ChunkCoord& coord) {
  auto it = chunks_.find(coord);
  if (it != chunks_.end()) {
    return it->second;
  }

  Chunk chunk;
  chunk.coord = coord;
  generate_terrain_and_deposits(chunk);
  auto [inserted, _] = chunks_.emplace(coord, std::move(chunk));
  return inserted->second;
}

const EnvironmentWorld::Chunk* EnvironmentWorld::find_chunk(const ChunkCoord& coord) const {
  auto it = chunks_.find(coord);
  if (it == chunks_.end()) {
    return nullptr;
  }
  return &it->second;
}

void EnvironmentWorld::generate_terrain_and_deposits(Chunk& chunk) {
  for (int z = 0; z < kChunkSize; ++z) {
    for (int y = 0; y < kChunkSize; ++y) {
      for (int x = 0; x < kChunkSize; ++x) {
        Cell& cell = chunk.cells[cell_index(x, y, z)];
        const int wx = chunk.coord.x * kChunkSize + x;
        const int wy = chunk.coord.y * kChunkSize + y;
        const int wz = chunk.coord.z * kChunkSize + z;

        const double terrain = seeded_noise(wx, wy, wz, 11);
        cell.terrain_height = terrain * 120.0;
        cell.temperature = 273.0 + terrain * 40.0;
        cell.pressure = 0.8 + seeded_noise(wx, wy, wz, 23) * 0.6;
        cell.rainfall = std::max(0.0, seeded_noise(wx, wy, wz, 31) - 0.4);
        cell.chemical_weathering = std::max(0.0, seeded_noise(wx, wy, wz, 47) - 0.5);
        cell.geological_compression = std::max(0.0, seeded_noise(wx, wy, wz, 59) - 0.35);

        const double iron_likelihood = seeded_noise(wx, wy, wz, 71);
        if (iron_likelihood > 0.65) {
          cell.materials[26] = (iron_likelihood - 0.65) * 5.0;
        }

        const double carbon_likelihood = seeded_noise(wx, wy, wz, 73);
        if (carbon_likelihood > 0.70) {
          cell.materials[6] = (carbon_likelihood - 0.70) * 6.5;
        }
      }
    }
  }
}

void EnvironmentWorld::apply_rainfall(Chunk& chunk, const std::uint64_t tick_index) {
  const double phase = static_cast<double>((tick_index + seed_) % 360) * (kPi / 180.0);
  for (auto& cell : chunk.cells) {
    const double target = std::clamp(cell.pressure - 0.8, 0.0, 1.0) * (0.5 + 0.5 * std::sin(phase));
    cell.rainfall = 0.92 * cell.rainfall + 0.08 * target;
  }
}

void EnvironmentWorld::apply_temperature(Chunk& chunk, const std::uint64_t tick_index) {
  const double season = std::sin((static_cast<double>(tick_index) + static_cast<double>(seed_)) * 0.01);
  for (auto& cell : chunk.cells) {
    const double altitude_penalty = cell.terrain_height * 0.004;
    cell.temperature = std::clamp(cell.temperature + season * 0.3 - altitude_penalty, 180.0, 340.0);
  }
}

void EnvironmentWorld::apply_pressure(Chunk& chunk, const std::uint64_t tick_index) {
  for (auto& cell : chunk.cells) {
    const double thermal_term = (cell.temperature - 273.0) / 273.0;
    const double tidal_term = std::sin((static_cast<double>(tick_index) + cell.terrain_height) * 0.02) * 0.02;
    cell.pressure = std::clamp(cell.pressure + 0.01 * thermal_term + tidal_term, 0.2, 3.0);
  }
}

void EnvironmentWorld::apply_chemical_weathering(Chunk& chunk, std::vector<InventoryItem>& recycled_materials) {
  for (auto& cell : chunk.cells) {
    const double reactive_flux = std::max(0.0, cell.rainfall * (cell.temperature / 300.0));
    cell.chemical_weathering = std::clamp(cell.chemical_weathering + reactive_flux * 0.01, 0.0, 10.0);

    auto iron_it = cell.materials.find(26);
    if (iron_it != cell.materials.end() && cell.chemical_weathering > 0.5) {
      const double oxidized = std::min(iron_it->second, cell.chemical_weathering * 0.005);
      iron_it->second -= oxidized;
      cell.materials[8] += oxidized * 0.6;
      recycled_materials.push_back(InventoryItem {8, oxidized * 0.4});
      if (iron_it->second <= 0.0001) {
        cell.materials.erase(iron_it);
      }
    }
  }
}

void EnvironmentWorld::apply_geological_compression(Chunk& chunk) {
  for (auto& cell : chunk.cells) {
    cell.geological_compression = std::clamp(cell.geological_compression + cell.pressure * 0.002, 0.0, 20.0);
    const auto carbon_it = cell.materials.find(6);
    if (carbon_it != cell.materials.end() && cell.geological_compression > 3.0) {
      const double transformed = std::min(carbon_it->second, cell.geological_compression * 0.002);
      carbon_it->second -= transformed;
      cell.materials[14] += transformed * 0.75;
      if (carbon_it->second <= 0.0001) {
        cell.materials.erase(carbon_it);
      }
    }
  }
}

void EnvironmentWorld::apply_resource_diffusion(Chunk& chunk) {
  std::vector<double> oxygen_buffer(chunk.cells.size(), 0.0);
  for (int z = 0; z < kChunkSize; ++z) {
    for (int y = 0; y < kChunkSize; ++y) {
      for (int x = 0; x < kChunkSize; ++x) {
        const std::size_t idx = cell_index(x, y, z);
        const auto oxygen_it = chunk.cells[idx].materials.find(8);
        if (oxygen_it == chunk.cells[idx].materials.end()) {
          continue;
        }
        const double source = oxygen_it->second;
        const double spread = source * 0.05;
        oxygen_buffer[idx] -= spread;

        const std::array<Int3, 6> neighbors {{{x + 1, y, z}, {x - 1, y, z}, {x, y + 1, z},
                                               {x, y - 1, z}, {x, y, z + 1}, {x, y, z - 1}}};
        const double per_neighbor = spread / 6.0;
        for (const auto& n : neighbors) {
          if (n.x < 0 || n.y < 0 || n.z < 0 || n.x >= kChunkSize || n.y >= kChunkSize || n.z >= kChunkSize) {
            continue;
          }
          oxygen_buffer[cell_index(n.x, n.y, n.z)] += per_neighbor;
        }
      }
    }
  }

  for (std::size_t i = 0; i < chunk.cells.size(); ++i) {
    const double delta = oxygen_buffer[i];
    if (std::abs(delta) < 0.0000001) {
      continue;
    }
    chunk.cells[i].materials[8] = std::max(0.0, chunk.cells[i].materials[8] + delta);
    if (chunk.cells[i].materials[8] <= 0.0001) {
      chunk.cells[i].materials.erase(8);
    }
  }
}

double EnvironmentWorld::seeded_noise(const int x, const int y, const int z, const std::uint64_t salt) const {
  std::uint64_t h = static_cast<std::uint64_t>(seed_) ^ (salt * 0x9e3779b185ebca87ULL);
  h ^= static_cast<std::uint64_t>(x) * 0xbf58476d1ce4e5b9ULL;
  h ^= static_cast<std::uint64_t>(y) * 0x94d049bb133111ebULL;
  h ^= static_cast<std::uint64_t>(z) * 0xd6e8feb86659fd93ULL;
  h ^= (h >> 30);
  h *= 0xbf58476d1ce4e5b9ULL;
  h ^= (h >> 27);
  h *= 0x94d049bb133111ebULL;
  h ^= (h >> 31);
  constexpr double denom = static_cast<double>(std::numeric_limits<std::uint64_t>::max());
  return static_cast<double>(h) / denom;
}

void EnvironmentWorld::SpatialGrid::rebuild(const std::vector<Creature>& creatures) {
  buckets.clear();
  positions.clear();
  buckets.reserve(creatures.size());
  positions.reserve(creatures.size());
  for (const auto& creature : creatures) {
    const ChunkCoord coord = EnvironmentWorld::to_chunk_coord(creature.position);
    buckets[coord].push_back(creature.id);
    positions[creature.id] = creature.position;
  }
}

} // namespace evo::sim
