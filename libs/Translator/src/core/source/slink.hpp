#pragma once

#include <type_traits>
#include <vector>

#include <boost/container_hash/hash.hpp>

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

    struct SPointHash {
        std::size_t operator()(const SLink::SPoint& point) const noexcept {
            std::size_t seed = 0;
            
            boost::hash_combine(seed, point.blockId);
            boost::hash_combine(seed, point.port.number);
            boost::hash_combine(seed, static_cast<std::underlying_type_t<SBlock::SPort::type>>(point.port.pType));

            return seed;
        }
    };
    
    struct SPointEqual {
        bool operator()(const SLink::SPoint& lhs, const SLink::SPoint& rhs) const noexcept {
            return lhs.blockId == rhs.blockId 
                && lhs.port.number == rhs.port.number
                && lhs.port.pType == rhs.port.pType;
        }
    };
}