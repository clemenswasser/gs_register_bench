cmake_minimum_required(VERSION 3.25)

include(FetchContent)
FetchContent_Declare(vcpkg
	URL https://github.com/microsoft/vcpkg/archive/refs/tags/2023.04.15.tar.gz
	CONFIGURE_COMMAND "")
FetchContent_MakeAvailable(vcpkg)
set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake")

project(gs_register_bench LANGUAGES CXX)

add_executable(gs_register_bench main.cpp)
find_package(benchmark CONFIG REQUIRED)
target_link_libraries(gs_register_bench PRIVATE benchmark::benchmark benchmark::benchmark_main)
target_compile_features(gs_register_bench PRIVATE cxx_std_20)

if (MSVC)
	target_compile_options(gs_register_bench PRIVATE "/W4" "/WX")
else()
	target_compile_options(gs_register_bench PRIVATE "-mfsgsbase" "-Werror" "-Wall" "-Wextra" "-Wpedantic")
endif()
