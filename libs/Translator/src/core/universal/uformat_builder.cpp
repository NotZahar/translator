#include "uformat_builder.hpp"

#include <cassert>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "../universal/u.hpp"
#include "../source/sblock.hpp"
#include "../source/slink.hpp"
#include "../../utility/messages.hpp"
#include "core/universal/uformat.hpp"

#define ERR_WITH_LINE(err) \
    (std::format("{}: {}:{}", err, __FUNCTION__, __LINE__))

namespace {
    using namespace ts::structures;

    void categorizeBlocks(
            std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>>& sBlocks, 
            std::unordered_map<SElements::blockId_t, std::shared_ptr<SBlock>>& inVars,
            std::unordered_map<SElements::blockId_t, std::shared_ptr<SBlock>>& operators) noexcept {
        for (auto& block : sBlocks) {
            switch (block.second->type) {
                case blockType::INPORT:
                    inVars.insert({ block.first, block.second->clone() });
                    break;
                case blockType::OUTPORT:
                    [[fallthrough]];
                case blockType::SUM:
                    [[fallthrough]];
                case blockType::GAIN:
                    operators.insert({ block.first, block.second->clone() });
                    break;
                case blockType::UNIT_DELAY:
                    inVars.insert({ block.first, block.second->clone() });
                    operators.insert({ block.first, block.second->clone() });
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
        std::unordered_map<SElements::blockId_t, std::shared_ptr<SBlock>> inVars;
        std::unordered_map<SElements::blockId_t, std::shared_ptr<SBlock>> operators;
        
        categorizeBlocks(sElements.blocks, inVars, operators);
        return makeU(splitLinks(sElements.links), inVars, operators);
    }

    [[nodiscard]] UFormatBuilder::ProcessLayerResult UFormatBuilder::processLinks(
            const splittedLinks_t& splittedLinks,
            const std::unordered_map<structures::SElements::blockId_t, std::shared_ptr<structures::SBlock>>& srcBlocks,
            const std::unordered_map<structures::SElements::blockId_t, std::shared_ptr<structures::SBlock>>& destBlocks,
            UCodeOperatorBuilder& uCodeOperatorBuilder,
            std::vector<std::shared_ptr<U::Operator>>& uCodeHigh,
            std::vector<std::shared_ptr<U::Operator>>& uCodeNormal,
            std::vector<std::shared_ptr<U::Operator>>& uCodeLow,
            std::unordered_map<std::string, U::Var>& uVars,
            std::unordered_map<std::string, U::Var>& uInitializedVars,
            std::unordered_map<std::string, U::Var>& uExportVars,
            bool forwardDirection) const {
        ProcessLayerResult result;
        for (const auto& srcBlock : srcBlocks) {
            const SElements::blockId_t srcBlockId = srcBlock.first;
            const std::shared_ptr<structures::SBlock> srcBlockData = srcBlock.second;
            
            if (!splittedLinks.contains(srcBlockId)) {
                if (srcBlockData->type == blockType::OUTPORT)
                    continue;
                throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::NOT_EXISTING_LINK) };
            }

            for (const splittedLink_t& links = splittedLinks.at(srcBlockId); const auto& link : links) {
                assert(srcBlockId == link.first.blockId);
                
                const SLink::SPoint& srcPoint = link.first;
                const SLink::SPoint& destPoint = link.second;
                const SElements::blockId_t destBlockId = destPoint.blockId;
                
                if (!destBlocks.contains(destBlockId))
                    throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::INVALID_LINK) };

                const std::shared_ptr<structures::SBlock> destBlockData = destBlocks.at(destBlockId);
                if (!forwardDirection && destBlockData->type != blockType::UNIT_DELAY)
                    continue;
            
                if (forwardDirection && destBlockData->type == blockType::UNIT_DELAY) {
                    result.blocksToProcessReverse.insert({ srcBlockId, srcBlockData->clone() });
                    continue;
                }

                result.blocksToProcessNext.insert({ destBlockId, destBlockData->clone() });

                MakeOperatorResult makeOperatorResult = uCodeOperatorBuilder.makeOperator(
                    BlockData{ srcBlockId, srcBlockData }, 
                    BlockData{ destBlockId, destBlockData },
                    srcPoint,
                    destPoint
                );

                for (const auto& var : makeOperatorResult.uVars)
                    uVars.insert({ var.first, var.second });
                for (const auto& var : makeOperatorResult.uInitializedVars)
                    uInitializedVars.insert({ var.first, var.second });
                for (const auto& var : makeOperatorResult.uExportVars)
                    uExportVars.insert({ var.first, var.second });

                if (makeOperatorResult.uOperators.empty())
                    continue;

                for (const auto& operators : makeOperatorResult.uOperators) {
                    switch (operators.uPriority) {
                        case uOperatorPriority::high:
                            uCodeHigh.emplace_back(operators.uOperator);
                            break;
                        case uOperatorPriority::normal:
                            uCodeNormal.emplace_back(operators.uOperator);
                            break;
                        case uOperatorPriority::low:
                            uCodeLow.emplace_back(operators.uOperator);
                            break;
                    }
                }
            }
        }

        return result;
    }

    U::UFormat UFormatBuilder::makeU(
            splittedLinks_t splittedLinks,
            const std::unordered_map<structures::SElements::blockId_t, std::shared_ptr<structures::SBlock>>& inVars,
            const std::unordered_map<structures::SElements::blockId_t, std::shared_ptr<structures::SBlock>>& operators) const {
        std::unordered_map<std::string, U::Var> uVars;
        std::unordered_map<std::string, U::Var> uInitializedVars;
        std::unordered_map<std::string, U::Var> uExportVars;
        std::vector<std::shared_ptr<U::Operator>> uCode;
        std::vector<std::shared_ptr<U::Operator>> uCodeHigh;
        std::vector<std::shared_ptr<U::Operator>> uCodeNormal;
        std::vector<std::shared_ptr<U::Operator>> uCodeLow;
        UCodeOperatorBuilder uCodeOperatorBuilder;
        std::unordered_map<structures::SElements::blockId_t, std::shared_ptr<structures::SBlock>> blocksToProcessReverse;

        if (inVars.empty())
            throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::NO_INPUT_VARS) };

        UFormatBuilder::ProcessLayerResult processResult = processLinks(splittedLinks, inVars, operators, uCodeOperatorBuilder, uCodeHigh, uCodeNormal, uCodeLow, uVars, uInitializedVars, uExportVars);
        blocksToProcessReverse = std::move(processResult.blocksToProcessReverse);
        while (!processResult.blocksToProcessNext.empty()) {
            auto processResultInternal = processLinks(splittedLinks, processResult.blocksToProcessNext, operators, uCodeOperatorBuilder, uCodeHigh, uCodeNormal, uCodeLow, uVars, uInitializedVars, uExportVars);
            processResult.blocksToProcessNext = std::move(processResultInternal.blocksToProcessNext);
            for (const auto& futureBlock : processResultInternal.blocksToProcessReverse)
                blocksToProcessReverse.insert(futureBlock);
        }

        UFormatBuilder::ProcessLayerResult processResultFuture = processLinks(splittedLinks, blocksToProcessReverse, operators, uCodeOperatorBuilder, uCodeHigh, uCodeNormal, uCodeLow, uVars, uInitializedVars, uExportVars, false);

        uCode = std::move(uCodeHigh);
        for (auto& normalOperator : uCodeNormal)
            uCode.emplace_back(normalOperator);
        for (auto& lowOperator : uCodeLow)
            uCode.emplace_back(lowOperator);

        std::vector<U::Var> vars;
        std::vector<U::Var> initializedVars;
        std::vector<U::Var> exportVars;
        for (const auto& var : uVars)
            vars.emplace_back(var.second);
        for (const auto& var : uInitializedVars)
            initializedVars.emplace_back(var.second);
        for (const auto& var : uExportVars)
            exportVars.emplace_back(var.second);

        return U::UFormat{
            vars,
            initializedVars,
            exportVars,
            std::move(uCode)
        };
    }

    UFormatBuilder::MakeOperatorResult UFormatBuilder::UCodeOperatorBuilder::makeOperator(
            BlockData srcBlock,
            BlockData destBlock,
            const structures::SLink::SPoint& srcPoint,
            const structures::SLink::SPoint& destPoint) {
        UFormatBuilder::MakeOperatorResult result;

        assert(srcBlock.block);
        assert(destBlock.block);
        assert(srcBlock.blockId == srcBlock.blockId);
        assert(destBlock.blockId == destPoint.blockId);

        switch (destBlock.block->type) {
            case ts::structures::blockType::INPORT: {
                throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::INVALID_LINK) };
            } case ts::structures::blockType::SUM: {
                assert(dynamic_cast<SSumBlock*>(destBlock.block.get()));
                
                if (_sumOperatorBuildData.contains(destBlock.blockId)) {
                    SSumBlock* destSumBlock = static_cast<SSumBlock*>(destBlock.block.get());
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
                    const structures::SElements::blockId_t srcBlockId1 = sumArg1LinkData.srcPoint.value().blockId;
                    const structures::SElements::blockId_t srcBlockId2 = srcBlock.blockId;
                    if (srcBlockId1 == srcBlockId2)
                        throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::INVALID_LINK) };
                    if (destArg1Port.number == destArg2Port.number)
                        throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::INVALID_LINK) };
                    if (destArg1Port.pType != SBlock::SPort::type::IN || destArg2Port.pType != SBlock::SPort::type::IN)
                        throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::INVALID_LINK) };
                    
                    arg1Name = sumArg1LinkData.srcBlock.value()->name;
                    arg2Name = srcBlock.block->name;
                    arg1Sign = destSumInputSigns.at(destSumPort1.number);
                    arg2Sign = destSumInputSigns.at(destSumPort2.number);
                    bool arg1IsInport = sumArg1LinkData.srcBlock.value()->type == blockType::INPORT;
                    bool arg2IsInport = srcBlock.block->type == blockType::INPORT;
                    if (destArg1Port.number == destSumPort1.number && destArg2Port.number == destSumPort2.number) {
                        // nothing
                    } else if (destArg1Port.number == destSumPort2.number && destArg2Port.number == destSumPort1.number) {
                        std::swap(arg1Name, arg2Name);
                        std::swap(arg1Sign, arg2Sign);
                        std::swap(arg1IsInport, arg2IsInport);
                    } else {
                        throw std::runtime_error{ ERR_WITH_LINE(ts::messages::errors::INVALID_LINK) };
                    }
                    
                    const U::Var sumRes = U::Var{ U::Var::type::DOUBLE, destSumBlock->name, std::nullopt, std::nullopt };
                    const U::Var sumArg1 = U::Var{ U::Var::type::DOUBLE, arg1Name, std::nullopt, arg1Sign };
                    const U::Var sumArg2 = U::Var{ U::Var::type::DOUBLE, arg2Name, std::nullopt, arg2Sign };
                    result.uVars.insert({ sumRes.name.value(), sumRes });
                    result.uVars.insert({ sumArg1.name.value(), sumArg1 });
                    result.uVars.insert({ sumArg2.name.value(), sumArg2 });
                    if (arg1IsInport)
                        result.uExportVars.insert({ sumArg1.name.value(), sumArg1 });
                    if (arg2IsInport)
                        result.uExportVars.insert({ sumArg2.name.value(), sumArg2 });
                    
                    auto uOperator = std::make_shared<U::Sum>(sumRes, sumArg1, sumArg2); 
                    const bool firstArgCompleted = _incompletedBlocks.contains(srcBlockId1);
                    const bool secondArgCompleted = _incompletedBlocks.contains(srcBlockId2);
                    if (firstArgCompleted && secondArgCompleted) {
                        _ready2ArgOperators.emplace_back(Ready2ArgOperator{ MakeOperatorResult::OperatorData{ uOperator }, { srcBlockId1, srcBlockId2 } });
                    } else if (firstArgCompleted) {
                        _ready2ArgOperators.emplace_back(Ready2ArgOperator{ MakeOperatorResult::OperatorData{ uOperator }, { srcBlockId1 } });
                    } else if (secondArgCompleted) {
                        _ready2ArgOperators.emplace_back(Ready2ArgOperator{ MakeOperatorResult::OperatorData{ uOperator }, { srcBlockId2 } });
                    } else {
                        result.uOperators.emplace_back(MakeOperatorResult::OperatorData{ std::move(uOperator) });
                        assert(_incompletedBlocks.contains(destBlock.blockId));
                        _incompletedBlocks.erase(destBlock.blockId);

                        for (auto it = _ready2ArgOperators.begin(); it != _ready2ArgOperators.end();) {
                            std::erase(it->incompletedBlocks, destBlock.blockId);
                            if (it->incompletedBlocks.empty()) {
                                result.uOperators.emplace_back(it->uOperatorData);
                                it = _ready2ArgOperators.erase(it);
                            } else {
                                ++it;
                            }
                        }
                    }

                    _sumOperatorBuildData.erase(destBlock.blockId);
                    break;
                }

                _sumOperatorBuildData.insert({
                    destBlock.blockId,
                    U2ArgsOperatorBuildData{
                        U2ArgsOperatorBuildData::PointsLink{
                            srcBlock.block,
                            srcPoint,
                            destPoint
                        },
                        destBlock.block
                    }
                });

                _incompletedBlocks.insert(destBlock.blockId);

                break;
            } case ts::structures::blockType::GAIN: {
                assert(dynamic_cast<SGainBlock*>(destBlock.block.get()));
                
                SGainBlock* destGainBlock = static_cast<SGainBlock*>(destBlock.block.get());
                assert(destGainBlock);

                assert(destGainBlock->ports[0].pType == SBlock::SPort::type::IN);
                
                const U::Var multRes = U::Var{ U::Var::type::DOUBLE, destGainBlock->name, std::nullopt, std::nullopt };
                const U::Var multArg1 = U::Var{ U::Var::type::DOUBLE, srcBlock.block->name, std::nullopt, std::nullopt };
                const U::Var multArg2 = U::Var{ U::Var::type::DOUBLE, std::nullopt, std::to_string(destGainBlock->gain), U::Var::sign::PLUS };
                result.uVars.insert({ multRes.name.value(), multRes });
                result.uVars.insert({ multArg1.name.value(), multArg1 });
                if (srcBlock.block->type == blockType::INPORT)
                    result.uExportVars.insert({ multArg1.name.value(), multArg1 });

                assert(!_incompletedBlocks.contains(destBlock.blockId));
                result.uOperators.emplace_back(MakeOperatorResult::OperatorData{ std::make_shared<U::Mult>(multRes, multArg1, multArg2) });

                break;
            } case ts::structures::blockType::UNIT_DELAY: {
                assert(dynamic_cast<SUnitDelayBlock*>(destBlock.block.get()));
                
                SUnitDelayBlock* destUnitDelayBlock = static_cast<SUnitDelayBlock*>(destBlock.block.get());
                assert(destUnitDelayBlock);

                assert(destUnitDelayBlock->ports[0].pType == SBlock::SPort::type::IN);
                
                const U::Var to = U::Var{ U::Var::type::DOUBLE, destUnitDelayBlock->name, std::nullopt, std::nullopt };
                const U::Var from = U::Var{ U::Var::type::DOUBLE, srcBlock.block->name, std::nullopt, std::nullopt };
                result.uVars.insert({ to.name.value(), to });
                result.uVars.insert({ from.name.value(), from });
                result.uInitializedVars.insert({ to.name.value(), to });
                if (srcBlock.block->type == blockType::INPORT)
                    result.uExportVars.insert({ from.name.value(), from });

                assert(!_incompletedBlocks.contains(destBlock.blockId));
                result.uOperators.emplace_back(MakeOperatorResult::OperatorData{ std::make_shared<U::Assign>(to, from), uOperatorPriority::low });

                break;
            } case ts::structures::blockType::OUTPORT: {
                assert(dynamic_cast<SOutportBlock*>(destBlock.block.get()));
                
                SOutportBlock* destOutportBlock = static_cast<SOutportBlock*>(destBlock.block.get());
                assert(destOutportBlock);

                assert(destOutportBlock->ports[0].pType == SBlock::SPort::type::IN);
                
                const U::Var to = U::Var{ U::Var::type::DOUBLE, destOutportBlock->name, std::nullopt, std::nullopt };
                const U::Var from = U::Var{ U::Var::type::DOUBLE, srcBlock.block->name, std::nullopt, std::nullopt };
                result.uVars.insert({ to.name.value(), to });
                result.uVars.insert({ from.name.value(), from });
                result.uExportVars.insert({ to.name.value(), to });

                assert(!_incompletedBlocks.contains(destBlock.blockId));
                result.uOperators.emplace_back(MakeOperatorResult::OperatorData{ std::make_shared<U::Assign>(to, from), uOperatorPriority::normal });

                break;
            }
        }

        return result;
    }
}