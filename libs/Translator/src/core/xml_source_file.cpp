#include "xml_source_file.hpp"

#include <cassert>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <regex>

#include <boost/bimap.hpp>
#include <boost/assign.hpp>

#include <pugixml.hpp>

#include "core/structures/ublock.hpp"
#include "core/structures/ulink.hpp"
#include "core/u.hpp"
#include "structures/uelements.hpp"
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

    std::unique_ptr<UBlock> makeInportBlock(const std::string& nameValue) {
        using UPort = UInportBlock::UPort;

        const std::array<UPort, UInportBlock::numberOfPorts> ports{ 
            UPort{ UPort::defaultNumber, UPort::type::OUT }
        };

        return std::make_unique<UInportBlock>(ts::U::block::INPORT, nameValue, ports);
    }

    std::unique_ptr<UBlock> makeSumBlock(const std::string& nameValue, const std::string& pNodePattern, const std::string& nameAttributeNamePattern, const pugi::xml_node& blockNode) {
        using UPort = USumBlock::UPort;

        const std::string& inputsAttributeValuePattern = xmlAttributeValues.left.find(xmlAttributeValue::INPUTS)->second;

        int portNumber = UPort::defaultNumber;
        std::array<int, USumBlock::numberOfPorts> portNumbers{
            portNumber++,
            portNumber++,
            portNumber
        };
        
        const std::array<UPort, USumBlock::numberOfPorts> ports{
            UPort{ portNumbers[0], UPort::type::IN },
            UPort{ portNumbers[1], UPort::type::IN },
            UPort{ portNumbers[2], UPort::type::OUT }
        };

        std::unordered_map<USumBlock::inputPortNumber_t, USumBlock::sign> inputSigns;
        
        const auto inputsNode = blockNode.find_child_by_attribute(pNodePattern.c_str(), nameAttributeNamePattern.c_str(), inputsAttributeValuePattern.c_str());
    
        if (inputsNode.empty()) {
            inputSigns.insert({ portNumbers[0], USumBlock::sign::PLUS });
            inputSigns.insert({ portNumbers[1], USumBlock::sign::PLUS });
        } else {
            const std::string inputs = inputsNode.text().as_string();
            if (inputs.empty())
                throw std::runtime_error{ ts::messages::errors::INVALID_SUM_BLOCK };

            if (!std::regex_match(inputs, inputsValueRegex))
                throw std::runtime_error{ ts::messages::errors::INVALID_SUM_BLOCK };

            assert(inputs.size() == (USumBlock::numberOfPorts - 1));
            inputSigns.insert({ portNumbers[0], USumBlock::signs.right.find(inputs[0])->second });
            inputSigns.insert({ portNumbers[1], USumBlock::signs.right.find(inputs[1])->second });
        }

        return std::make_unique<USumBlock>(ts::U::block::SUM, nameValue, ports, inputSigns);
    }

    std::unique_ptr<UBlock> makeGainBlock(const std::string& nameValue, const std::string& pNodePattern, const std::string& nameAttributeNamePattern, const pugi::xml_node& blockNode) {
        using UPort = UGainBlock::UPort;

        const std::string& gainAttributeValuePattern = xmlAttributeValues.left.find(xmlAttributeValue::GAIN)->second;

        int portNumber = UPort::defaultNumber;
        std::array<int, UGainBlock::numberOfPorts> portNumbers{
            portNumber++,
            portNumber,
        };
        
        const std::array<UPort, UGainBlock::numberOfPorts> ports{
            UPort{ portNumbers[0], UPort::type::IN },
            UPort{ portNumbers[1], UPort::type::OUT }
        };

        const auto gainNode = blockNode.find_child_by_attribute(pNodePattern.c_str(), nameAttributeNamePattern.c_str(), gainAttributeValuePattern.c_str());
        if (gainNode.empty())
            throw std::runtime_error{ ts::messages::errors::INVALID_GAIN_BLOCK };

        return std::make_unique<UGainBlock>(ts::U::block::GAIN, nameValue, ports, std::stod(gainNode.text().as_string()));
    }

    std::unique_ptr<UBlock> makeUnitDelayBlock(const std::string& nameValue, const std::string& pNodePattern, const std::string& nameAttributeNamePattern, const pugi::xml_node& blockNode) {
        using UPort = UUnitDelayBlock::UPort;

        const std::string& sampleTimeAttributeValuePattern = xmlAttributeValues.left.find(xmlAttributeValue::SAMPLE_TIME)->second;

        int portNumber = UPort::defaultNumber;
        std::array<int, UUnitDelayBlock::numberOfPorts> portNumbers{
            portNumber++,
            portNumber,
        };
        
        const std::array<UPort, UUnitDelayBlock::numberOfPorts> ports{
            UPort{ portNumbers[0], UPort::type::IN },
            UPort{ portNumbers[1], UPort::type::OUT }
        };

        const auto sampleTimeNode = blockNode.find_child_by_attribute(pNodePattern.c_str(), nameAttributeNamePattern.c_str(), sampleTimeAttributeValuePattern.c_str());
        if (sampleTimeNode.empty())
            throw std::runtime_error{ ts::messages::errors::INVALID_UNIT_DELAY_BLOCK };

        return std::make_unique<UUnitDelayBlock>(ts::U::block::UNIT_DELAY, nameValue, ports, std::stoi(sampleTimeNode.text().as_string()));
    }

    std::unique_ptr<UBlock> makeOutportBlock(const std::string& nameValue) {
        using UPort = UOutportBlock::UPort;

        const std::array<UPort, UOutportBlock::numberOfPorts> ports{ 
            UPort{ UPort::defaultNumber, UPort::type::IN }
        };

        return std::make_unique<UOutportBlock>(ts::U::block::OUTPORT, nameValue, ports);
    }

    std::unordered_map<UElements::blockId_t, std::unique_ptr<UBlock>> getBlocks(const pugi::xml_node& systemNode) {
        std::unordered_map<UElements::blockId_t, std::unique_ptr<UBlock>> uBlocks;

        const std::string& blockNodePattern = xmlNodes.left.find(xmlNode::BLOCK)->second;
        const std::string& pNodePattern = xmlNodes.left.find(xmlNode::P)->second;

        const std::string& blockTypeAttributeNamePattern = xmlAttributeNames.left.find(xmlAttributeName::BLOCK_TYPE)->second;
        const std::string& nameAttributeNamePattern = xmlAttributeNames.left.find(xmlAttributeName::NAME)->second;
        const std::string& idAttributeNamePattern = xmlAttributeNames.left.find(xmlAttributeName::ID)->second;

        const auto blockNodes = systemNode.children(blockNodePattern.c_str());
        for (const pugi::xml_node& node : blockNodes) {
            std::unique_ptr<UBlock> uBlock;

            const std::string blockTypeValue = node.attribute(blockTypeAttributeNamePattern.c_str()).as_string();
            const std::string blockNameValue = node.attribute(nameAttributeNamePattern.c_str()).as_string();
            const int idValue = node.attribute(idAttributeNamePattern.c_str()).as_int();

            if (blockTypeValue.empty())
                throw std::runtime_error{ ts::messages::errors::NO_BLOCK_TYPE };

            if (ts::U::blocks.right.count(blockTypeValue) == 0)
                throw std::runtime_error{ ts::messages::errors::INVALID_BLOCK_TYPE };
            
            const ts::U::block blockType = ts::U::blocks.right.find(blockTypeValue)->second;
            switch (blockType) {
                case ts::U::block::INPORT: {
                    uBlock = makeInportBlock(blockNameValue);
                    break;
                } case ts::U::block::SUM: {
                    uBlock = makeSumBlock(blockNameValue, pNodePattern, nameAttributeNamePattern, node);
                    break;
                } case ts::U::block::GAIN: {
                    uBlock = makeGainBlock(blockNameValue, pNodePattern, nameAttributeNamePattern, node);
                    break;
                } case ts::U::block::UNIT_DELAY: {
                    uBlock = makeUnitDelayBlock(blockNameValue, pNodePattern, nameAttributeNamePattern, node);
                    break;
                } case ts::U::block::OUTPORT: {
                    uBlock = makeOutportBlock(blockNameValue);
                    break;
                }
            }

            assert(!uBlocks.contains(idValue));
            uBlocks.insert({ idValue, std::move(uBlock) });
        }

        return uBlocks;
    }

    std::vector<ULink> getLinks(const pugi::xml_node& systemNode) {
        auto getPoint = [](const std::string& string) -> ULink::UPoint {
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
            assert(UBlock::UPort::types.right.count(portType) != 0);
            return ULink::UPoint{
                numbers[0],
                UBlock::UPort{
                    numbers[1],
                    UBlock::UPort::types.right.find(portType)->second
                }
            };
        };

        std::vector<ULink> links;

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

            ULink::UPoint srcPoint = getPoint(source);

            // destination
            std::vector<ULink::UPoint> destinations;
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

            links.emplace_back(ULink{ std::move(srcPoint), std::move(destinations) });
        }

        return links; 
    }
}

namespace ts {
    XmlSourceFile::XmlSourceFile(std::string_view path) noexcept 
        : SourceFile{ path }
    {

    }

    structures::UElements XmlSourceFile::getElements() const {
        pugi::xml_document xmlDocument;
        pugi::xml_parse_result parseResult = xmlDocument.load_file(_sourcePath.c_str());
        if (!parseResult)
            throw std::runtime_error{ messages::errors::NO_SOURCE };
            
        const std::string& systemNodePattern = xmlNodes.left.find(xmlNode::SYSTEM)->second;
        const pugi::xml_node systemNode = xmlDocument.child(systemNodePattern.c_str());

        return structures::UElements{ 
            getBlocks(systemNode), 
            getLinks(systemNode) 
        };
    }
}