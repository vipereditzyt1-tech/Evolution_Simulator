#pragma once

#include <cstdint>
#include <vector>

#include "engine/render_loop.hpp"
#include "simulation/systems.hpp"
#include "ui/panels.hpp"

namespace evo {

class App {
public:
  App();

  void run(std::uint64_t frames);

private:
  void step_simulation(double frame_dt);
  void sync_ui();

  std::uint64_t tick_ {0};
  sim::FixedTimestepRunner timestep_ {};
  double interpolation_alpha_ {0.0};
  sim::SimulationEnvironment environment_;
  std::vector<sim::Creature> creatures_;
  engine::OrbitCamera camera_;
  engine::RenderLoop renderer_;
  ui::UiState ui_state_;
};

} // namespace evo
