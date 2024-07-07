#pragma once

#include <vector>
#include <memory>

#include "u.hpp"

namespace ts::U {
    struct UFormat {
        UFormat() = delete;
        UFormat(std::vector<U::Var> uVars, std::vector<U::Var> uInitializedVars, std::vector<U::Var> uExportVars, std::vector<std::shared_ptr<U::Operator>> uCode) noexcept 
            : uVars{ std::move(uVars) },
              uInitializedVars{ std::move(uInitializedVars) },
              uExportVars{ std::move(uExportVars) },
              uCode{ std::move(uCode) }
        {}
        
        ~UFormat() = default;

        std::vector<U::Var> uVars;
        std::vector<U::Var> uInitializedVars;
        std::vector<U::Var> uExportVars;
        std::vector<std::shared_ptr<U::Operator>> uCode;
    };
}