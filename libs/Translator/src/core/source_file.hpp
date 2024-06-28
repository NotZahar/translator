#pragma once

#include <filesystem>
#include <string_view>

#include "universal_format.hpp"

namespace ts {
    class SourceFile {
    public:
        SourceFile() = delete;
        explicit SourceFile(std::string_view path) noexcept;
        
        virtual ~SourceFile() = default;

        virtual UFormat toUFormat() const = 0;

    protected:
        std::filesystem::path _sourcePath;        
    };
}
