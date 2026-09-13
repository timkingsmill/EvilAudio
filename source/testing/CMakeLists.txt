
include_guard(GLOBAL)

cmake_minimum_required(VERSION 4.2.0)

project(EvilAudioTestDriver VERSION 0.1.0 LANGUAGES C CXX)

evil_audio_setup_google_test()
evil_audio_find_test_sources(TEST_SOURCES)

# --------------------------------------------------------------------------------

add_executable(${PROJECT_NAME}
    tests/libs/evil_patterns_lib/mocks/evil_patterns_lib_mocks.h
    tests/libs/evil_patterns_lib/mocks/evil_patterns_lib_mocks.cpp
    source/EvilAudioTesting.cpp
    ${TEST_SOURCES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Testing/Drivers")

target_include_directories(${PROJECT_NAME}      
    PRIVATE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tests/libs/evil_patterns_lib>
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        GTest::gtest_main
        GTest::gtest

    PRIVATE
        evil::EvilDAWLib
        evil::evil_patterns_lib
)


# --------------------------------------------------------------------------------

# Organize source files into groups in Visual Studio.
get_target_property(target_sources ${PROJECT_NAME} SOURCES)

# Create the Source files group.
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Source Files"
    FILES 
        ${target_sources}
)

# --------------------------------------------------------------------------------

include(GoogleTest)

gtest_discover_tests(
    ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# --------------------------------------------------------------------------------
