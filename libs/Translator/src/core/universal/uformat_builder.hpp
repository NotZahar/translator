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
        class UCodeOperatorBuilder {
        public:
            struct BlockData {
                structures::SElements::blockId_t blockId;
                const std::unique_ptr<structures::SBlock>& block;
            };

            UCodeOperatorBuilder() noexcept = default;

            ~UCodeOperatorBuilder() = default;

            [[nodiscard]] std::unique_ptr<U::Operator> makeOperator(
                BlockData srcBlock, 
                BlockData destBlock,
                const structures::SLink::SPoint& srcPoint,
                const structures::SLink::SPoint& destPoint);
        
            [[nodiscard]] bool extraBuildDataExists() const noexcept;

        private:
            struct U2ArgsOperatorBuildData {
                struct PointsLink {
                    std::optional<structures::SBlock*> srcBlock;
                    std::optional<structures::SLink::SPoint> srcPoint;
                    std::optional<structures::SLink::SPoint> destPoint;
                };
                
                bool linkExists() const noexcept {
                    return link.srcBlock && link.srcPoint && link.destPoint;
                }

                PointsLink getLink() const noexcept {
                    assert(linkExists());
                    return link;
                }

                PointsLink link;
                structures::SBlock* destBlock;
            };

            // key == dest operator block id
            std::unordered_map<structures::SElements::blockId_t, U2ArgsOperatorBuildData> _sumOperatorBuildData;
            std::unordered_map<structures::SElements::blockId_t, U2ArgsOperatorBuildData> _multOperatorBuildData;
        };

        void makeUCode(
            splittedLinks_t splittedLinks,
            const std::unordered_map<structures::SElements::blockId_t, std::unique_ptr<structures::SBlock>>& inVars,
            const std::unordered_map<structures::SElements::blockId_t, std::unique_ptr<structures::SBlock>>& outVars,
            const std::unordered_map<structures::SElements::blockId_t, std::unique_ptr<structures::SBlock>>& operators);
    };
}