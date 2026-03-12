#include "elements.hpp"

#include <stdexcept>

namespace evo::sim {

ElementRegistry::ElementRegistry() {
  elements_.reserve(kBaseElementCount);
  for (std::size_t i = 0; i < kBaseElementCount; ++i) {
    const auto id = static_cast<ElementId>(i);
    const double t = static_cast<double>(i) / static_cast<double>(kBaseElementCount - 1);
    Element e;
    e.id = id;
    e.name = "Element_" + std::to_string(i + 1);
    e.density = 0.5 + (t * 19.5);
    e.hardness = 1.0 + (t * 9.0);
    e.conductivity = t;
    e.reactivity = 1.0 - t;
    e.energy_potential = 0.2 + (t * 4.8);
    e.base_color = Color {
      static_cast<float>(0.2 + (0.8 * t)),
      static_cast<float>(0.1 + (0.7 * (1.0 - t))),
      static_cast<float>(0.3 + (0.5 * (0.5 + t / 2.0))),
    };
    elements_.push_back(e);
  }
}

const Element& ElementRegistry::get(const ElementId id) const {
  if (id >= elements_.size()) {
    throw std::out_of_range("Element id out of range");
  }
  return elements_[id];
}

const std::vector<Element>& ElementRegistry::all() const noexcept {
  return elements_;
}

} // namespace evo::sim
