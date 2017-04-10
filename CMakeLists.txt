cmake_minimum_required(VERSION 2.6)

project(skeleton CXX)
set(project_exec "main")
set(project_lib "libmain")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}.")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fstack-protector")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Werror -pedantic")


set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}/src")
file(GLOB_RECURSE src_files "${src_dir}/*.cpp")
set(main_src "${src_dir}/main.cpp")
list(REMOVE_ITEM src_files "${main_src}")

add_library("${project_lib}" STATIC ${src_files})
set_target_properties(${project_lib} PROPERTIES
	OUTPUT_NAME ${PROJECT_NAME})

find_library(libuv libuv.a)
message(STATUS "libuv: ${libuv}")

add_executable("${project_exec}" "${main_src}")
target_link_libraries("${project_exec}" "${project_lib}" "${libuv}" pthread)


add_custom_target(run COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${project_exec})
