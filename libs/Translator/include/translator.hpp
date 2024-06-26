#pragma once

#include "options_parser.hpp"

namespace ts {
    /*!
        \brief Отвечает за запуск сервера

        Точка входа в приложение, отвечает за создание сетевых подключений
    */
    class StockMarket {
    public:
        StockMarket() = delete;
        explicit StockMarket(OptionsParser::Options options) noexcept;
        
        ~StockMarket() = default;

        void run();
    
    private:
        asio::awaitable<void> makeListener() const;
        asio::awaitable<void> makeSession(tcp_stream stream) const;
    };
}
