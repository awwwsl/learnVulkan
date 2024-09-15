cmake_minimum_required(VERSION 3.15)

project(learnVulkan)

set(ENV{LANGUAGE} "zh_Hentai:zh_CN.UTF_8:zh_CN:C")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -O2")

set(SOURCES
    src/main.cpp
    src/Utils/VkResultThrowable.cpp

    src/Vulkan/buffer.cpp
    src/Vulkan/bufferView.cpp
    src/Vulkan/commandBuffer.cpp
    src/Vulkan/commandPool.cpp
    src/Vulkan/deviceBuffer.cpp
    src/Vulkan/deviceMemory.cpp
    src/Vulkan/fence.cpp
    src/Vulkan/framebuffer.cpp
    src/Vulkan/graphicsPipelineCreateInfoPack.cpp
    src/Vulkan/image.cpp
    src/Vulkan/imageView.cpp
    src/Vulkan/pipeline.cpp
    src/Vulkan/pipelineLayout.cpp
    src/Vulkan/renderPass.cpp
    src/Vulkan/semaphore.cpp
    src/Vulkan/shader.cpp

    src/Models/graphic.cpp
    src/Models/graphicPlus.cpp
    src/Models/graphicPlusImpl.cpp
    src/Models/window.cpp
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
