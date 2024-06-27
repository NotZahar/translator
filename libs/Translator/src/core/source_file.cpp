#include "source_file.hpp"

#include <format>
#include <stdexcept>

#include <pugixml.hpp>

#include "../logger.hpp"
#include "../utility/config.hpp"
#include "../utility/messages.hpp"

namespace ts {
    SourceFile::SourceFile(std::string_view path) noexcept 
        : _sourcePath{ path }
    {

    }

    UFormat SourceFile::toUFormat() const {
        UFormat uFormat;

        // pugi::xml_document doc;
        // pugi::xml_parse_result result = doc.load_file(config::sourcePath.c_str());
        // if (!result)
        //     throw std::runtime_error{ messages::errors::NO_SOURCE };
            
        // for (pugi::xml_node tool : doc.child("System").children("Block")) {
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