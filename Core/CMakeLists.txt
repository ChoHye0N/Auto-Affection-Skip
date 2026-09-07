cmake_minimum_required(VERSION 3.12)
project(Core LANGUAGES CXX)

set(PRJ_NAME "Core")
set(DLL_NAME "AasCore")

option(COPY_DLL_TO_UI "Automatically copy the output DLL to the C# project directory" ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

# CONFIGURE_DEPENDS: 새 .cpp 파일을 추가해도 매번 수동으로 재구성(configure)하지 않아도 빌드에 반영됨
file(GLOB SRC CONFIGURE_DEPENDS "*.cpp")

add_library(${PRJ_NAME} SHARED ${SRC})
set_target_properties(${PRJ_NAME} PROPERTIES OUTPUT_NAME ${DLL_NAME})

target_include_directories(${PRJ_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})

if(WIN32)
    target_compile_definitions(${PRJ_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS UNICODE _UNICODE)
endif()

target_link_libraries(${PRJ_NAME} PRIVATE ${OpenCV_LIBS})

if(MSVC)
    # 소스의 한글 주석이 BOM 유무와 무관하게 항상 UTF-8로 해석되도록 강제
    # (BOM이 없으면 시스템 코드페이지로 오인식되어 컴파일이 깨질 수 있음)
    target_compile_options(${PRJ_NAME} PRIVATE /utf-8)
endif()

if(COPY_DLL_TO_UI)
    if(NOT CMAKE_VS_PLATFORM_NAME)
        set(CMAKE_VS_PLATFORM_NAME "x64")
    endif()

    set(UI_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../UI/bin/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/net8.0-windows")

    add_custom_command(TARGET ${PRJ_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "${UI_OUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${PRJ_NAME}>"
            "${UI_OUT_DIR}/"
        COMMENT "Copying ${DLL_NAME}.dll to C# UI bin directory..."
    )

    foreach(_opencv_target ${OpenCV_LIBS})
        if(TARGET ${_opencv_target})
            get_target_property(_opencv_target_type ${_opencv_target} TYPE)
            if(_opencv_target_type STREQUAL "SHARED_LIBRARY")
                add_custom_command(TARGET ${PRJ_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "$<TARGET_FILE:${_opencv_target}>"
                        "${UI_OUT_DIR}/"
                    COMMENT "Copying ${_opencv_target} runtime DLL to C# UI bin directory..."
                )
            endif()
        endif()
    endforeach()
endif()
