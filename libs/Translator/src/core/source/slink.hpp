#pragma once

#include <vector>

#include "sblock.hpp"

namespace ts::structures {
    struct SLink {
        struct SPoint {
            int blockId;
            SBlock::SPort port;
        };

        SPoint source;
        std::vector<SPoint> destinations;        
    };
}