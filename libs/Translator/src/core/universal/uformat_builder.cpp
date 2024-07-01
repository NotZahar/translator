#include "uformat_builder.hpp"

#include <cassert>
#include <stdexcept>
#include <unordered_map>

#include "logger.hpp"
#include "u.hpp"
#include "core/source/sblock.hpp"
#include "core/source/slink.hpp"
#include "utility/messages.hpp"

namespace {
    using namespace ts::structures;
    using splittedLink_t = std::unordered_multimap<SLink::SPoint, SLink::SPoint, SPointHash, SPointEqual>;
    using splittedLinks_t = std::unordered_map<SElements::blockId_t, splittedLink_t>;

    void categorizeBlocks(
            std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& sBlocks, 
            std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& inVars,
            std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& outVars, 
            std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& operators) noexcept {
        for (auto& block : sBlocks) {
            switch (block.second->type) {
                case blockType::INPORT:
                    inVars.insert({ block.first, std::move(block.second) });
                    break;
                case blockType::OUTPORT:
                    outVars.insert({ block.first, std::move(block.second) });
                    break;
                case blockType::SUM:
                    [[fallthrough]];
                case blockType::GAIN:
                    [[fallthrough]];
                case blockType::UNIT_DELAY:
                    operators.insert({ block.first, std::move(block.second) });
                    break;
            }
        }
    }

    [[nodiscard]] splittedLinks_t splitLinks(const std::vector<SLink>& sLinks) {
        splittedLinks_t splittedLinks;

        for (const SLink& link : sLinks) {
            const SLink::SPoint& srcPoint = link.source;
            const SElements::blockId_t srcBlockId = srcPoint.blockId;

            splittedLink_t splittedLink;
            for (const SLink::SPoint& dstPoint : link.destinations)
                splittedLink.insert({ srcPoint, dstPoint });

            splittedLinks.insert({ srcBlockId, std::move(splittedLink) });
        }

        return splittedLinks;
    }

    void makeCode (
            splittedLinks_t splittedLinks,
            const std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& inVars,
            const std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& outVars, 
            const std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& operators) {
        if (inVars.empty())
            throw std::runtime_error{ ts::messages::errors::NO_INPUT_VARS };

        if (outVars.empty())
            throw std::runtime_error{ ts::messages::errors::NO_OUTPUT_VARS };

        for (const auto& link : splittedLinks) {
            if (link.second.empty())
                throw std::runtime_error{ ts::messages::errors::INVALID_LINK };

            // ...
        }
    }
}

namespace ts {
    U::UFormat UFormatBuilder::build(structures::SElements& sElements) {
        U::UFormat format;

        std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>> inVars;
        std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>> outVars;
        std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>> operators;
        
        categorizeBlocks(sElements.blocks, inVars, outVars, operators);
        makeCode(splitLinks(sElements.links), inVars, outVars, operators);

        return format;
    }
}