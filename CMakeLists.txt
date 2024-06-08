cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
project(TripleBuffering LANGUAGES CXX)

add_subdirectory(external)

set(target_name "TripleBuffering")

set(SRC_DIR "src")
add_executable(${target_name} ${SRC_DIR}/TripleBufferIndex.hpp main.cpp)
target_include_directories(${target_name} PUBLIC ${SRC_DIR})

set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TripleBuffering)
    set_property(TARGET ${target_name} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${target_name}>)
endif()

set_target_properties(${target_name} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
target_link_libraries(${target_name} PUBLIC ext::relacy)
