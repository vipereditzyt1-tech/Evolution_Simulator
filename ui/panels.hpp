#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "simulation/creatures/creature.hpp"

namespace evo::ui {

enum class OverlayMode : std::uint8_t {
  None,
  ResourceDensity,
  Temperature,
  Energy,
  Pressure,
};

struct SimulationControls {
  bool paused {false};
  double speed_multiplier {1.0};
  bool reset_requested {false};

  void pause();
  void resume();
  void set_speed(double speed);
  void request_reset();
};

struct SaveLoadActions {
  std::optional<std::string> save_path;
  std::optional<std::string> load_path;
};

struct NeuralGraphSummary {
  std::size_t neuron_count {0};
  std::size_t activation_count {0};
};

struct CreatureInspector {
  std::optional<sim::CreatureId> focused_creature;
  std::string genome_summary;
  std::string block_layout_summary;
  NeuralGraphSummary neural_graph;
  double energy {0.0};
  std::uint64_t age_ticks {0};
  std::vector<std::string> reproduction_history;

  void update_from(const sim::Creature& creature);
};

struct StatisticsPoint {
  std::uint64_t tick {0};
  std::size_t population {0};
  double diversity {0.0};
  std::size_t extinctions {0};
  double average_genome_size {0.0};
};

struct StatisticsPanel {
  std::size_t population {0};
  double diversity {0.0};
  std::size_t extinctions {0};
  double average_genome_size {0.0};
  std::vector<StatisticsPoint> timeline;

  void append_timeline_point(const StatisticsPoint& point);
};

class UiState {
public:
  SimulationControls controls;
  SaveLoadActions save_load;
  CreatureInspector inspector;
  OverlayMode overlay_mode {OverlayMode::None};
  StatisticsPanel statistics;

  void set_overlay_mode(OverlayMode mode);
};

} // namespace evo::ui
