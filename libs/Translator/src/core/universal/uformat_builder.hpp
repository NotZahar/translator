#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../source/selements.hpp"
#include "uformat.hpp"
#include "../source/sblock.hpp"
#include "u.hpp"

namespace ts {
    class UFormatBuilder {
    public:
        using splittedLink_t = std::unordered_multimap<structures::SLink::SPoint, structures::SLink::SPoint, structures::SPointHash, structures::SPointEqual>;
        using splittedLinks_t = std::unordered_map<structures::SElements::blockId_t, splittedLink_t>;

        constexpr UFormatBuilder() noexcept = default;

        ~UFormatBuilder() = default;

        [[nodiscard]] U::UFormat build(structures::SElements& sElements);

    private:
        enum class uOperatorPriority {
            high,
            normal,
            low
        };

        struct BlockData {
            structures::SElements::blockId_t blockId;
            std::shared_ptr<structures::SBlock> block;
        };

        struct MakeUResult {
            std::vector<U::Var> uVars;
            std::vector<U::Var> uExportVars;
            std::vector<std::shared_ptr<U::Operator>> uCode;
        };

        struct ProcessLayerResult {
            std::unordered_map<structures::SElements::blockId_t, std::shared_ptr<structures::SBlock>> blocksToProcessNext;
            std::unordered_map<structures::SElements::blockId_t, std::shared_ptr<structures::SBlock>> blocksToProcessInFuture;
        };

        struct MakeOperatorResult {
            struct OperatorData {
                std::shared_ptr<U::Operator> uOperator;
                uOperatorPriority uPriority = uOperatorPriority::high;
            };

            std::vector<OperatorData> uOperators;
        };

        struct Ready2ArgOperator {
            MakeOperatorResult::OperatorData uOperatorData;
            std::vector<structures::SElements::blockId_t> incompletedBlocks;
        };

        class UCodeOperatorBuilder {
        private:
            struct U2ArgsOperatorBuildData {
                struct PointsLink {
                    std::optional<std::shared_ptr<structures::SBlock>> srcBlock;
                    std::optional<structures::SLink::SPoint> srcPoint;
                    std::optional<structures::SLink::SPoint> destPoint;
                };
                
                [[nodiscard]] bool linkExists() const noexcept {
                    return link.srcBlock && link.srcPoint && link.destPoint;
                }

                [[nodiscard]] PointsLink getLink() const noexcept {
                    assert(linkExists());
                    return link;
                }

                PointsLink link;
                std::shared_ptr<structures::SBlock> destBlock;
            };

        public:
            UCodeOperatorBuilder() noexcept = default;

            ~UCodeOperatorBuilder() = default;

            [[nodiscard]] MakeOperatorResult makeOperator(
                BlockData srcBlock, 
                BlockData destBlock,
                const structures::SLink::SPoint& srcPoint,
                const structures::SLink::SPoint& destPoint);

        private:
            // key == dest operator block id
            std::unordered_map<structures::SElements::blockId_t, U2ArgsOperatorBuildData> _sumOperatorBuildData;
            std::unordered_set<structures::SElements::blockId_t> _incompletedBlocks;
            std::vector<Ready2ArgOperator> _ready2ArgOperators;
        };

        [[nodiscard]] ProcessLayerResult processLinks(
            const splittedLinks_t& splittedLinks,
            const std::unordered_map<structures::SElements::blockId_t, std::shared_ptr<structures::SBlock>>& srcBlocks,
            const std::unordered_map<structures::SElements::blockId_t, std::shared_ptr<structures::SBlock>>& destBlocks,
            UCodeOperatorBuilder& uCodeOperatorBuilder,
            std::vector<std::shared_ptr<U::Operator>>& uCodeHigh,
            std::vector<std::shared_ptr<U::Operator>>& uCodeNormal,
            std::vector<std::shared_ptr<U::Operator>>& uCodeLow,
            bool forwardDirection = true) const;

        [[nodiscard]] MakeUResult makeU(
            splittedLinks_t splittedLinks,
            const std::unordered_map<structures::SElements::blockId_t, std::shared_ptr<structures::SBlock>>& inVars,
            const std::unordered_map<structures::SElements::blockId_t, std::shared_ptr<structures::SBlock>>& operators) const;
    };
}