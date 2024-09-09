cmake_minimum_required(VERSION 3.15)
project(learnVulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -O2")

find_package(VulkanLoader REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} glfw Vulkan::Loader)
