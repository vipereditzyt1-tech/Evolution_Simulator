#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include "elements.hpp"

namespace evo::sim {

struct MaterialComponent {
  ElementId element_id {0};
  double weight {0.0};
};

struct MaterialComposition {
  std::vector<MaterialComponent> components;

  void normalize() {
    double sum = 0.0;
    for (const auto& c : components) {
      sum += std::max(0.0, c.weight);
    }
    if (sum <= 0.0) {
      components.clear();
      return;
    }
    for (auto& c : components) {
      c.weight = std::max(0.0, c.weight) / sum;
    }
  }

  [[nodiscard]] Color derived_color(const ElementRegistry& registry) const {
    Color color {};
    for (const auto& component : components) {
      const auto& e = registry.get(component.element_id);
      color.r += static_cast<float>(e.base_color.r * component.weight);
      color.g += static_cast<float>(e.base_color.g * component.weight);
      color.b += static_cast<float>(e.base_color.b * component.weight);
    }
    return color;
  }
};

} // namespace evo::sim
