#pragma once

#include <string>

#include "paths.hpp"

namespace ts {
    struct config {
        inline static std::string sourcePath = paths::sourcePath;
    };
}