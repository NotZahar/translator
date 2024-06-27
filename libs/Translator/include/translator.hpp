#pragma once

#include "options_parser.hpp"

namespace ts {
    class Translator {
    public:
        Translator() = delete;
        explicit Translator(OptionsParser::Options options) noexcept;
        
        ~Translator() = default;

        void run();
    
    private:
    
    };
}
