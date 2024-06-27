#pragma once

#include <boost/program_options.hpp>

#include "../src/utility/config.hpp"

namespace ts {
    class OptionsParser {
    public:
        struct Options {
            bool help = false;
            std::string sourcePath = config::sourcePath;
        };

        OptionsParser() = delete;
        OptionsParser(int argc, char** argv);
        
        ~OptionsParser() = default;

        Options getOptions() const;

    private:
        void initOptions();
        Options parseOptions(int argc, char** argv) const;

        boost::program_options::options_description _description;
        Options _options;
    };
}
