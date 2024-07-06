#include "uformat_builder.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>

#include "core/universal/u.hpp"
#include "core/source/sblock.hpp"
#include "core/source/slink.hpp"
#include "utility/messages.hpp"

#define ERR_WITH_LINE(err) \
    (std::format("{}: {}:{}", err, __FUNCTION__, __LINE__))

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
            throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::NO_INPUT_VARS) };

        if (outVars.empty())
            throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::NO_OUTPUT_VARS) };

        UCodeOperatorBuilder uCodeOperatorBuilder;
        std::unordered_map<int, std::string> varNames; // TODO: unused
        for (const auto& inVar : inVars) {
            const SElements::blockId_t srcBlockId = inVar.first;
            const auto& srcBlock = inVar.second;
            
            if (!splittedLinks.contains(srcBlockId))
                throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::NO_INPUT_LINK) };

            for (const auto& links = splittedLinks.at(srcBlockId); const auto& link : links) {
                assert(srcBlockId == link.first.blockId);
                
                const SLink::SPoint& srcPoint = link.first;
                const SLink::SPoint& destPoint = link.second;
                const SElements::blockId_t destBlockId = destPoint.blockId;
                
                if (!operators.contains(destBlockId))
                    throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::INVALID_LINK) };

                const auto& destBlock = operators.at(destBlockId);
                auto uOperator = uCodeOperatorBuilder.makeOperator(
                    UCodeOperatorBuilder::BlockData{ srcBlockId, srcBlock }, 
                    UCodeOperatorBuilder::BlockData{ destBlockId, destBlock },
                    srcPoint,
                    destPoint
                );

                if (!uOperator)
                    continue;
                // TODO: here
            }
        }

        // process other operators
        // check if extra
    }

    bool UFormatBuilder::UCodeOperatorBuilder::extraBuildDataExists() const noexcept {
        return !_sumOperatorBuildData.empty() || !_multOperatorBuildData.empty();
    }

    std::unique_ptr<U::Operator> UFormatBuilder::UCodeOperatorBuilder::makeOperator(
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
                throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::INVALID_LINK) };
            } case ts::structures::blockType::SUM: {
                assert(dynamic_cast<SSumBlock*>(&*destBlock.block));
                if (_sumOperatorBuildData.contains(destBlock.blockId)) {
                    SSumBlock* destSumBlock = static_cast<SSumBlock*>(&*destBlock.block);
                    const U2ArgsOperatorBuildData& sumBuildData = _sumOperatorBuildData.at(destBlock.blockId);
                    const U2ArgsOperatorBuildData::PointsLink sumArg1LinkData = sumBuildData.getLink();
                    assert(sumArg1LinkData.srcBlock && sumArg1LinkData.srcPoint && sumArg1LinkData.destPoint);

                    assert(destSumBlock);
                    const auto& destSumPorts = destSumBlock->ports;
                    const auto& destSumInputSigns = destSumBlock->inputSigns;
                    const auto& destSumPort1 = destSumPorts[0];
                    const auto& destSumPort2 = destSumPorts[1];
                    assert(destSumInputSigns.contains(destSumPort1.number));
                    assert(destSumInputSigns.contains(destSumPort2.number));
                    assert(destSumPort1.pType == SBlock::SPort::type::IN);
                    assert(destSumPort2.pType == SBlock::SPort::type::IN);
                    assert(destSumPort1.number != destSumPort2.number);

                    std::string arg1Name;
                    std::string arg2Name;
                    U::Var::sign arg1Sign;
                    U::Var::sign arg2Sign;

                    const auto& destArg1Port = sumArg1LinkData.destPoint->port;
                    const auto& destArg2Port = destPoint.port;
                    if (sumArg1LinkData.srcPoint.value().blockId == srcBlock.blockId)
                        throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::INVALID_LINK) };
                    if (destArg1Port.number == destArg2Port.number)
                        throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::INVALID_LINK) };
                    if (destArg1Port.pType != SBlock::SPort::type::IN || destArg2Port.pType != SBlock::SPort::type::IN)
                        throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::INVALID_LINK) };
                    
                    arg1Name = sumArg1LinkData.srcBlock.value()->name;
                    arg2Name = srcBlock.block->name;
                    arg1Sign = destSumInputSigns.at(destSumPort1.number);
                    arg2Sign = destSumInputSigns.at(destSumPort2.number);
                    if (destArg1Port.number == destSumPort1.number && destArg2Port.number == destSumPort2.number) {
                        // nothing
                    } else if (destArg1Port.number == destSumPort2.number && destArg2Port.number == destSumPort1.number) {
                        std::swap(arg1Name, arg2Name);
                        std::swap(arg1Sign, arg2Sign);
                    } else {
                        throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::INVALID_LINK) };
                    }
                    
                    const U::Var sumRes = U::Var{ U::Var::type::DOUBLE, destSumBlock->name, std::nullopt, std::nullopt };
                    const U::Var sumArg1 = U::Var{ U::Var::type::DOUBLE, arg1Name, std::nullopt, arg1Sign };
                    const U::Var sumArg2 = U::Var{ U::Var::type::DOUBLE, arg2Name, std::nullopt, arg2Sign };

                    uOperator = std::make_unique<U::Sum>(sumRes, sumArg1, sumArg2);
                    _sumOperatorBuildData.erase(destBlock.blockId);
                    
                    break;
                }

                _sumOperatorBuildData.insert({
                    destBlock.blockId,
                    U2ArgsOperatorBuildData{
                        U2ArgsOperatorBuildData::PointsLink{
                            &*srcBlock.block,
                            srcPoint,
                            destPoint
                        },
                        &*destBlock.block
                    }
                });

                break;
            }
        }

        return uOperator;
    }
}
