#include "xml_source_file.hpp"

#include <cassert>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <regex>

#include <boost/bimap.hpp>
#include <boost/assign.hpp>

#include <pugixml.hpp>

#include "source/sblock.hpp"
#include "source/slink.hpp"
#include "source/selements.hpp"
#include "../utility/messages.hpp"

 namespace {
    using namespace ts::structures;

    enum class xmlNode {
        SYSTEM,
        BLOCK,
        P,
        PORT,
        LINE,
        BRANCH
    };

    enum class xmlAttributeName {
        BLOCK_TYPE,
        NAME,
        ID
    };

    enum class xmlAttributeValue {
        INPUTS,
        GAIN,
        SAMPLE_TIME,
        SOURCE,
        DESTINATION
    };

    inline static const boost::bimap<xmlNode, std::string> xmlNodes = 
        boost::assign::list_of<boost::bimap<xmlNode, std::string>::relation>
        ( xmlNode::SYSTEM, "System" )
        ( xmlNode::BLOCK, "Block" )
        ( xmlNode::P, "P" )
        ( xmlNode::PORT, "Port" )
        ( xmlNode::LINE, "Line" )
        ( xmlNode::BRANCH, "Branch" );

    inline static const boost::bimap<xmlAttributeName, std::string> xmlAttributeNames = 
        boost::assign::list_of<boost::bimap<xmlAttributeName, std::string>::relation>
        ( xmlAttributeName::BLOCK_TYPE, "BlockType" )
        ( xmlAttributeName::NAME, "Name" )
        ( xmlAttributeName::ID, "SID" );

    inline static const boost::bimap<xmlAttributeValue, std::string> xmlAttributeValues = 
        boost::assign::list_of<boost::bimap<xmlAttributeValue, std::string>::relation>
        ( xmlAttributeValue::INPUTS, "Inputs" )
        ( xmlAttributeValue::GAIN, "Gain" )
        ( xmlAttributeValue::SAMPLE_TIME, "SampleTime" )
        ( xmlAttributeValue::SOURCE, "Src" )
        ( xmlAttributeValue::DESTINATION, "Dst" );

    inline static const std::regex inputsValueRegex{ "^[\\+\\-]{2}$" };
    inline static const std::regex upointValueRegex{ "^[1-9][0-9]*\\#(in|out)\\:[1-9][0-9]*$" };
    inline static const std::regex upointNumberValueRegex{ "[1-9][0-9]*" };
    inline static const std::regex upointPortTypeValueRegex{ "(in|out)" };

    [[nodiscard]] std::unique_ptr<SBlock> makeInportBlock(const std::string& nameValue) {
        using SPort = SInportBlock::SPort;

        const std::array<SPort, SInportBlock::numberOfPorts> ports{ 
            SPort{ SPort::defaultNumber, SPort::type::OUT }
        };

        return std::make_unique<SInportBlock>(blockType::INPORT, nameValue, ports);
    }

    [[nodiscard]] std::unique_ptr<SBlock> makeSumBlock(const std::string& nameValue, const std::string& pNodePattern, const std::string& nameAttributeNamePattern, const pugi::xml_node& blockNode) {
        using SPort = SSumBlock::SPort;

        const std::string& inputsAttributeValuePattern = xmlAttributeValues.left.find(xmlAttributeValue::INPUTS)->second;

        int portNumber = SPort::defaultNumber;
        std::array<int, SSumBlock::numberOfPorts> portNumbers{
            portNumber++,
            portNumber++,
            portNumber
        };
        
        const std::array<SPort, SSumBlock::numberOfPorts> ports{
            SPort{ portNumbers[0], SPort::type::IN },
            SPort{ portNumbers[1], SPort::type::IN },
            SPort{ portNumbers[2], SPort::type::OUT }
        };

        std::unordered_map<SSumBlock::inputPortNumber_t, SSumBlock::sign> inputSigns;
        
        const auto inputsNode = blockNode.find_child_by_attribute(pNodePattern.c_str(), nameAttributeNamePattern.c_str(), inputsAttributeValuePattern.c_str());
    
        if (inputsNode.empty()) {
            inputSigns.insert({ portNumbers[0], SSumBlock::sign::PLUS });
            inputSigns.insert({ portNumbers[1], SSumBlock::sign::PLUS });
        } else {
            const std::string inputs = inputsNode.text().as_string();
            if (inputs.empty())
                throw std::runtime_error{ ts::messages::errors::INVALID_SUM_BLOCK };

            if (!std::regex_match(inputs, inputsValueRegex))
                throw std::runtime_error{ ts::messages::errors::INVALID_SUM_BLOCK };

            assert(inputs.size() == (SSumBlock::numberOfPorts - 1));
            inputSigns.insert({ portNumbers[0], SSumBlock::signs.right.find(inputs[0])->second });
            inputSigns.insert({ portNumbers[1], SSumBlock::signs.right.find(inputs[1])->second });
        }

        return std::make_unique<SSumBlock>(blockType::SUM, nameValue, ports, inputSigns);
    }

    [[nodiscard]] std::unique_ptr<SBlock> makeGainBlock(const std::string& nameValue, const std::string& pNodePattern, const std::string& nameAttributeNamePattern, const pugi::xml_node& blockNode) {
        using SPort = SGainBlock::SPort;

        const std::string& gainAttributeValuePattern = xmlAttributeValues.left.find(xmlAttributeValue::GAIN)->second;

        int portNumber = SPort::defaultNumber;
        std::array<int, SGainBlock::numberOfPorts> portNumbers{
            portNumber++,
            portNumber,
        };
        
        const std::array<SPort, SGainBlock::numberOfPorts> ports{
            SPort{ portNumbers[0], SPort::type::IN },
            SPort{ portNumbers[1], SPort::type::OUT }
        };

        const auto gainNode = blockNode.find_child_by_attribute(pNodePattern.c_str(), nameAttributeNamePattern.c_str(), gainAttributeValuePattern.c_str());
        if (gainNode.empty())
            throw std::runtime_error{ ts::messages::errors::INVALID_GAIN_BLOCK };

        return std::make_unique<SGainBlock>(blockType::GAIN, nameValue, ports, std::stod(gainNode.text().as_string()));
    }

    [[nodiscard]] std::unique_ptr<SBlock> makeUnitDelayBlock(const std::string& nameValue, const std::string& pNodePattern, const std::string& nameAttributeNamePattern, const pugi::xml_node& blockNode) {
        using SPort = SUnitDelayBlock::SPort;

        const std::string& sampleTimeAttributeValuePattern = xmlAttributeValues.left.find(xmlAttributeValue::SAMPLE_TIME)->second;

        int portNumber = SPort::defaultNumber;
        std::array<int, SUnitDelayBlock::numberOfPorts> portNumbers{
            portNumber++,
            portNumber,
        };
        
        const std::array<SPort, SUnitDelayBlock::numberOfPorts> ports{
            SPort{ portNumbers[0], SPort::type::IN },
            SPort{ portNumbers[1], SPort::type::OUT }
        };

        const auto sampleTimeNode = blockNode.find_child_by_attribute(pNodePattern.c_str(), nameAttributeNamePattern.c_str(), sampleTimeAttributeValuePattern.c_str());
        if (sampleTimeNode.empty())
            throw std::runtime_error{ ts::messages::errors::INVALID_UNIT_DELAY_BLOCK };

        return std::make_unique<SUnitDelayBlock>(blockType::UNIT_DELAY, nameValue, ports, std::stoi(sampleTimeNode.text().as_string()));
    }

    [[nodiscard]] std::unique_ptr<SBlock> makeOutportBlock(const std::string& nameValue) {
        using SPort = SOutportBlock::SPort;

        const std::array<SPort, SOutportBlock::numberOfPorts> ports{ 
            SPort{ SPort::defaultNumber, SPort::type::IN }
        };

        return std::make_unique<SOutportBlock>(blockType::OUTPORT, nameValue, ports);
    }

    [[nodiscard]] std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>> getBlocks(const pugi::xml_node& systemNode) {
        std::unordered_map<SElements::blockId_t, std::unique_ptr<SBlock>> sBlocks;

        const std::string& blockNodePattern = xmlNodes.left.find(xmlNode::BLOCK)->second;
        const std::string& pNodePattern = xmlNodes.left.find(xmlNode::P)->second;

        const std::string& blockTypeAttributeNamePattern = xmlAttributeNames.left.find(xmlAttributeName::BLOCK_TYPE)->second;
        const std::string& nameAttributeNamePattern = xmlAttributeNames.left.find(xmlAttributeName::NAME)->second;
        const std::string& idAttributeNamePattern = xmlAttributeNames.left.find(xmlAttributeName::ID)->second;

        const auto blockNodes = systemNode.children(blockNodePattern.c_str());
        for (const pugi::xml_node& node : blockNodes) {
            std::unique_ptr<SBlock> sBlock;

            const std::string blockTypeValue = node.attribute(blockTypeAttributeNamePattern.c_str()).as_string();
            const std::string blockNameValue = node.attribute(nameAttributeNamePattern.c_str()).as_string();
            const int idValue = node.attribute(idAttributeNamePattern.c_str()).as_int();

            if (blockTypeValue.empty())
                throw std::runtime_error{ ts::messages::errors::NO_BLOCK_TYPE };

            if (blockTypes.right.count(blockTypeValue) == 0)
                throw std::runtime_error{ ts::messages::errors::INVALID_BLOCK_TYPE };
            
            const blockType blockTp = blockTypes.right.find(blockTypeValue)->second;
            switch (blockTp) {
                case blockType::INPORT: {
                    sBlock = makeInportBlock(blockNameValue);
                    break;
                } case blockType::SUM: {
                    sBlock = makeSumBlock(blockNameValue, pNodePattern, nameAttributeNamePattern, node);
                    break;
                } case blockType::GAIN: {
                    sBlock = makeGainBlock(blockNameValue, pNodePattern, nameAttributeNamePattern, node);
                    break;
                } case blockType::UNIT_DELAY: {
                    sBlock = makeUnitDelayBlock(blockNameValue, pNodePattern, nameAttributeNamePattern, node);
                    break;
                } case blockType::OUTPORT: {
                    sBlock = makeOutportBlock(blockNameValue);
                    break;
                }
            }

            assert(!sBlocks.contains(idValue));
            assert(!sBlock->name.empty());
            sBlocks.insert({ idValue, std::move(sBlock) });
        }

        return sBlocks;
    }

    [[nodiscard]] std::vector<SLink> getLinks(const pugi::xml_node& systemNode) {
        auto getPoint = [](const std::string& string) -> SLink::SPoint {
            auto numberRegexBegin = std::sregex_iterator(string.begin(), string.end(), upointNumberValueRegex);
            auto numberRegexEnd = std::sregex_iterator();
            std::vector<int> numbers;
            for (std::sregex_iterator i = numberRegexBegin; i != numberRegexEnd; ++i) {
                std::smatch match = *i;
                const std::string matchStr = match.str();
                numbers.emplace_back(std::stoi(matchStr));
            }

            auto portTypeRegexBegin = std::sregex_iterator(string.begin(), string.end(), upointPortTypeValueRegex);
            auto portTypeRegexEnd = std::sregex_iterator();
            std::string portType;
            for (std::sregex_iterator i = portTypeRegexBegin; i != portTypeRegexEnd; ++i) {
                std::smatch match = *i;
                portType = match.str();
            }

            assert(numbers.size() == 2); // blockId, portNumber
            assert(SBlock::SPort::types.right.count(portType) != 0);
            return SLink::SPoint{
                numbers[0],
                SBlock::SPort{
                    numbers[1],
                    SBlock::SPort::types.right.find(portType)->second
                }
            };
        };

        std::vector<SLink> links;

        const std::string& linkNodePattern = xmlNodes.left.find(xmlNode::LINE)->second;
        const std::string& pNodePattern = xmlNodes.left.find(xmlNode::P)->second;
        const std::string& branchNodePattern = xmlNodes.left.find(xmlNode::BRANCH)->second;

        const std::string& nameAttributeNamePattern = xmlAttributeNames.left.find(xmlAttributeName::NAME)->second;
        
        const std::string& sourceAttributeValuePattern = xmlAttributeValues.left.find(xmlAttributeValue::SOURCE)->second;
        const std::string& destinationAttributeValuePattern = xmlAttributeValues.left.find(xmlAttributeValue::DESTINATION)->second;

        const auto linkNodes = systemNode.children(linkNodePattern.c_str());
        for (const pugi::xml_node& linkNode : linkNodes) {
            // source
            const auto srcNode = linkNode.find_child_by_attribute(pNodePattern.c_str(), nameAttributeNamePattern.c_str(), sourceAttributeValuePattern.c_str());
            if (srcNode.empty())
                throw std::runtime_error{ ts::messages::errors::INVALID_LINE_BLOCK };

            const std::string source = srcNode.text().as_string();
            if (!std::regex_match(source, upointValueRegex))
                throw std::runtime_error{ ts::messages::errors::INVALID_LINE_BLOCK };

            SLink::SPoint srcPoint = getPoint(source);

            // destination
            std::vector<SLink::SPoint> destinations;
            const auto dstNode = linkNode.find_child_by_attribute(pNodePattern.c_str(), nameAttributeNamePattern.c_str(), destinationAttributeValuePattern.c_str());
            if (dstNode.empty()) {
                const auto branchNodes = linkNode.children(branchNodePattern.c_str());
                for (const pugi::xml_node& branchNode : branchNodes) {
                    const auto dstNode = branchNode.find_child_by_attribute(pNodePattern.c_str(), nameAttributeNamePattern.c_str(), destinationAttributeValuePattern.c_str());
                    if (dstNode.empty())
                        throw std::runtime_error{ ts::messages::errors::INVALID_LINE_BLOCK };

                    const std::string destination = dstNode.text().as_string();
                    if (!std::regex_match(destination, upointValueRegex))
                        throw std::runtime_error{ ts::messages::errors::INVALID_LINE_BLOCK };
                    
                    destinations.emplace_back(getPoint(destination));
                }
            } else {
                const std::string destination = dstNode.text().as_string();
                if (!std::regex_match(destination, upointValueRegex))
                    throw std::runtime_error{ ts::messages::errors::INVALID_LINE_BLOCK };

                destinations.emplace_back(getPoint(destination));
            }

            links.emplace_back(SLink{ std::move(srcPoint), std::move(destinations) });
        }

        return links; 
    }
}

namespace ts {
    XmlSourceFile::XmlSourceFile(std::string_view path) noexcept 
        : SourceFile{ path }
    {

    }

    structures::SElements XmlSourceFile::getElements() const {
        pugi::xml_document xmlDocument;
        pugi::xml_parse_result parseResult = xmlDocument.load_file(_sourcePath.c_str());
        if (!parseResult)
            throw std::runtime_error{ messages::errors::NO_SOURCE };
            
        const std::string& systemNodePattern = xmlNodes.left.find(xmlNode::SYSTEM)->second;
        const pugi::xml_node systemNode = xmlDocument.child(systemNodePattern.c_str());

        return structures::SElements{ 
            getBlocks(systemNode), 
            getLinks(systemNode) 
        };
    }
}