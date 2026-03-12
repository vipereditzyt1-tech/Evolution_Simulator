#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "../blocks/block.hpp"

namespace evo::sim {

using GeneId = std::uint64_t;
using GenomeId = std::uint64_t;

struct ActivationCondition {
  double min_energy {0.0};
  double max_damage {1.0};
};

struct Gene {
  GeneId id {0};
  BlockType block_type {BlockType::Structural};
  MaterialComposition composition;
  std::array<int, 3> relative_coordinates {0, 0, 0};
  std::uint32_t build_order {0};
  std::uint32_t priority {0};
  ActivationCondition activation_conditions {};
  std::vector<double> parameters;
};

struct Genome {
  GenomeId id {0};
  std::vector<Gene> genes;
};

} // namespace evo::sim
