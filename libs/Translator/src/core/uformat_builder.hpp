#pragma once

#include "structures/uelements.hpp"
#include "structures/uformat.hpp"

namespace ts {
    class UFormatBuilder {
    public:
        constexpr UFormatBuilder() noexcept = default;

        ~UFormatBuilder() = default;

        static structures::UFormat build(structures::UElements uElements);
    };
}