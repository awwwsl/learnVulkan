cmake_minimum_required(VERSION 3.15)

project(learnVulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -O2")

set(SOURCES
    src/main.cpp
    src/Utils/VkResultThrowable.cpp
    src/Models/window.cpp
    src/Models/graphic.cpp
    src/Models/fence.cpp
    src/Models/semaphore.cpp
    src/Models/commandBuffer.cpp
    src/Models/commandPool.cpp
    src/Models/pipelineLayout.cpp
    src/Models/pipeline.cpp
    src/Models/renderPass.cpp
    src/Models/graphicsPipelineCreateInfoPack.cpp
    src/Models/shader.cpp
    src/Models/framebuffer.cpp
)

include_directories(src)
include_directories(src/Utils)
include_directories(src/Models)

find_package(VulkanLoader REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} glfw Vulkan::Loader glm::glm)
