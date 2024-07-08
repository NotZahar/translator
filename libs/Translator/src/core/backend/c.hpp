#pragma once

#include <format>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/erase.hpp>

#include "../universal/uformat.hpp"
#include "core/universal/u.hpp"

namespace ts::backend {
    class C {
    public:
        static std::string toC(U::UFormat uFormat) {
            std::string out;

            out += "static struct\n{\n";
            for (auto& var : uFormat.uVars)
                out += makeVarLine(var);
            out += "} run;\n";

            out += "\nvoid initialize()\n{\n";
            // ...
            out += "}\n";

            out += "\nvoid step()\n{\n";
            for (auto& line : uFormat.uCode)
                out += makeOperatorLine(line);
            out += "}\n";

            out += "\nstatic const ExportVar exportVars[] =\n{\n";
            // ...
            out += "};\n";

            return out;
        }
    
    private:
        static std::string makeVarLine(U::Var& var) {
            std::string type;

            switch (var.varType) {
                case U::Var::type::DOUBLE:
                    type = "double";
                    break;
                case U::Var::type::INT:
                    type = "int";
                    break;
            }

            assert(var.name.has_value());
            boost::erase_all(var.name.value(), " ");
            return std::format("    {} {};\n", type, var.name.value());
        }

        static std::string makeOperatorLine(std::shared_ptr<U::Operator>& op) {
            if (auto sum = dynamic_cast<U::Sum*>(op.get()); sum) {
                std::string res = sum->res.name.value();
                boost::erase_all(res, " ");

                std::string arg1Sign = sum->arg1.valueSign.value() == U::Var::sign::PLUS ? "" : "-";

                std::string arg1 = sum->arg1.name.value();
                boost::erase_all(arg1, " ");

                std::string sumSign = sum->arg2.valueSign.value() == U::Var::sign::PLUS ? "+" : "-";

                std::string arg2 = sum->arg2.name.value();
                boost::erase_all(arg2, " ");

                return std::format("    run.{} = {}run.{} {} run.{};\n", res, arg1Sign, arg1, sumSign, arg2);
            }

            if (auto mult = dynamic_cast<U::Mult*>(op.get()); mult) {
                assert(mult->res.name.has_value());
                std::string res = mult->res.name.value();
                boost::erase_all(res, " ");

                assert(mult->arg1.name.has_value());
                std::string arg1 = mult->arg1.name.value();
                boost::erase_all(arg1, " ");

                std::string arg2 = mult->arg2.value.value();

                return std::format("    run.{} = run.{} * {};\n", res, arg1, arg2);
            }

            if (auto assign = dynamic_cast<U::Assign*>(op.get()); assign) {
                assert(assign->to.name.has_value());
                std::string to = assign->to.name.value();
                boost::erase_all(to, " ");

                assert(assign->from.name.has_value());
                std::string from = assign->from.name.value();
                boost::erase_all(from, " ");

                return std::format("    run.{} = run.{};\n", to, from);
            }

            return "";
        }
    };
}