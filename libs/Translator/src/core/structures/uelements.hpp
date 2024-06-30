#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "ublock.hpp"
#include "ulink.hpp"

namespace ts::structures {
    class UElements {
    public:
        using blockId_t = int;

        UElements() = delete;
        UElements(std::unordered_map<blockId_t, std::unique_ptr<UBlock>> blocks, std::vector<ULink> links) noexcept 
            : blocks{ std::move(blocks) }, 
              links{ std::move(links) }
        {}
        
        ~UElements() = default;

        std::unordered_map<blockId_t, std::unique_ptr<UBlock>> blocks;
        std::vector<ULink> links;
    };
}