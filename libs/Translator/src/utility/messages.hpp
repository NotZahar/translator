#pragma once

#include <format>

namespace ts {
    struct messages {
        struct options {
            inline static const std::string HELP_F = "help";
            inline static const std::string HELP_S = "h";
            inline static const std::string HELP = std::format("{},{}", HELP_F, HELP_S);
            inline static const std::string HELP_DESCR = "печать описания параметров";
            
            inline static const std::string ADDRESS_F = "address";
            inline static const std::string ADDRESS_S = "a";
            inline static const std::string ADDRESS = std::format("{},{}", ADDRESS_F, ADDRESS_S);
            inline static const std::string ADDRESS_DESCR = "ip-адрес";

            inline static const std::string PORT_F = "port";
            inline static const std::string PORT_S = "p";
            inline static const std::string PORT = std::format("{},{}", PORT_F, PORT_S);
            inline static const std::string PORT_DESCR = "порт";

            inline static const std::string THREADS_F = "threads";
            inline static const std::string THREADS_S = "t";
            inline static const std::string THREADS = std::format("{},{}", THREADS_F, THREADS_S);
            inline static const std::string THREADS_DESCR = "кол-во потоков";
        };

        struct general {
            inline static const std::string HELP_HEADER = "Параметры";
            inline static const std::string HELP_MESSAGE = std::format(
    "{}:\n \
    -{} [ --{:8} ]        {}\n \
    -{} [ --{:8} ] arg    {}\n \
    -{} [ --{:8} ] arg    {}\n \
    -{} [ --{:8} ] arg    {}\n \
                ",
                HELP_HEADER,
                options::HELP_S, options::HELP_F, options::HELP_DESCR,
                options::ADDRESS_S, options::ADDRESS_F, options::ADDRESS_DESCR,
                options::PORT_S, options::PORT_F, options::PORT_DESCR,
                options::THREADS_S, options::THREADS_F, options::THREADS_DESCR
            );
        };

        struct errors {
            // http error responses
            inline static const std::string INVALID_METHOD = "Недопустимый HTTP-метод";
            inline static const std::string INVALID_REQUEST = "Недопустимый запрос";
            inline static const std::string INVALID_AUTH_CREATE = "Ошибка создания пользователя";
            inline static const std::string INVALID_AUTH_LOGIN = "Неверные данные";
            inline static const std::string INVALID_AUTH = "Ошибка аутентификации";
            inline static const std::string INTERNAL_ERROR_GENERAL = "Возникла ошибка";
            inline static const std::string INTERNAL_ERROR = "Возникла ошибка: ";
        };

        struct success {
            // http success responses
            inline static const std::string USER_CREATED = "Пользователь успешно создан";
            inline static const std::string USER_LOGGED = "Пользователь авторизован";
            inline static const std::string PURCHASED = "Заявка на покупку успешно создана";
            inline static const std::string SOLD = "Заявка на продажу успешно создана";
        };
    };
}