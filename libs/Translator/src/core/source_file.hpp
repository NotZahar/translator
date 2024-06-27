#pragma once

#include <filesystem>
#include <string_view>

#include "universal_format.hpp"

namespace ts {
    class SourceFile {
    public:
        SourceFile() = delete;
        explicit SourceFile(std::string_view path) noexcept;
        
        ~SourceFile() = default;

        UFormat toUFormat() const;

    private:
        std::filesystem::path _sourcePath;        
    };
}
