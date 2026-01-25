cmake_minimum_required(VERSION 3.18)
project(CHIP8Emulator)
set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/showIncludes-)
endif()

set(SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/SFML-3.0.2/lib/cmake/SFML")
find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)

add_executable(CHIP8Emulator main.cpp
        Chip_emulator.cpp
        Chip_emulator.h)

target_link_libraries(CHIP8Emulator PRIVATE SFML::Graphics SFML::Window SFML::System)

add_custom_command(TARGET CHIP8Emulator POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/SFML-3.0.2/bin"
        $<TARGET_FILE_DIR:CHIP8Emulator>
)