#include "render_loop.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace evo::engine {

namespace {
double clamp_pitch(const double pitch) {
  return std::clamp(pitch, -1.5, 1.5);
}

sim::SpatialPosition block_world_position(const sim::Creature& creature, std::size_t block_index) {
  return sim::SpatialPosition {
    creature.position.x + static_cast<double>(block_index) * 0.25,
    creature.position.y,
    creature.position.z,
  };
}

} // namespace

void OrbitCamera::orbit(const double delta_yaw, const double delta_pitch) {
  yaw_radians += delta_yaw;
  pitch_radians = clamp_pitch(pitch_radians + delta_pitch);
}

void OrbitCamera::pan(const double delta_x, const double delta_y, const double delta_z) {
  target.x += delta_x;
  target.y += delta_y;
  target.z += delta_z;
}

void OrbitCamera::zoom(const double delta_distance) {
  distance = std::max(1.0, distance + delta_distance);
}

RenderLoop::RenderLoop(sim::ElementRegistry registry) : registry_(std::move(registry)) {}

void RenderLoop::set_selected_creature(const std::optional<sim::CreatureId> creature_id) {
  selected_creature_ = creature_id;
}

std::optional<sim::CreatureId> RenderLoop::selected_creature() const {
  return selected_creature_;
}

std::optional<sim::CreatureId> RenderLoop::pick_creature(const std::vector<sim::Creature>& creatures,
                                                         const sim::SpatialPosition& near_world_position,
                                                         const double max_distance) const {
  double best_distance = std::numeric_limits<double>::max();
  std::optional<sim::CreatureId> best_id;

  for (const auto& creature : creatures) {
    const double dx = creature.position.x - near_world_position.x;
    const double dy = creature.position.y - near_world_position.y;
    const double dz = creature.position.z - near_world_position.z;
    const double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (distance <= max_distance && distance < best_distance) {
      best_distance = distance;
      best_id = creature.id;
    }
  }

  return best_id;
}

RenderFrame RenderLoop::build_frame(const std::vector<sim::Creature>& creatures, const OrbitCamera& camera) const {
  RenderFrame frame;
  frame.camera = camera;
  frame.highlighted_creature = selected_creature_;

  for (const auto& creature : creatures) {
    for (std::size_t i = 0; i < creature.blocks.size(); ++i) {
      const auto& block = creature.blocks[i];
      RenderedBlock rendered;
      rendered.creature_id = creature.id;
      rendered.block_id = block.id;
      rendered.world_position = block_world_position(creature, i);
      rendered.color = block.composition.derived_color(registry_);
      rendered.selected = selected_creature_.has_value() && selected_creature_ == creature.id;
      frame.blocks.push_back(rendered);
    }
  }

  return frame;
}

} // namespace evo::engine
