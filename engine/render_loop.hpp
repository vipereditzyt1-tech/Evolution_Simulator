#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "simulation/creatures/creature.hpp"
#include "simulation/elements.hpp"

namespace evo::engine {

struct OrbitCamera {
  double yaw_radians {0.0};
  double pitch_radians {0.5};
  double distance {20.0};
  sim::SpatialPosition target {0.0, 0.0, 0.0};

  void orbit(double delta_yaw, double delta_pitch);
  void pan(double delta_x, double delta_y, double delta_z = 0.0);
  void zoom(double delta_distance);
};

struct RenderedBlock {
  sim::CreatureId creature_id {0};
  sim::BlockId block_id {0};
  sim::SpatialPosition world_position;
  sim::Color color;
  bool selected {false};
};

struct RenderFrame {
  OrbitCamera camera;
  std::vector<RenderedBlock> blocks;
  std::optional<sim::CreatureId> highlighted_creature;
};

class RenderLoop {
public:
  explicit RenderLoop(sim::ElementRegistry registry = sim::ElementRegistry());

  void set_selected_creature(std::optional<sim::CreatureId> creature_id);
  [[nodiscard]] std::optional<sim::CreatureId> selected_creature() const;

  [[nodiscard]] std::optional<sim::CreatureId> pick_creature(const std::vector<sim::Creature>& creatures,
                                                             const sim::SpatialPosition& near_world_position,
                                                             double max_distance) const;

  [[nodiscard]] RenderFrame build_frame(const std::vector<sim::Creature>& creatures,
                                        const OrbitCamera& camera) const;

private:
  sim::ElementRegistry registry_;
  std::optional<sim::CreatureId> selected_creature_;
};

} // namespace evo::engine
