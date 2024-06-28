#pragma once

#include <string>
#include <format>

namespace ts {
    struct messages {
        struct options {
            inline static const std::string HELP_F = "help";
            inline static const std::string HELP_S = "h";
            inline static const std::string HELP = std::format("{},{}", HELP_F, HELP_S);
            inline static const std::string HELP_DESCR = "печать описания параметров";
            
            inline static const std::string SOURCE_F = "source";
            inline static const std::string SOURCE_S = "s";
            inline static const std::string SOURCE = std::format("{},{}", SOURCE_F, SOURCE_S);
            inline static const std::string SOURCE_DESCR = "входной файл";
        };

        struct general {
            inline static const std::string HELP_HEADER = "Параметры";
            inline static const std::string HELP_MESSAGE = std::format(
    "{}:\n \
    -{} [ --{:8} ]        {}\n \
    -{} [ --{:8} ] arg    {}\n \
                ",
                HELP_HEADER,
                options::HELP_S, options::HELP_F, options::HELP_DESCR,
                options::SOURCE_S, options::SOURCE_F, options::SOURCE_DESCR
            );
        };

        struct errors {
            inline static constexpr char GENERAL_ERROR[] = "ERROR";
            inline static constexpr char CANT_OPEN_FILE[] = "Файл с таким именем не может быть использован";
            inline static constexpr char INVAILD_SOURCE[] = "Данный файл не может являться входным";
            inline static constexpr char NO_SOURCE[] = "Входной файл не найден";
        };
    };
}