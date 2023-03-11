set_project("CDX12")
set_version("0.0.1")
set_xmakever("2.7.5")
set_languages("c++20")
set_policy("check.auto_ignore_flags", false)

add_rules("mode.release", "mode.debug")

option("is_clang")
add_csnippets("is_clang", "return (__clang__)?0:-1;", {
    tryrun = true
})
option_end()

option("is_msvc")
add_csnippets("is_msvc", "return (_MSC_VER)?0:-1;", {
    tryrun = true
})
option_end()

option("is_gnu")
add_csnippets("is_gnu", "return (__GNUC__)?0:-1;", {
    tryrun = true
})
option_end()

add_defines("NOMINMAX")
add_defines("_USE_MATH_DEFINES")
add_defines("UNICODE")
add_defines("_UNICODE")

add_cxxflags("-Wno-unused-value", "-Wno-unused-function", "-Wno-unused-private-field")
set_warnings("all")

if is_mode("debug") then
	set_targetdir("bin/debug")
    add_defines("DEBUG")
    set_optimize("none")
else
	set_targetdir("bin/release")
    set_optimize("fastest")
end

add_requires("vcpkg::directxtk12")

includes("core")
includes("test")
