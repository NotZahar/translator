#include "source_file.hpp"

namespace ts {
    SourceFile::SourceFile(std::string_view path) noexcept 
        : _sourcePath{ path }
    {

    }
}