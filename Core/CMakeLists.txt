# =============================================================================
# AasCore.dll — 화면 캡처 + OpenCV 이미지 인식 + 입력 (C# UI가 P/Invoke로 호출)
# =============================================================================
cmake_minimum_required(VERSION 3.21)

# -----------------------------------------------------------------------------
# 옵션 (project() 이전에 선언해야 vcpkg 트리플렛 선택에 반영됨)
# -----------------------------------------------------------------------------
option(COPY_DLL_TO_UI "빌드 후 DLL을 C# UI 출력 폴더로 복사" ON)
option(AAS_STATIC_RUNTIME
       "OpenCV와 MSVC 런타임을 정적 링크해 AasCore.dll 하나로 배포 (x64-windows-static 트리플렛 사용)"
       OFF)
option(AAS_WARNINGS_AS_ERRORS "컴파일러 경고를 오류로 취급 (CI용)" OFF)

# 정적 링크 모드면 vcpkg가 정적 라이브러리를 설치하도록 트리플렛 지정
if(AAS_STATIC_RUNTIME AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "vcpkg 트리플렛")
endif()

project(AutoAffectionSkipCore LANGUAGES CXX)

set(PRJ_NAME "Core")
set(DLL_NAME "AasCore")

# C# UI 출력 폴더 구성 요소 (UI.csproj의 Platform / TargetFramework와 맞춰야 함)
set(AAS_UI_PLATFORM "x64" CACHE STRING "C# UI 빌드 플랫폼 이름")
set(AAS_UI_TARGET_FRAMEWORK "net8.0-windows" CACHE STRING "C# UI TargetFramework")

# 정적 링크 모드: MSVC 런타임도 정적(/MT, /MTd)으로 → VC++ 재배포 패키지 불필요
if(AAS_STATIC_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# -----------------------------------------------------------------------------
# 의존성
# -----------------------------------------------------------------------------
find_package(OpenCV CONFIG REQUIRED)

# -----------------------------------------------------------------------------
# 타깃
# -----------------------------------------------------------------------------
# 소스는 명시적으로 나열 (새 파일을 추가하면 여기에도 추가)
# pch.cpp는 target_precompile_headers가 대신하므로 목록에서 제외
add_library(${PRJ_NAME} SHARED
    dllmain.cpp
    imageProcessing.cpp
    macro.cpp
    framework.h
    imageProcessing.h
    macro.h
    pch.h)

set_target_properties(${PRJ_NAME} PROPERTIES
    OUTPUT_NAME ${DLL_NAME}
    CXX_EXTENSIONS OFF)

target_compile_features(${PRJ_NAME} PRIVATE cxx_std_17)

target_include_directories(${PRJ_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# OpenCV 타깃을 링크하면 include 경로와 컴파일 옵션이 자동으로 따라옴
target_link_libraries(${PRJ_NAME} PRIVATE ${OpenCV_LIBS})

# 미리 컴파일된 헤더 (기존 pch.h 재사용, 빌드 속도 향상)
target_precompile_headers(${PRJ_NAME} PRIVATE pch.h)

if(WIN32)
    target_compile_definitions(${PRJ_NAME} PRIVATE
        UNICODE _UNICODE
        _CRT_SECURE_NO_WARNINGS)
endif()

if(MSVC)
    target_compile_options(${PRJ_NAME} PRIVATE
        /utf-8          # 소스의 한글 주석을 BOM 유무와 무관하게 항상 UTF-8로 해석
        /W4             # 높은 경고 수준
        /permissive-)   # 표준 준수 모드
    if(AAS_WARNINGS_AS_ERRORS)
        target_compile_options(${PRJ_NAME} PRIVATE /WX)
    endif()
endif()

# -----------------------------------------------------------------------------
# 빌드 후 C# UI 출력 폴더로 복사
# -----------------------------------------------------------------------------
if(COPY_DLL_TO_UI)
    set(UI_OUT_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/../UI/bin/${AAS_UI_PLATFORM}/$<CONFIG>/${AAS_UI_TARGET_FRAMEWORK}")

    add_custom_command(TARGET ${PRJ_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${UI_OUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${PRJ_NAME}>" "${UI_OUT_DIR}/"
        COMMENT "Copying ${DLL_NAME}.dll to C# UI bin directory...")

    # 동적 링크 모드에서만 OpenCV 런타임 DLL 복사 (정적 모드는 DLL 하나로 끝)
    # TARGET_RUNTIME_DLLS: 이 타깃이 링크한 공유 라이브러리 타깃의 DLL 목록 (CMake 3.21+)
    if(NOT AAS_STATIC_RUNTIME)
        add_custom_command(TARGET ${PRJ_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_RUNTIME_DLLS:${PRJ_NAME}>" "${UI_OUT_DIR}/"
            COMMAND_EXPAND_LISTS
            COMMENT "Copying OpenCV runtime DLLs to C# UI bin directory...")
    endif()
endif()
