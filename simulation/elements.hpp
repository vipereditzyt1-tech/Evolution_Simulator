#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace evo::sim {

using ElementId = std::uint32_t;

struct Color {
  float r {0.0F};
  float g {0.0F};
  float b {0.0F};
};

struct Element {
  ElementId id {0};
  std::string name;
  double density {0.0};
  double hardness {0.0};
  double conductivity {0.0};
  double reactivity {0.0};
  double energy_potential {0.0};
  Color base_color {};
};

class ElementRegistry {
 public:
  static constexpr std::size_t kBaseElementCount = 50;

  ElementRegistry();

  [[nodiscard]] const Element& get(ElementId id) const;
  [[nodiscard]] const std::vector<Element>& all() const noexcept;

 private:
  std::vector<Element> elements_;
};

} // namespace evo::sim
