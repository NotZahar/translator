#include "stock_market.hpp"

#include <boost/asio/co_spawn.hpp>

#include "logger.hpp"
#include "router.hpp"
#include "utility/config.hpp"
#include "utility/paths.hpp"

namespace ts {
    StockMarket::StockMarket(OptionsParser::Options options) noexcept
    {
        config::address = ip::make_address(options.address);
        config::port = options.port;
        config::threads = options.threads;
        config::authSecret = options.authSecret;
    }

    void StockMarket::run() {
        if (std::error_code code; !fs::is_regular_file(paths::dbPath, code))
            throw code; 

        asio::io_context ioContext{ config::threads };
        asio::co_spawn(
            ioContext,
            makeListener(),
            [](std::exception_ptr exceptionPtr) {
                if (!exceptionPtr)
                    return;
                
                try {
                    std::rethrow_exception(exceptionPtr);
                } catch(const std::exception& exception) {
                    Logger::instance().err(exception.what());
                }
            }
        );

        asio::detail::thread_group contextRunners;
        for (int i = 0; i < config::threads - 1; ++i)
            contextRunners.create_thread([&ioContext] { ioContext.run(); });
        ioContext.run();
    }

    asio::awaitable<void> StockMarket::makeListener() const {
        auto executor = co_await asio::this_coro::executor;
        auto acceptor = 
            asio::use_awaitable.as_default_on(ip::tcp::acceptor{ executor });
        const auto endpoint = ip::tcp::endpoint{ config::address, config::port };
        
        acceptor.open(endpoint.protocol());
        acceptor.set_option(asio::socket_base::reuse_address(true));
        acceptor.bind(endpoint);
        acceptor.listen();

        co_await acceptor.async_accept();
        while (true) {
            asio::co_spawn(
                acceptor.get_executor(),
                makeSession(tcp_stream(co_await acceptor.async_accept())),
                [](std::exception_ptr exceptionPtr) {
                    if (!exceptionPtr)
                        return;

                    try {
                        std::rethrow_exception(exceptionPtr);
                    } catch (const std::exception& exception) {
                        Logger::instance().err(exception.what());
                    }
                }
            );
        }
    }

    asio::awaitable<void> StockMarket::makeSession(tcp_stream stream) const {    
        beast::flat_buffer buffer;

        try {
            while (true) {
                stream.expires_after(config::sessionTimeout);
                
                http::request<http::string_body> request;
                co_await http::async_read(stream, buffer, request);
                
                auto response = Router::makeResponse(std::move(request));
                const bool keepAlive = response.keep_alive();

                co_await beast::async_write(stream, std::move(response), asio::use_awaitable);
                if(!keepAlive)
                    break;
            }
        } catch (const boost::system::system_error& error) {
            if (error.code() != http::error::end_of_stream)
                throw;
        }

        beast::error_code errorCode;
        [[maybe_unused]] auto code = 
            stream.socket().shutdown(ip::tcp::socket::shutdown_send, errorCode);
    }
}