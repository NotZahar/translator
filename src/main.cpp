#include <translator.hpp>
#include <options_parser.hpp>
#include <logger.hpp>

int main(int argc, char** argv) {
    ts::OptionsParser optionsParser(argc, argv);
    const auto options = optionsParser.getOptions();
    if (options.help) {
        ts::Logger::instance().logHelp();
        return 0;
    }

    ts::Translator translator{ std::move(options) };
    translator.run();

    return 0;
}
