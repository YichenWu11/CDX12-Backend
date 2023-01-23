cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILECOMMANDS ON)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "")
set(VCPKG_TARGET_TRIPLET $ENV{VCPKG_DEFAULT_TRIPLET} CACHE STRING "")

if(MSVC)
    add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
endif()

add_definitions(-DUNICODE -D_UNICODE -DNOMINMAX)

project(CDX12)

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)

add_library(${PROJECT_NAME} ${srcs})
add_library(Chen::CDX12 ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

find_package(directxtk12 CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
    PUBLIC d3dcompiler.lib
    PUBLIC Microsoft::DirectXTK12
)
