#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "simulation/creatures/creature.hpp"

namespace evo::sim {

struct Int3 {
  int x {0};
  int y {0};
  int z {0};
};


struct EnvironmentSensorSample {
  SpatialPosition temperature_gradient;
  SpatialPosition pressure_gradient;
  double local_rainfall_density {0.0};
  double local_material_density {0.0};
};

class EnvironmentWorld {
public:
  static constexpr int kChunkSize = 16;

  explicit EnvironmentWorld(std::uint32_t seed = 1);

  void tick(std::uint64_t tick_index, const std::vector<Creature>& creatures,
            std::vector<InventoryItem>& recycled_materials);

  [[nodiscard]] EnvironmentSensorSample query_environment(const SpatialPosition& world_position, ElementId element,
                                                          int radius = 2) const;

  [[nodiscard]] std::size_t loaded_chunk_count() const;

private:
  struct ChunkCoord {
    int x {0};
    int y {0};
    int z {0};

    bool operator==(const ChunkCoord& other) const {
      return x == other.x && y == other.y && z == other.z;
    }
  };

  struct ChunkCoordHash {
    std::size_t operator()(const ChunkCoord& coord) const;
  };

  struct Cell {
    double terrain_height {0.0};
    double temperature {0.0};
    double pressure {1.0};
    double rainfall {0.0};
    double chemical_weathering {0.0};
    double geological_compression {0.0};
    std::unordered_map<ElementId, double> materials;
  };

  struct Chunk {
    ChunkCoord coord;
    std::vector<Cell> cells;

    Chunk();
  };

  struct SpatialGrid {
    std::unordered_map<ChunkCoord, std::vector<CreatureId>, ChunkCoordHash> buckets;

    void rebuild(const std::vector<Creature>& creatures);
  };

  [[nodiscard]] static ChunkCoord to_chunk_coord(const SpatialPosition& position);
  [[nodiscard]] static Int3 to_local_index(const SpatialPosition& position);
  [[nodiscard]] static std::size_t cell_index(int x, int y, int z);

  Chunk& ensure_chunk(const ChunkCoord& coord);
  const Chunk* find_chunk(const ChunkCoord& coord) const;
  void generate_terrain_and_deposits(Chunk& chunk);

  void apply_rainfall(Chunk& chunk, std::uint64_t tick_index);
  void apply_temperature(Chunk& chunk, std::uint64_t tick_index);
  void apply_pressure(Chunk& chunk, std::uint64_t tick_index);
  void apply_chemical_weathering(Chunk& chunk, std::vector<InventoryItem>& recycled_materials);
  void apply_geological_compression(Chunk& chunk);
  void apply_resource_diffusion(Chunk& chunk);

  [[nodiscard]] double seeded_noise(int x, int y, int z, std::uint64_t salt = 0) const;

  std::uint32_t seed_;
  std::unordered_map<ChunkCoord, Chunk, ChunkCoordHash> chunks_;
  SpatialGrid creature_grid_;
};

} // namespace evo::sim
