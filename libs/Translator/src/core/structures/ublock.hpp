#pragma once

#include <array>
#include <unordered_map>

#include <boost/bimap.hpp>
#include <boost/assign.hpp>

#include "../u.hpp"

namespace ts::structures {
    struct UBlock {
        struct UPort {
            enum class type {
                IN,
                OUT
            };

            inline static const boost::bimap<type, std::string> types = 
                boost::assign::list_of<boost::bimap<type, std::string>::relation>
                ( type::IN, "in" )
                ( type::OUT, "out" );

            inline static constexpr int defaultNumber = 1;

            UPort() = delete;
            explicit UPort(int number, type pType) noexcept 
                : number{ number },
                  pType{ pType }
            {}

            int number;
            type pType;
        };

        virtual ~UBlock() = default;

        U::block type;
        std::string name;

    protected:
        UBlock() = delete;
        UBlock(U::block type, std::string name) noexcept
            : type{ type },
              name{ std::move(name) }
        {}
    };

    struct UInportBlock final : public UBlock {
        inline static constexpr int numberOfPorts = 1;

        UInportBlock() = delete;
        UInportBlock(U::block type, std::string name, std::array<UPort, numberOfPorts> ports) noexcept
            : UBlock{ type, std::move(name) },
              ports{ ports }
        {}

        ~UInportBlock() override = default;

        std::array<UPort, numberOfPorts> ports;
    };

    struct USumBlock final : public UBlock {
        using inputPortNumber_t = int;

        enum class sign {
            PLUS,
            MINUS
        };

        inline static const boost::bimap<sign, char> signs = 
            boost::assign::list_of<boost::bimap<sign, char>::relation>
            ( sign::PLUS, '+' )
            ( sign::MINUS, '-' );

        inline static constexpr int numberOfPorts = 3;

        USumBlock() = delete;
        USumBlock(U::block type, std::string name, std::array<UPort, numberOfPorts> ports, std::unordered_map<inputPortNumber_t, sign> inputSigns) noexcept
            : UBlock{ type, std::move(name) },
              ports{ ports },
              inputSigns{ std::move(inputSigns) }
        {}

        ~USumBlock() override = default;

        std::array<UPort, numberOfPorts> ports;
        std::unordered_map<inputPortNumber_t, sign> inputSigns;
    };

    struct UGainBlock final : public UBlock {
        inline static constexpr int numberOfPorts = 2;

        UGainBlock() = delete;
        UGainBlock(U::block type, std::string name, std::array<UPort, numberOfPorts> ports, double gain) noexcept
            : UBlock{ type, std::move(name) },
              ports{ ports },
              gain{ gain }
        {}

        ~UGainBlock() override = default;

        std::array<UPort, numberOfPorts> ports;
        double gain;
    };

    struct UUnitDelayBlock final : public UBlock {
        inline static constexpr int numberOfPorts = 2;

        UUnitDelayBlock() = delete;
        UUnitDelayBlock(U::block type, std::string name, std::array<UPort, numberOfPorts> ports, int sampleTime) noexcept
            : UBlock{ type, std::move(name) },
              ports{ ports },
              sampleTime{ sampleTime }
        {}

        ~UUnitDelayBlock() override = default;

        std::array<UPort, numberOfPorts> ports;
        int sampleTime;
    };

    struct UOutportBlock final : public UBlock {
        inline static constexpr int numberOfPorts = 1;

        UOutportBlock() = delete;
        UOutportBlock(U::block type, std::string name, std::array<UPort, numberOfPorts> ports) noexcept
            : UBlock{ type, std::move(name) },
              ports{ ports }
        {}

        ~UOutportBlock() override = default;

        std::array<UPort, numberOfPorts> ports;
    };
}