#include "uformat_builder.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
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
        auto uCode = makeUCode(splitLinks(sElements.links), inVars, outVars, operators);
        // TODO: [here]

        return format;
    }

    std::vector<std::unique_ptr<U::Operator>> UFormatBuilder::makeUCode(
            splittedLinks_t splittedLinks,
            const std::unordered_map<structures::SElements::blockId_t, std::unique_ptr<structures::SBlock>>& inVars,
            const std::unordered_map<structures::SElements::blockId_t, std::unique_ptr<structures::SBlock>>& outVars,
            const std::unordered_map<structures::SElements::blockId_t, std::unique_ptr<structures::SBlock>>& operators) {
        std::vector<std::unique_ptr<U::Operator>> uCode;
        std::vector<std::unique_ptr<U::Operator>> uCodeHigh;
        std::vector<std::unique_ptr<U::Operator>> uCodeNormal;

        if (inVars.empty())
            throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::NO_INPUT_VARS) };

        if (outVars.empty())
            throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::NO_OUTPUT_VARS) };

        UCodeOperatorBuilder uCodeOperatorBuilder;
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
                MakeOperatorResult makeOperatorResult = uCodeOperatorBuilder.makeOperator(
                    UCodeOperatorBuilder::BlockData{ srcBlockId, srcBlock }, 
                    UCodeOperatorBuilder::BlockData{ destBlockId, destBlock },
                    srcPoint,
                    destPoint
                );

                if (!makeOperatorResult.uOperator)
                    continue;

                switch (makeOperatorResult.uPriority) {
                    case uOperatorPriority::high:
                        uCodeHigh.emplace_back(std::move(makeOperatorResult.uOperator));
                        break;
                    case uOperatorPriority::normal:
                        uCodeNormal.emplace_back(std::move(makeOperatorResult.uOperator));
                        break;
                }
            }
        }

        // TODO: process other operators
        

        if (uCodeOperatorBuilder.extraBuildDataExists())
            throw std::runtime_error{ ts::messages::errors::EXTRA_LINKS };

        uCode = std::move(uCodeHigh);
        for (auto& normalOperator : uCodeNormal)
            uCode.emplace_back(std::move(normalOperator));

        assert(uCode.size() >= uCodeNormal.size());
        return uCode;
    }

    bool UFormatBuilder::UCodeOperatorBuilder::extraBuildDataExists() const noexcept {
        return !_sumOperatorBuildData.empty();
    }

    UFormatBuilder::MakeOperatorResult UFormatBuilder::UCodeOperatorBuilder::makeOperator(
            BlockData srcBlock,
            BlockData destBlock,
            const structures::SLink::SPoint& srcPoint,
            const structures::SLink::SPoint& destPoint) {
        UFormatBuilder::MakeOperatorResult result;
        result.uPriority = uOperatorPriority::high;

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
                    assert(destSumBlock);
                    assert(sumArg1LinkData.srcBlock && sumArg1LinkData.srcPoint && sumArg1LinkData.destPoint);

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

                    result.uOperator = std::make_unique<U::Sum>(sumRes, sumArg1, sumArg2);
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
            } case ts::structures::blockType::GAIN: {
                assert(dynamic_cast<SGainBlock*>(&*destBlock.block));
                
                SGainBlock* destGainBlock = static_cast<SGainBlock*>(&*destBlock.block);
                assert(destGainBlock);

                assert(destGainBlock->ports[0].pType == SBlock::SPort::type::IN);
                
                const U::Var multRes = U::Var{ U::Var::type::DOUBLE, destGainBlock->name, std::nullopt, std::nullopt };
                const U::Var multArg1 = U::Var{ U::Var::type::DOUBLE, srcBlock.block->name, std::nullopt, std::nullopt };
                const U::Var multArg2 = U::Var{ U::Var::type::DOUBLE, std::nullopt, std::to_string(destGainBlock->gain), U::Var::sign::PLUS };

                result.uOperator = std::make_unique<U::Mult>(multRes, multArg1, multArg2);
                
                break;
            } case ts::structures::blockType::UNIT_DELAY: {
                assert(dynamic_cast<SUnitDelayBlock*>(&*destBlock.block));
                
                SUnitDelayBlock* destUnitDelayBlock = static_cast<SUnitDelayBlock*>(&*destBlock.block);
                assert(destUnitDelayBlock);

                assert(destUnitDelayBlock->ports[0].pType == SBlock::SPort::type::IN);
                
                const U::Var to = U::Var{ U::Var::type::DOUBLE, destUnitDelayBlock->name, std::nullopt, std::nullopt };
                const U::Var from = U::Var{ U::Var::type::DOUBLE, srcBlock.block->name, std::nullopt, std::nullopt };

                result.uOperator = std::make_unique<U::Assign>(to, from);
                result.uPriority = uOperatorPriority::normal;
                
                break;
            } case ts::structures::blockType::OUTPORT: {
                assert(dynamic_cast<SOutportBlock*>(&*destBlock.block));
                
                SOutportBlock* destOutportBlock = static_cast<SOutportBlock*>(&*destBlock.block);
                assert(destOutportBlock);

                assert(destOutportBlock->ports[0].pType == SBlock::SPort::type::IN);
                
                const U::Var to = U::Var{ U::Var::type::DOUBLE, destOutportBlock->name, std::nullopt, std::nullopt };
                const U::Var from = U::Var{ U::Var::type::DOUBLE, srcBlock.block->name, std::nullopt, std::nullopt };

                result.uOperator = std::make_unique<U::Assign>(to, from);
            }
        }

        return result;
    }
}
