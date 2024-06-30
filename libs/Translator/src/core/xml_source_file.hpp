#pragma once

#include "source_file.hpp"

namespace ts {
    class XmlSourceFile final : public SourceFile {
    public:
        explicit XmlSourceFile(std::string_view path) noexcept;
        
        ~XmlSourceFile() override = default;

        structures::SElements getElements() const override;
    };
}
