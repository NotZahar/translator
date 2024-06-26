#include "options_parser.hpp"

#include "utility/messages.hpp"

namespace ts {
    namespace po = boost::program_options;

    OptionsParser::OptionsParser(int argc, char** argv)
        : _description(messages::general::HELP_HEADER)
    {
        initOptions();
        _options = parseOptions(argc, argv);
    }

    OptionsParser::Options OptionsParser::getOptions() const {
        return _options;
    }

    void OptionsParser::initOptions() {
        _description.add_options()
            (
                messages::options::HELP.c_str(), 
                messages::options::HELP_DESCR.c_str()
            )
            (
                messages::options::ADDRESS.c_str(),
                po::value<decltype(Options::address)>(),
                messages::options::ADDRESS_DESCR.c_str()
            )
            (
                messages::options::PORT.c_str(),
                po::value<decltype(Options::port)>(),
                messages::options::PORT_DESCR.c_str()
            )
            (
                messages::options::THREADS.c_str(),
                po::value<decltype(Options::threads)>(),
                messages::options::THREADS_DESCR.c_str()
            )
        ;
    }

    OptionsParser::Options OptionsParser::parseOptions(int argc, char** argv) const {
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

        if (variablesMap.count(messages::options::ADDRESS_F))
            options.address = 
                variablesMap[messages::options::ADDRESS_F].as<decltype(Options::address)>();
        
        if (variablesMap.count(messages::options::PORT_F))
            options.port = 
                variablesMap[messages::options::PORT_F].as<decltype(Options::port)>();

        if (variablesMap.count(messages::options::THREADS_F))
            options.threads = 
                variablesMap[messages::options::THREADS_F].as<decltype(Options::threads)>();

        return options;
    }
}