#pragma once

#include "block.hpp"

namespace evo::sim {

struct StructuralBlock : public Block {
  double load_capacity {100.0};
};

struct MovementBlock : public Block {
  double stretch_range {0.0};
  double twist_range_degrees {0.0};
};

struct ComputeBlock : public Block {
  std::uint32_t logic_units {1};
};

struct ReactorBlock : public Block {
  double conversion_efficiency {0.0};
};

struct CollectorBlock : public Block {
  double collection_rate {0.0};
};

struct SensorBlock : public Block {
  double sensing_radius {0.0};
};

} // namespace evo::sim
