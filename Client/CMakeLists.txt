cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(BlaiBottClient)

file(GLOB_RECURSE sources "src/**.cpp" "src/**.h")
add_executable(BlaiBottClient ${sources})
target_include_directories(${PROJECT_NAME} PUBLIC "src")
target_include_directories(${PROJECT_NAME} PUBLIC "../vendor/SympleNet/include")
target_include_directories(${PROJECT_NAME} PUBLIC "../vendor/SympleNet/asio/include")