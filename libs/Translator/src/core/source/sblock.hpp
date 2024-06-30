#pragma once

#include <array>
#include <unordered_map>

#include <boost/bimap.hpp>
#include <boost/assign.hpp>

namespace ts::structures {
    enum class blockType {
        INPORT,
        SUM,
        GAIN,
        UNIT_DELAY,
        OUTPORT
    };

    inline static const boost::bimap<blockType, std::string> blockTypes = 
        boost::assign::list_of<boost::bimap<blockType, std::string>::relation>
        ( blockType::INPORT, "Inport" )
        ( blockType::SUM, "Sum" )
        ( blockType::GAIN, "Gain" )
        ( blockType::UNIT_DELAY, "UnitDelay" )
        ( blockType::OUTPORT, "Outport" );

    struct SBlock {
        struct SPort {
            enum class type {
                IN,
                OUT
            };

            inline static const boost::bimap<type, std::string> types = 
                boost::assign::list_of<boost::bimap<type, std::string>::relation>
                ( type::IN, "in" )
                ( type::OUT, "out" );

            inline static constexpr int defaultNumber = 1;

            SPort() = delete;
            explicit SPort(int number, type pType) noexcept 
                : number{ number },
                  pType{ pType }
            {}

            int number;
            type pType;
        };

        virtual ~SBlock() = default;

        blockType type;
        std::string name;

    protected:
        SBlock() = delete;
        SBlock(blockType type, std::string name) noexcept
            : type{ type },
              name{ std::move(name) }
        {}
    };

    struct SInportBlock final : public SBlock {
        inline static constexpr int numberOfPorts = 1;

        SInportBlock() = delete;
        SInportBlock(blockType type, std::string name, std::array<SPort, numberOfPorts> ports) noexcept
            : SBlock{ type, std::move(name) },
              ports{ ports }
        {}

        ~SInportBlock() override = default;

        std::array<SPort, numberOfPorts> ports;
    };

    struct SSumBlock final : public SBlock {
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

        SSumBlock() = delete;
        SSumBlock(blockType type, std::string name, std::array<SPort, numberOfPorts> ports, std::unordered_map<inputPortNumber_t, sign> inputSigns) noexcept
            : SBlock{ type, std::move(name) },
              ports{ ports },
              inputSigns{ std::move(inputSigns) }
        {}

        ~SSumBlock() override = default;

        std::array<SPort, numberOfPorts> ports;
        std::unordered_map<inputPortNumber_t, sign> inputSigns;
    };

    struct SGainBlock final : public SBlock {
        inline static constexpr int numberOfPorts = 2;

        SGainBlock() = delete;
        SGainBlock(blockType type, std::string name, std::array<SPort, numberOfPorts> ports, double gain) noexcept
            : SBlock{ type, std::move(name) },
              ports{ ports },
              gain{ gain }
        {}

        ~SGainBlock() override = default;

        std::array<SPort, numberOfPorts> ports;
        double gain;
    };

    struct SUnitDelayBlock final : public SBlock {
        inline static constexpr int numberOfPorts = 2;

        SUnitDelayBlock() = delete;
        SUnitDelayBlock(blockType type, std::string name, std::array<SPort, numberOfPorts> ports, int sampleTime) noexcept
            : SBlock{ type, std::move(name) },
              ports{ ports },
              sampleTime{ sampleTime }
        {}

        ~SUnitDelayBlock() override = default;

        std::array<SPort, numberOfPorts> ports;
        int sampleTime;
    };

    struct SOutportBlock final : public SBlock {
        inline static constexpr int numberOfPorts = 1;

        SOutportBlock() = delete;
        SOutportBlock(blockType type, std::string name, std::array<SPort, numberOfPorts> ports) noexcept
            : SBlock{ type, std::move(name) },
              ports{ ports }
        {}

        ~SOutportBlock() override = default;

        std::array<SPort, numberOfPorts> ports;
    };
}