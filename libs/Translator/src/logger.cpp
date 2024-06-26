#include "logger.hpp"

#include <iostream>

#include "utility/messages.hpp"

namespace ts {
    Logger& Logger::instance() {
        static Logger instance;
        return instance;
    }

    void Logger::log(std::string_view log) {
        std::cout << log << '\n';
    }

    void Logger::err(std::string_view err) {
        std::cerr << err << '\n';
    }

    void Logger::logHelp() {
        log(messages::general::HELP_MESSAGE);
    }
}