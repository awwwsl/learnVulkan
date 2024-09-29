cmake_minimum_required(VERSION 3.15)

project(learnVulkan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wno-unused")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native -Wall -Wno-unused")

set(SOURCES
    src/main.cpp

    src/Utils/imageOperation.cpp

    src/Vulkan/attachment.cpp
    src/Vulkan/buffer.cpp
    src/Vulkan/bufferMemory.cpp
    src/Vulkan/bufferView.cpp
    src/Vulkan/colorAttachment.cpp
    src/Vulkan/commandBuffer.cpp
    src/Vulkan/commandPool.cpp
    src/Vulkan/depthStencilAttachment.cpp
    src/Vulkan/descriptorPool.cpp
    src/Vulkan/descriptorSet.cpp
    src/Vulkan/descriptorSetLayout.cpp
    src/Vulkan/deviceLocalBuffer.cpp
    src/Vulkan/deviceMemory.cpp
    src/Vulkan/dynamicTexture.cpp
    src/Vulkan/dynamicTexture2d.cpp
    src/Vulkan/fence.cpp
    src/Vulkan/formatInfo.cpp
    src/Vulkan/framebuffer.cpp
    src/Vulkan/gameTexture.cpp
    src/Vulkan/graphicsPipelineCreateInfoPack.cpp
    src/Vulkan/image.cpp
    src/Vulkan/imageMemory.cpp
    src/Vulkan/imageView.cpp
    src/Vulkan/pipeline.cpp
    src/Vulkan/pipelineLayout.cpp
    src/Vulkan/renderPass.cpp
    src/Vulkan/sampler.cpp
    src/Vulkan/semaphore.cpp
    src/Vulkan/shader.cpp
    src/Vulkan/stagingBuffer.cpp
    src/Vulkan/texture.cpp
    src/Vulkan/texture2d.cpp

    src/Models/chunk.cpp
    src/Models/graphic.cpp
    src/Models/graphicPlus.cpp
    src/Models/graphicPlusImpl.cpp
    src/Models/rpwfUtils.cpp
    src/Models/textureManager.cpp
    src/Models/window.cpp
    src/Models/world.cpp
)

include_directories(src)
include_directories(src/Utils)
include_directories(src/Models)

find_package(VulkanLoader REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(stb REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} glfw Vulkan::Loader glm::glm stb::stb)
