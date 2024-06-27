#include "translator.hpp"

#include "logger.hpp"
#include "utility/config.hpp"

namespace ts {
    Translator::Translator(OptionsParser::Options options) noexcept
    {
        config::sourcePath = options.sourcePath;
    }

    void Translator::run() {
        Logger::instance().log(config::sourcePath);
    }
}