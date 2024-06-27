#pragma once

#include <string_view>

#include "../src/utility/types.hpp"

namespace ts {
    class Logger {
    public:
        TS_NCM(Logger)

        static Logger& instance() noexcept;

        void log(std::string_view log) const noexcept;
        void err(std::string_view err) const noexcept;

        void flog(const std::string& path, std::string_view log) const noexcept;

        void logHelp() const noexcept;

    private:
        constexpr Logger() = default;
        
        ~Logger() = default;
    };
}