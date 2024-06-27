#include "options_parser.hpp"

#include <boost/program_options.hpp>

#include "utility/messages.hpp"

namespace ts {
    namespace po = boost::program_options;

    OptionsParser::OptionsParser() noexcept
        : _description(messages::general::HELP_HEADER)
    {
        initOptions();
    }

    void OptionsParser::initOptions() noexcept {
        _description.add_options()
            (
                messages::options::HELP.c_str(), 
                messages::options::HELP_DESCR.c_str()
            )
            (
                messages::options::SOURCE.c_str(),
                po::value<decltype(Options::sourcePath)>(),
                messages::options::SOURCE_DESCR.c_str()
            )
        ;
    }

    OptionsParser::Options OptionsParser::parseOptions(int argc, char** argv) const{
        Options options;
        po::variables_map variablesMap;
        po::store(
            po::parse_command_line(argc, argv, _description),
            variablesMap);
        po::notify(variablesMap);

        if (variablesMap.count(messages::options::HELP_F)) {
            options.help = true;
            return options;
        }

        if (variablesMap.count(messages::options::SOURCE_F))
            options.sourcePath = 
                variablesMap[messages::options::SOURCE_F].as<decltype(Options::sourcePath)>();

        return options;
    }
}