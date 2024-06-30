#pragma once

#include "source/selements.hpp"
#include "u/uformat.hpp"

namespace ts {
    class UFormatBuilder {
    public:
        constexpr UFormatBuilder() noexcept = default;

        ~UFormatBuilder() = default;

        static U::UFormat build(const structures::SElements& sElements);
    };
}