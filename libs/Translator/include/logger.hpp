#pragma once

#include "../src/utility/types.hpp"

namespace ts {
    /*!
        \brief Логгер

        Синглтон
    */
    class Logger final {
    public:
        SM_NCM(Logger)

        static Logger& instance();

        void log(std::string_view log);
        void err(std::string_view err);

        void logHelp();

    private:
        Logger() = default;
        
        ~Logger() = default;
    };
}