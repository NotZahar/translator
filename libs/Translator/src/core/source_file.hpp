#pragma once

#include <filesystem>
#include <string_view>

#include "structures/uelements.hpp"

namespace ts {
    class SourceFile {
    public:
        SourceFile() = delete;
        explicit SourceFile(std::string_view path) noexcept;
        
        virtual ~SourceFile() = default;

        virtual structures::UElements getElements() const = 0;

    protected:
        std::filesystem::path _sourcePath;        
    };
}
