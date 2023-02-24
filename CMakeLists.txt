cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
endif()

add_definitions(-DUNICODE -D_UNICODE -DNOMINMAX)

project(CDX12)

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)

add_subdirectory(${CDX12_SOURCE_DIR}/_deps/DirectXTK12 ${CDX12_BINARY_DIR}/bin/CMake/DirectXTK12)
add_library(${PROJECT_NAME} ${srcs})
add_library(Chen::CDX12 ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
    PUBLIC d3dcompiler.lib
    PUBLIC DirectXTK12
)
