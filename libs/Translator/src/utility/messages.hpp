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
            inline static constexpr char NO_BLOCK_TYPE[] = "Тип блока не определен";
            inline static constexpr char INVALID_BLOCK_TYPE[] = "Несуществующий тип блока";
            inline static constexpr char INVALID_SUM_BLOCK[] = "Блок типа Sum невалиден";
            inline static constexpr char INVALID_GAIN_BLOCK[] = "Блок типа Gain невалиден";
            inline static constexpr char INVALID_UNIT_DELAY_BLOCK[] = "Блок типа UnitDelay невалиден";
            inline static constexpr char INVALID_LINE_BLOCK[] = "Блок типа Line невалиден";
            inline static constexpr char INVALID_LINK[] = "Встречена невалидная связь";
            inline static constexpr char NO_INPUT_VARS[] = "Нет входных переменных";
            inline static constexpr char NO_OUTPUT_VARS[] = "Нет выходных переменных";
            inline static constexpr char NO_INPUT_LINK[] = "Нет связи, исходящей из Inport блока";
            inline static constexpr char WRONG_BLOCK_TYPE[] = "Встречен невалидный тип блока при создании порядка команд";
        };
    };
}