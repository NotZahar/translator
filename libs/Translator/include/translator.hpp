#pragma once

#include <string_view>

#include "options_parser.hpp"

namespace ts {
    class Translator {
    public:
        Translator() = delete;
        explicit Translator(OptionsParser::Options options) noexcept;
        
        ~Translator() = default;

        void translate();
        std::string_view getTranslated() const noexcept;

    private:
        std::string _translated;
    };
}
