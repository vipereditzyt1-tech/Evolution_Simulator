#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "../material.hpp"

namespace evo::sim {

using BlockId = std::uint64_t;

enum class BlockType : std::uint8_t {
  Structural,
  Movement,
  Compute,
  Reactor,
  Collector,
  Sensor,
};

struct DamageState {
  double wear {0.0};
  bool broken {false};
};

struct Block {
  BlockId id {0};
  BlockType type {BlockType::Structural};
  MaterialComposition composition;
  double mass {1.0};
  double durability {100.0};
  double maintenance_energy_cost {0.0};
  double repair_energy_cost {0.0};
  double action_energy_cost {0.0};
  DamageState damage_state {};

  [[nodiscard]] double integrity() const {
    return durability <= 0.0 ? 0.0 : std::max(0.0, 1.0 - damage_state.wear / durability);
  }
};

} // namespace evo::sim
