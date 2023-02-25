cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
endif()

add_definitions(-DUNICODE -D_UNICODE -DNOMINMAX)

option(USE_VCPKG "use vcpkg or not" OFF)

project(CDX12)

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)

add_library(${PROJECT_NAME} ${srcs})
add_library(Chen::CDX12 ALIAS ${PROJECT_NAME})

if(USE_VCPKG)
    set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "")
    set(VCPKG_TARGET_TRIPLET $ENV{VCPKG_DEFAULT_TRIPLET} CACHE STRING "")
    find_package(directxtk12 CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC Microsoft::DirectXTK12)
else()
    add_subdirectory(${CDX12_SOURCE_DIR}/_deps/DirectXTK12 ${CDX12_BINARY_DIR}/bin/CMake/DirectXTK12)
    target_link_libraries(${PROJECT_NAME} PUBLIC DirectXTK12)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
    PUBLIC d3dcompiler.lib
)
