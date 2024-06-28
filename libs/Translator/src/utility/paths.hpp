#pragma once

#include <filesystem>

namespace ts {
    struct paths {
        inline static const std::filesystem::path resourcesPath = std::filesystem::current_path() / "resources";
        inline static const std::filesystem::path sourcePath = resourcesPath / "example.xml";
    };
}