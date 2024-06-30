#pragma once

#include <memory>
#include <string_view>

#include <boost/bimap.hpp>
#include <boost/assign.hpp>

#include "options_parser.hpp"
#include "../src/core/source_file.hpp"

namespace ts {
    class Translator {
    public:
        Translator() = delete;
        explicit Translator(OptionsParser::Options options) noexcept;
        
        ~Translator() = default;

        void translate();
        [[nodiscard]] std::string_view getTranslated() const noexcept;

    private:
        enum class validExtension {
            XML
        };

        inline static const boost::bimap<validExtension, std::filesystem::path> validExtensions = 
            boost::assign::list_of<boost::bimap<validExtension, std::filesystem::path>::relation>
            ( validExtension::XML, ".xml" );

        [[nodiscard]] std::unique_ptr<SourceFile> identifySourceFile(std::string_view path) const noexcept; 

        std::string _translated;
    };
}
