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

    src/Models/buffer.cpp
    src/Models/bufferView.cpp
    src/Models/commandBuffer.cpp
    src/Models/commandPool.cpp
    src/Models/deviceBuffer.cpp
    src/Models/deviceMemory.cpp
    src/Models/fence.cpp
    src/Models/framebuffer.cpp
    src/Models/graphic.cpp
    src/Models/graphicPlus.cpp
    src/Models/graphicPlusImpl.cpp
    src/Models/graphicsPipelineCreateInfoPack.cpp
    src/Models/image.cpp
    src/Models/imageView.cpp
    src/Models/pipeline.cpp
    src/Models/pipelineLayout.cpp
    src/Models/renderPass.cpp
    src/Models/semaphore.cpp
    src/Models/shader.cpp
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
