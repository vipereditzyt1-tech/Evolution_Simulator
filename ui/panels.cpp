#include "panels.hpp"

#include <algorithm>
#include <sstream>

namespace evo::ui {

void SimulationControls::pause() {
  paused = true;
}

void SimulationControls::resume() {
  paused = false;
}

void SimulationControls::set_speed(const double speed) {
  speed_multiplier = std::clamp(speed, 0.1, 64.0);
}

void SimulationControls::request_reset() {
  reset_requested = true;
}

void CreatureInspector::update_from(const sim::Creature& creature) {
  focused_creature = creature.id;

  std::ostringstream genome;
  genome << "Genome " << creature.genome.id << " genes=" << creature.genome.genes.size();
  genome_summary = genome.str();

  std::ostringstream blocks;
  blocks << "blocks=" << creature.blocks.size() << " edges=" << creature.block_graph.size();
  block_layout_summary = blocks.str();

  neural_graph = NeuralGraphSummary {
    .neuron_count = creature.neural_network_state.neurons.size(),
    .activation_count = creature.neural_network_state.activations.size(),
  };

  energy = creature.energy_pool;
  age_ticks = creature.age_ticks;

  reproduction_history.clear();
  for (const auto& event : creature.history.events) {
    if (event.find("reproduction") != std::string::npos || event == "born") {
      reproduction_history.push_back(event);
    }
  }
}

void StatisticsPanel::append_timeline_point(const StatisticsPoint& point) {
  timeline.push_back(point);
  population = point.population;
  diversity = point.diversity;
  extinctions = point.extinctions;
  average_genome_size = point.average_genome_size;
}

void UiState::set_overlay_mode(const OverlayMode mode) {
  overlay_mode = mode;
}

} // namespace evo::ui
