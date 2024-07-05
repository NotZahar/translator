/*! \brief U description
 *  
 * Possible structures:
 * sum(resVar, arg1Var, arg2Var)
 * mult(resVar, arg1Var, arg2Var)
 * assign(toVar, fromVar) 
 */

#pragma once

#include <cassert>
#include <string>
#include <optional>

namespace ts::U {
    struct Var {
        enum class type {
            DOUBLE,
            INT
        };

        enum class sign {
            PLUS,
            MINUS
        };
        
        enum class linkage {
            INTERNAL,
            EXTERNAL
        };

        Var() = delete;
        Var(type varType, linkage linkageType, std::optional<std::string> name, std::optional<std::string> value, std::optional<sign> valueSign) noexcept 
            : varType{ varType },
              linkageType{ linkageType },
              name{ name },
              value{ value },
              valueSign{ valueSign }
        {
            assert(!name || !value);
        }

        type varType;
        linkage linkageType;
        std::optional<std::string> name;
        std::optional<std::string> value;
        std::optional<sign> valueSign;
    };

    struct Operator {
        constexpr Operator() noexcept = default;

        virtual ~Operator() = default;
    };

    struct Sum final : public Operator {
        Sum() = delete;
        explicit Sum(const Var& res, const Var& arg1, const Var& arg2) noexcept 
            : res{ res },
              arg1{ arg1 },
              arg2{ arg2 }
        {}

        ~Sum() override = default;

        Var res;
        Var arg1;
        Var arg2;
    };

    struct Mult final : public Operator {
        Mult() = delete;
        explicit Mult(const Var& res, const Var& arg1, const Var& arg2) noexcept 
            : res{ res },
              arg1{ arg1 },
              arg2{ arg2 }
        {}

        ~Mult() override = default;

        Var res;
        Var arg1;
        Var arg2;
    };

    struct Assign final : public Operator {
        Assign() = delete;
        explicit Assign(const Var& to, const Var& from) noexcept 
            : to{ to },
              from{ from }
        {}

        ~Assign() override = default;

        Var to;
        Var from;
    };
}