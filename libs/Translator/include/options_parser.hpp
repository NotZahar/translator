#pragma once

#include "../src/utility/config.hpp"

namespace ts {
    /*!
        \brief Опции приложения

        Задаются на этапе запуска сервера
    */
    class OptionsParser {
    public:
        struct Options {
            bool help = false;
            std::string address = config::defaultAddress;
            unsigned short port = config::defaultPort;
            int threads = config::defaultThreads;
            std::string authSecret = config::defaultAuthSecret;
        };

        OptionsParser() = delete;
        OptionsParser(int argc, char** argv);
        
        ~OptionsParser() = default;

        Options getOptions() const;

    private:
        void initOptions();
        Options parseOptions(int argc, char** argv) const;

        options_description _description;
        Options _options;
    };
}
