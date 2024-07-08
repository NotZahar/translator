#include "translator.hpp"

#include <filesystem>
#include <stdexcept>

#include "core/xml_source_file.hpp"
#include "core/universal/uformat_builder.hpp"
#include "core/backend/c.hpp"
#include "utility/config.hpp"
#include "utility/messages.hpp"

namespace ts {
    Translator::Translator(OptionsParser::Options options) noexcept
    {
        config::sourcePath = options.sourcePath;
    }

    void Translator::translate() {
        auto sourceFile = identifySourceFile(config::sourcePath);
        if (!sourceFile)
            throw std::runtime_error{ messages::errors::INVAILD_SOURCE };

        UFormatBuilder uFormatBuilder;
        auto sElements = sourceFile->getElements();
        auto uFormat = uFormatBuilder.build(sElements);
        _translated = backend::C::toC(std::move(uFormat));
    }

    std::string_view Translator::getTranslated() const noexcept {
        return _translated;
    }

    std::unique_ptr<SourceFile> Translator::identifySourceFile(std::string_view path) const noexcept {
        const std::filesystem::path sourcePath{ path };
        if (!std::filesystem::exists(sourcePath))
            return nullptr;

        const auto extension = validExtensions.right.find(sourcePath.extension())->second;
        
        switch (extension) {
            case validExtension::XML:
                return std::make_unique<XmlSourceFile>(path);
            default:
                return nullptr;
        }
    }
}