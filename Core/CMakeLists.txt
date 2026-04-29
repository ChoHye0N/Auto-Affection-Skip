cmake_minimum_required(VERSION 3.10)
project(Core LANGUAGES CXX)

set(PRJ_NAME "Core")
set(DLL_NAME "AasCore")

option(USE_OPENCV "Enable OpenCV features" ON)
option(COPY_DLL_TO_UI "Automatically copy the output DLL to the C# project directory" ON)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DUNICODE -D_UNICODE)
endif()

if(USE_OPENCV)
    find_package(OpenCV REQUIRED)
endif()

file(GLOB SRC "*.cpp")

add_library(${PRJ_NAME} SHARED ${SRC})
set_target_properties(${PRJ_NAME} PROPERTIES OUTPUT_NAME ${DLL_NAME})

target_include_directories(${PRJ_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(USE_OPENCV)
    target_include_directories(${PRJ_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(${PRJ_NAME} PRIVATE ${OpenCV_LIBRARIES})
endif()

if(COPY_DLL_TO_UI)
    set(UI_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../UI/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/net8.0-windows")

    add_custom_command(TARGET ${PRJ_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${PRJ_NAME}>"
            "${UI_OUT_DIR}/"
        COMMENT "Copying ${DLL_NAME}.dll to C# UI bin directory..."
    )
endif()