#include "translator.hpp"

#include "core/source_file.hpp"
#include "utility/config.hpp"

namespace ts {
    Translator::Translator(OptionsParser::Options options) noexcept
    {
        config::sourcePath = options.sourcePath;
    }

    void Translator::translate() {
        SourceFile src{ config::sourcePath };
        auto uFormat = src.toUFormat();
        
    }

    std::string_view Translator::getTranslated() const noexcept {
        return _translated;
    }
}