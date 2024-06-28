#include "xml_source_file.hpp"

#include <format>
#include <stdexcept>

#include <pugixml.hpp>

#include "logger.hpp"
#include "../utility/config.hpp"
#include "../utility/messages.hpp"

namespace ts {
    XmlSourceFile::XmlSourceFile(std::string_view path) noexcept 
        : SourceFile{ path }
    {

    }

    UFormat XmlSourceFile::toUFormat() const {
        UFormat uFormat;

        pugi::xml_document xmlDocument;
        pugi::xml_parse_result parseResult = xmlDocument.load_file(_sourcePath.c_str());
        if (!parseResult)
            throw std::runtime_error{ messages::errors::NO_SOURCE };
            
        // for (const pugi::xml_node& node : xmlDocument.child("System").children("Block")) {
        //     // Logger::instance().log("ALLAH\n");
        //     // std::string name = tool.attribute("BlockType").as_string();
        //     Logger::instance().log(std::format("{}\n", tool.attribute("BlockType").value()));
        //     // if (timeout > 0)
        //     //     Logger::instance().log("ALLAH");
        //         // std::cout << "Tool " << tool.attribute("Filename").value() << " has timeout " << timeout << "\n";
        // }

        return uFormat;
    }
}