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

        OptionsParser() noexcept;
        
        ~OptionsParser() = default;

        [[nodiscard]] Options parseOptions(int argc, char** argv) const;

    private:
        void initOptions() noexcept;

        boost::program_options::options_description _description;
    };
}
