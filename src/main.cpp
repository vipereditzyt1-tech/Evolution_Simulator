#include <iostream>

#include "app.hpp"
#include "core.hpp"

int main() {
  std::cout << evo::version_string() << '\n';
  evo::App app;
  app.run(3);
  return 0;
}
