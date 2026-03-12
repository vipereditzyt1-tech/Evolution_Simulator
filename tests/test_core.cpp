#include <gtest/gtest.h>

#include "core.hpp"

TEST(Core, VersionStringIsNotEmpty) {
  EXPECT_FALSE(evo::version_string().empty());
}
