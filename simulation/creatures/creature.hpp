#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "genome.hpp"

namespace evo::sim {

using CreatureId = std::uint64_t;

struct BlockEdge {
  BlockId from {0};
  BlockId to {0};
};

struct InventoryItem {
  ElementId element_id {0};
  double quantity {0.0};
};

struct NeuralNetworkState {
  std::vector<double> neurons;
  std::vector<double> activations;
};

struct CreatureHistory {
  std::vector<std::string> events;
};

struct Creature {
  CreatureId id {0};
  std::vector<Block> blocks;
  std::vector<BlockEdge> block_graph;
  Genome genome;
  std::vector<InventoryItem> inventory;
  NeuralNetworkState neural_network_state;
  double energy_pool {0.0};
  std::uint64_t age_ticks {0};
  CreatureHistory history;

  [[nodiscard]] std::vector<Block*> sorted_blocks() {
    std::vector<Block*> ordered;
    ordered.reserve(blocks.size());
    for (auto& b : blocks) {
      ordered.push_back(&b);
    }
    std::sort(ordered.begin(), ordered.end(), [](const Block* lhs, const Block* rhs) {
      return lhs->id < rhs->id;
    });
    return ordered;
  }
};

} // namespace evo::sim
