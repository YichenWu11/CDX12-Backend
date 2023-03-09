set_project("CDX12")
set_version("0.0.1")

set_xmakever("2.7.5")

set_languages("c++20")

add_rules("mode.debug", "mode.release")

add_defines("NOMINMAX")
add_defines("_USE_MATH_DEFINES")
add_defines("UNICODE")
add_defines("_UNICODE")

add_cxxflags("-Wno-unused-value")

add_requires("vcpkg::directxtk12")

target("Lib_CDX12")
    set_kind("static")
    add_files("src/**.cpp")
    add_headerfiles("include/**.h")
    add_includedirs("include")
    add_packages("vcpkg::directxtk12")
