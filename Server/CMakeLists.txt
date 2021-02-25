cmake_minimum_required(VERSION 1.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(BlaiBottServer)

file(GLOB_RECURSE sources "src/**.cpp" "src/**.h")
add_executable(BlaiBottServer ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC "src")
target_include_directories(${PROJECT_NAME} PUBLIC "vendor/SympleNet/include")