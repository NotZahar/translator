#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "sblock.hpp"
#include "slink.hpp"

namespace ts::structures {
    struct SElements {
        using blockId_t = int;

        SElements() = delete;
        SElements(std::unordered_map<blockId_t, std::unique_ptr<SBlock>> blocks, std::vector<SLink> links) noexcept 
            : blocks{ std::move(blocks) }, 
              links{ std::move(links) }
        {}
        
        ~SElements() = default;

        std::unordered_map<blockId_t, std::unique_ptr<SBlock>> blocks;
        std::vector<SLink> links;
    };
}