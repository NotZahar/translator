#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <unordered_map>

#include "../source/selements.hpp"
#include "uformat.hpp"
#include "core/source/sblock.hpp"
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
        class UCodeLineBuilder {
        public:
            struct BlockData {
                structures::SElements::blockId_t blockId;
                const std::unique_ptr<structures::SBlock>& block;
            };

            UCodeLineBuilder() noexcept = default;

            ~UCodeLineBuilder() = default;

            [[nodiscard]] std::unique_ptr<U::Operator> makeOperatorLine(
                BlockData srcBlock, 
                BlockData destBlock,
                const structures::SLink::SPoint& srcPoint,
                const structures::SLink::SPoint& destPoint);
        
            [[nodiscard]] bool extraBuildDataExists() const noexcept;

        private:
            struct UOperator2BuildData {
                struct PointsLink {
                    std::optional<structures::SBlock*> srcBlock;
                    std::optional<structures::SLink::SPoint> srcPoint;
                    std::optional<structures::SLink::SPoint> destPoint;    
                };
                
                bool oneDestPointAlreadyOccupied() const noexcept {
                    assert(!src1.srcBlock || !src2.srcBlock);
                    return (src1.srcBlock && src1.srcPoint && src1.destPoint)
                        || (src2.srcBlock && src2.srcPoint && src2.destPoint);
                }

                PointsLink getOccupiedPointsLink() const noexcept {
                    assert(oneDestPointAlreadyOccupied());
                    if (src1.srcBlock) {
                        assert(src1.srcPoint && src1.destPoint);
                        return src1;
                    }

                    if (src2.srcBlock) {
                        assert(src2.srcPoint && src2.destPoint);
                        return src2;
                    }

                    assert(false);
                    return {};
                }

                PointsLink getFreePointsLink() const noexcept {
                    if (!src1.srcBlock) {
                        assert(!src1.srcBlock && !src1.srcPoint && !src1.destPoint);
                        return src1;
                    }

                    if (!src2.srcBlock) {
                        assert(!src2.srcBlock && !src2.srcPoint && !src2.destPoint);
                        return src2;
                    }

                    assert(false);
                    return {};    
                }

                PointsLink src1;
                PointsLink src2;
                structures::SBlock* destBlock;
            };

            // key == dest operator block id
            std::unordered_map<structures::SElements::blockId_t, UOperator2BuildData> _sumOperatorBuildData;
            std::unordered_map<structures::SElements::blockId_t, UOperator2BuildData> _multOperatorBuildData;
        };

        void makeUCode(
            splittedLinks_t splittedLinks,
            const std::unordered_map<structures::SElements::blockId_t, std::unique_ptr<structures::SBlock>>& inVars,
            const std::unordered_map<structures::SElements::blockId_t, std::unique_ptr<structures::SBlock>>& outVars,
            const std::unordered_map<structures::SElements::blockId_t, std::unique_ptr<structures::SBlock>>& operators);
    };
}