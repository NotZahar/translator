cmake_minimum_required(VERSION 3.28)

project("Translator" VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
    Boost 1.82 REQUIRED 
        COMPONENTS 
            system
            program_options
)
find_package(
    SQLiteCpp REQUIRED
)

add_library(
    ${PROJECT_NAME} STATIC
)

target_sources(
    ${PROJECT_NAME}
        PRIVATE
            src/translator.cpp
            src/options_parser.cpp
            src/logger.cpp

            # core
            src/core/source_file.cpp
            src/core/xml_source_file.cpp
)

target_include_directories(
    ${PROJECT_NAME}
        PRIVATE
            # where the library itself will look for its internal headers
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/pugixml/include
        PUBLIC
            # where top-level project will look for the library's public headers
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            # where external projects will look for the library's public headers
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# without it public headers won't get installed
set(
    PUBLIC_HEADERS
        include/translator.hpp
        include/options_parser.hpp
        include/logger.hpp
)

set_target_properties(
    ${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

target_link_libraries(
    ${PROJECT_NAME} 
        PUBLIC 
            ${Boost_LIBRARIES}
            ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/pugixml/lib/libpugixml.a
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(
        ${PROJECT_NAME} 
            PRIVATE 
                -Wall 
                -Wextra 
                -Werror 
                -pedantic
    )
endif()

# where to find our CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Installing)

# test files after deploy
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})