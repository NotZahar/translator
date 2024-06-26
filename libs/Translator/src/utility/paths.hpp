#pragma once

#include <filesystem>

#include "types.hpp"

namespace ts {
    struct paths {
        inline static const fs::path resourcesPath = fs::current_path() / "resources";
        inline static const fs::path dbPath = resourcesPath / "db/sm.db3";
    };
}