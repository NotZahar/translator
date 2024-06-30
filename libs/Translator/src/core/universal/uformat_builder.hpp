#pragma once

#include "../source/selements.hpp"
#include "uformat.hpp"

namespace ts {
    class UFormatBuilder {
    public:
        constexpr UFormatBuilder() noexcept = default;

        ~UFormatBuilder() = default;

        [[nodiscard]] static U::UFormat build(structures::SElements& sElements);
    };
}