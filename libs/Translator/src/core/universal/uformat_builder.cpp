#include "uformat_builder.hpp"

#include <cassert>
#include <optional>
#include <stdexcept>
#include <unordered_map>

#include "core/universal/u.hpp"
#include "core/source/sblock.hpp"
#include "core/source/slink.hpp"
#include "utility/messages.hpp"

namespace {
    using namespace ts::structures;

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

    [[nodiscard]] ts::UFormatBuilder::splittedLinks_t splitLinks(const std::vector<SLink>& sLinks) {
        ts::UFormatBuilder::splittedLinks_t splittedLinks;

        for (const SLink& link : sLinks) {
            const SLink::SPoint& srcPoint = link.source;
            const SElements::blockId_t srcBlockId = srcPoint.blockId;

            ts::UFormatBuilder::splittedLink_t splittedLink;
            for (const SLink::SPoint& dstPoint : link.destinations)
                splittedLink.insert({ srcPoint, dstPoint });

            splittedLinks.insert({ srcBlockId, std::move(splittedLink) });
        }

        return splittedLinks;
    }
}

namespace ts {
    U::UFormat UFormatBuilder::build(structures::SElements& sElements) {
        U::UFormat format;

        std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>> inVars;
        std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>> outVars;
        std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>> operators;
        
        categorizeBlocks(sElements.blocks, inVars, outVars, operators);
        makeUCode(splitLinks(sElements.links), inVars, outVars, operators);

        return format;
    }

    void UFormatBuilder::makeUCode(
            splittedLinks_t splittedLinks,
            const std::unordered_map<structures::SElements::blockId_t, std::unique_ptr<structures::SBlock>>& inVars,
            const std::unordered_map<structures::SElements::blockId_t, std::unique_ptr<structures::SBlock>>& outVars,
            const std::unordered_map<structures::SElements::blockId_t, std::unique_ptr<structures::SBlock>>& operators) {
        if (inVars.empty())
            throw std::runtime_error{ ts::messages::errors::NO_INPUT_VARS };

        if (outVars.empty())
            throw std::runtime_error{ ts::messages::errors::NO_OUTPUT_VARS };

        UCodeLineBuilder uCodeLineBuilder;
        std::unordered_map<int, std::string> varNames; // TODO: unused
        for (const auto& inVar : inVars) {
            const SElements::blockId_t srcBlockId = inVar.first;
            const auto& srcBlock = inVar.second;
            
            if (!splittedLinks.contains(srcBlockId))
                throw std::runtime_error{ ts::messages::errors::NO_INPUT_LINK };

            for (const auto& links = splittedLinks.at(srcBlockId); const auto& link : links) {
                assert(srcBlockId == link.first.blockId);
                
                const SLink::SPoint& srcPoint = link.first;
                const SLink::SPoint& destPoint = link.second;
                const SElements::blockId_t destBlockId = destPoint.blockId;
                
                if (!operators.contains(destBlockId))
                    throw std::runtime_error{ ts::messages::errors::INVALID_LINK };

                const auto& destBlock = operators.at(destBlockId);
                auto codeLine = uCodeLineBuilder.makeOperatorLine(
                    UCodeLineBuilder::BlockData{ srcBlockId, srcBlock }, 
                    UCodeLineBuilder::BlockData{ destBlockId, destBlock },
                    srcPoint,
                    destPoint
                );

                // TODO: may be nullptr

                // TODO: [here]
            }
        }

        // process other operators
    }

    bool UFormatBuilder::UCodeLineBuilder::extraBuildDataExists() const noexcept {
        return !_sumOperatorBuildData.empty() || !_multOperatorBuildData.empty();
    }

    std::unique_ptr<U::Operator> UFormatBuilder::UCodeLineBuilder::makeOperatorLine(
            BlockData srcBlock,
            BlockData destBlock,
            const structures::SLink::SPoint& srcPoint,
            const structures::SLink::SPoint& destPoint) {
        std::unique_ptr<U::Operator> uOperator;

        assert(srcBlock.block);
        assert(destBlock.block);
        assert(srcBlock.blockId == srcBlock.blockId);
        assert(destBlock.blockId == destPoint.blockId);

        switch (destBlock.block->type) {
            case ts::structures::blockType::INPORT: {                
                throw std::runtime_error{ messages::errors::INVALID_LINK };
            } case ts::structures::blockType::SUM: {
                assert(dynamic_cast<SSumBlock*>(&*destBlock.block));
                if (_sumOperatorBuildData.contains(destBlock.blockId)) {
                    const UOperator2BuildData& sumBuildData = _sumOperatorBuildData.at(destBlock.blockId);
                    assert(sumBuildData.oneDestPointAlreadyOccupied());
                    UOperator2BuildData::PointsLink occupiedPointsLink = sumBuildData.getOccupiedPointsLink();
                    UOperator2BuildData::PointsLink freePointsLink = sumBuildData.getFreePointsLink();
                    // TODO: [here]
                    // match signs && numbers
                    // create u operator
                } else {
                    _sumOperatorBuildData.insert({
                        destBlock.blockId,
                        UOperator2BuildData{
                            UOperator2BuildData::PointsLink{
                                &*srcBlock.block,
                                srcPoint,
                                destPoint  
                            },
                            UOperator2BuildData::PointsLink{
                                std::nullopt,
                                std::nullopt,
                                std::nullopt    
                            },
                            &*destBlock.block
                        }
                    });
                }

                break;
            }
        }

        return uOperator;
    }
}
