#include <cstdint>
#include <string>

#include "app.hpp"
#include "core.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace {

evo::App& app_instance() {
  static evo::App app;
  return app;
}

} // namespace

extern "C" {

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
const char* evo_version() {
  static std::string version = evo::version_string();
  return version.c_str();
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void evo_run_frames(const int frames) {
  app_instance().run(frames > 0 ? static_cast<std::uint64_t>(frames) : 0U);
}

}
