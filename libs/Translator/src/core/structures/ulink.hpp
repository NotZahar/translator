#pragma once

#include <vector>

#include "ublock.hpp"

namespace ts::structures {
    struct ULink {
        struct UPoint {
            int blockId;
            UBlock::UPort port;
        };

        UPoint source;
        std::vector<UPoint> destinations;        
    };
}