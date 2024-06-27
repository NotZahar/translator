#include <cstdlib>
#include <exception>

#include <translator.hpp>
#include <options_parser.hpp>
#include <logger.hpp>

int main(int argc, char** argv) {
    auto& logger = ts::Logger::instance();
    
    ts::OptionsParser optionsParser{};
    const auto options = optionsParser.parseOptions(argc, argv);
    if (options.help) {
        logger.logHelp();
        return EXIT_SUCCESS;
    }

    ts::Translator translator{ std::move(options) };
    try {
        translator.translate();
    } catch (const std::exception& exception) {
        logger.err(exception.what());
        return EXIT_FAILURE;
    }

    logger.flog("out.c", translator.getTranslated());

    return EXIT_SUCCESS;
}
