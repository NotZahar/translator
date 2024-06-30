#include "uformat_builder.hpp"

#include "u.hpp"

namespace {
    using namespace ts::structures;

    void categorizeBlocks(
            std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& sBlocks, 
            std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& vars, 
            std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& operators) {
        for (auto& sBlock : sBlocks) {
            switch (sBlock.second->type) {
                case blockType::INPORT:
                    [[fallthrough]];
                case blockType::OUTPORT:
                    vars.insert({ sBlock.first, std::move(sBlock.second) });
                    break;
                case blockType::SUM:
                    [[fallthrough]];
                case blockType::GAIN:
                    [[fallthrough]];
                case blockType::UNIT_DELAY:
                    operators.insert({ sBlock.first, std::move(sBlock.second) });
                    break;
            }
        }
    }

    void makeOrder(
            const std::vector<SLink>& sLinks, 
            const std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& vars, 
            const std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& operators) {
        // through layers
    }
}

namespace ts {
    U::UFormat UFormatBuilder::build(structures::SElements& sElements) {
        U::UFormat format;

        std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>> vars;
        std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>> operators; 
        categorizeBlocks(sElements.blocks, vars, operators);
        makeOrder(sElements.links, vars, operators);

        return format;
    }
}