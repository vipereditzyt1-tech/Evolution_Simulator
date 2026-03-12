#include "app.hpp"

#include <iostream>

namespace evo {
namespace {

sim::Creature seed_creature() {
  sim::Creature creature;
  creature.id = 1;
  creature.genome.id = 1;
  creature.energy_pool = 100.0;
  creature.history.events = {"born", "asexual_reproduction"};

  sim::Block block;
  block.id = 1;
  block.type = sim::BlockType::Structural;
  block.composition.components = {{6, 0.6}, {26, 0.4}};
  block.composition.normalize();
  creature.blocks.push_back(block);

  return creature;
}

} // namespace

App::App() : renderer_(sim::ElementRegistry()) {
  creatures_.push_back(seed_creature());
}

void App::run(const std::uint64_t frames) {
  for (std::uint64_t frame = 0; frame < frames; ++frame) {
    constexpr double frame_dt = 1.0 / 30.0;
    step_simulation(frame_dt);
    sync_ui();

    if (frame == 0) {
      camera_.orbit(0.15, -0.05);
      camera_.zoom(-1.0);
      camera_.pan(0.5, 0.0);
      renderer_.set_selected_creature(renderer_.pick_creature(creatures_, sim::SpatialPosition {0.0, 0.0, 0.0}, 10.0));
    }

    const auto rendered = renderer_.build_frame(creatures_, camera_);
    std::cout << "frame=" << frame << " blocks=" << rendered.blocks.size() << " selected="
              << (rendered.highlighted_creature.has_value() ? std::to_string(*rendered.highlighted_creature) : std::string("none"))
              << " alpha=" << interpolation_alpha_ << '\n';
  }
}

void App::step_simulation(const double frame_dt) {
  if (ui_state_.controls.paused) {
    return;
  }

  timestep_.consume(frame_dt, [&](const double fixed_dt) {
    sim::TickContext ctx {.tick_index = tick_, .deterministic_seed = 42, .dt_seconds = fixed_dt};
    sim::run_tick(creatures_, environment_, ctx);
    ++tick_;
  });
  interpolation_alpha_ = timestep_.interpolation_alpha();
}

void App::sync_ui() {
  ui_state_.controls.set_speed(1.0);
  ui_state_.set_overlay_mode(ui::OverlayMode::Temperature);

  if (!creatures_.empty()) {
    ui_state_.inspector.update_from(creatures_.front());
    ui_state_.statistics.append_timeline_point(ui::StatisticsPoint {
      .tick = tick_,
      .population = creatures_.size(),
      .diversity = static_cast<double>(creatures_.front().genome.genes.size()),
      .extinctions = 0,
      .average_genome_size = static_cast<double>(creatures_.front().genome.genes.size()),
    });
  }
}

} // namespace evo
