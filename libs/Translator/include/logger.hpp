#pragma once

#include <string_view>

#include "../src/utility/types.hpp"

namespace ts {
    class Logger {
    public:
        TS_NCM(Logger)

        static Logger& instance();

        void log(std::string_view log);
        void err(std::string_view err);

        void logHelp();

    private:
        Logger() = default;
        
        ~Logger() = default;
    };
}