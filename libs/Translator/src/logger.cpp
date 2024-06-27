#include "logger.hpp"

#include <iostream>
#include <format>
#include <fstream>

#include "utility/messages.hpp"

namespace ts {
    Logger& Logger::instance() noexcept {
        static Logger instance;
        return instance;
    }

    void Logger::log(std::string_view log) const noexcept {
        std::cout << std::format("{}\n", log);
    }

    void Logger::err(std::string_view err) const noexcept {
        std::cerr << std::format("{}:\n  {}\n", messages::errors::GENERAL_ERROR, err);
    }

    void Logger::flog(const std::string& path, std::string_view log) const noexcept {
        std::ofstream outFile;
        outFile.open(path);
        if (!outFile) {
            err(std::format("{}: {}", messages::errors::CANT_OPEN_FILE, path));
            return;
        } 

        outFile << log;
        outFile.close();
    }

    void Logger::logHelp() const noexcept {
        log(messages::general::HELP_MESSAGE);
    }
}