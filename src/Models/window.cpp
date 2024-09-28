#define GLFW_INCLUDE_VULKAN

#define LIMIT_FRAME_RATE true

#define BOOT_SCREEN true

#include "../Utils/VkResultThrowable.hpp"
#include "../Utils/color.hpp"

#include "../Vulkan/vulkanWrapper.hpp"

#include "block.hpp"
#include "camera.hpp"
#include "graphic.hpp"
#include "graphicPlus.hpp"
#include "instance.hpp"
#include "rpwfUtils.hpp"
#include "window.hpp"
#include "world.hpp"

#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace learnVulkan {

enum class Face {
  UNDEFINED = 0,
  FRONT = 1,
  BACK = 2,
  LEFT = 3,
  RIGHT = 4,
  TOP = 5,
  BOTTOM = 6,
  EMERGED = INT32_MAX,
};

struct vertex {
  glm::vec3 position;
  glm::vec2 texCoord;
};

struct alignas(16) MVP {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
};

struct pushConstant_outline {
  glm::mat4 aimingBlockModel;
  glm::vec4 colorRGBA;
};

struct pushConstant_cursor {
  VkExtent2D imageExtent;
  VkExtent2D maskExtentMultiplied;
  uint32_t cursorScale;
};

struct pushConstant_skybox {
  uint32_t facing;
};

int32_t facing = int32_t(Face::UNDEFINED);
block *aimingEntity = nullptr;

uint64_t holdingItem = 0;

window::window() {}

bool window::initialize() {
  // glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

  printf("[ GLFW ] INFO: GLFW version: %s\n", glfwGetVersionString());

  if (!glfwInit()) {
    printf("[ window ] FATAL: Failed to initialize GLFW\n");
    return false;
  }

  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  glfwMonitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *glfwMonitorMode = glfwGetVideoMode(glfwMonitor);
  glfwWindow = glfwCreateWindow(defaultSize.width, defaultSize.height,
                                windowTitle.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(glfwWindow, this);
  glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwGetWindowPos(glfwWindow, &currentPosition.x, &currentPosition.y);
  currentSize = defaultSize;
#ifdef _WIN32
  graphicsBase::Base().AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
  graphicsBase::Base().AddInstanceExtension(
      VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
  uint32_t extensionCount = 0;
  const char **extensionNames;
  extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);

  if (!extensionNames) {
    printf("[ window ] FATAL: Vulkan is not available on this machine!\n");
    glfwTerminate();
    return false;
  }

  for (size_t i = 0; i < extensionCount; i++) {
    graphic::Singleton().AddInstanceExtension(extensionNames[i]);
  }
#endif
  graphic::Singleton().AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  graphic::Singleton().UseLatestApiVersion();
  if (graphic::Singleton().CreateInstance()) {
    printf("[ window ] ERROR: Failed to create Vulkan instance\n");
    return false;
  }

  VkSurfaceKHR surface = VK_NULL_HANDLE;
  if (VkResultThrowable result = glfwCreateWindowSurface(
          graphic::Singleton().Instance(), glfwWindow, nullptr, &surface)) {
    printf("[ window ] ERROR: Failed to create a "
           "window surface!\nError code: %d\n",
           int32_t(result));
    glfwTerminate();
    return false;
  }
  graphic::Singleton().Surface(surface);

  if (!graphic::Singleton().GetPhysicalDevices()) {
    for (int i = 0; i < graphic::Singleton().AvailablePhysicalDeviceCount();
         i++) {
      auto physicalDevice = graphic::Singleton().AvailablePhysicalDevice(i);
      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(physicalDevice, &properties);
      printf("[ window ] HINT: Device %d: %s\n", i, properties.deviceName);
    }
  }

  graphic::Plus();
  while (true) {
    printf("[ window ] HINT: Please select a device: ");
    int deviceIndex = 0;
    scanf("%d", &deviceIndex);

    if (deviceIndex < 0 ||
        deviceIndex >= graphic::Singleton().AvailablePhysicalDeviceCount()) {
      printf("[ window ] ERROR: Invalid device index\n");
      continue;
    }
    if (graphic::Singleton().DeterminePhysicalDevice(deviceIndex, true, true)) {
      printf(
          "[ window ] ERROR: Device not qualified for vulkan graphics queue\n");
      continue;
    }
    if (graphic::Singleton().CreateDevice()) {
      printf("[ window ] ERROR: Failed to create logical device\n");
      continue;
    }

    break;
  }

  // if (graphicsBase::Singleton().GetPhysicalDevices() ||
  //     graphicsBase::Singleton().DeterminePhysicalDevice(0, true, false) ||
  //     graphicsBase::Singleton().CreateDevice()) {
  //   return false;
  // }
  //

  if (graphic::Singleton().CreateSwapchain(LIMIT_FRAME_RATE))
    return false;

  return true;
}

block *window::rayIntersection(const glm::vec3 start, const glm::vec3 direction,
                               const float maxDistance, int *facing) {
  glm::vec3 now = start;
  glm::ivec3 current(glm::round(now));
  if (auto *entity = worldInstance.getEntity(current)) {
    *facing = int(Face::EMERGED);
    return entity;
  }
  while (glm::distance(now, start) < maxDistance) {
    now += direction * 0.01f;
    glm::ivec3 current(glm::round(now));
    if (auto *entity = worldInstance.getEntity(current)) {
      if (facing) {
        Face face;
        glm::vec3 diff = now - glm::vec3(current);
        if (glm::abs(diff.x) > glm::abs(diff.y) &&
            glm::abs(diff.x) > glm::abs(diff.z)) {
          face = (direction.x > 0) ? Face::LEFT : Face::RIGHT;
        } else if (glm::abs(diff.y) > glm::abs(diff.x) &&
                   glm::abs(diff.y) > glm::abs(diff.z)) {
          face = (direction.y > 0) ? Face::TOP : Face::BOTTOM;
        } else {
          face = (direction.z > 0) ? Face::BACK : Face::FRONT;
        }
        *facing = int(face);
      }
      return entity;
    }
  }
  return nullptr;
}

void BootScreen(const char *imagePath, VkFormat imageFormat, bool *pLoading) {
  if (!BOOT_SCREEN)
    return;
  VkExtent2D imageExtent;
  std::unique_ptr<uint8_t[]> pImageData = imageOperation::LoadFile_FileSystem(
      imagePath, imageExtent, formatInfo::FormatInfo(imageFormat));
  if (!pImageData)
    return;
  vulkanWrapper::stagingBuffer::BufferData_CurrentThread(
      pImageData.get(), formatInfo::FormatInfo(imageFormat).sizePerPixel *
                            imageExtent.width * imageExtent.height);

  vulkanWrapper::semaphore semaphore_imageIsAvailable;
  vulkanWrapper::fence fence;
  vulkanWrapper::commandBuffer commandBuffer;
  vulkanWrapper::commandPool commandPool(
      graphic::Singleton().QueueFamilyIndex_Graphics(),
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  commandPool.AllocateBuffers(commandBuffer);

  auto target = glfwGetTime() + 3;
  VkImage image;
  while (glfwGetTime() < target || *pLoading) {
    graphic::Singleton().SwapImage(semaphore_imageIsAvailable);
    commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VkExtent2D swapchainImageSize =
        graphic::Singleton().SwapchainCreateInfo().imageExtent;
    bool blit =
        imageExtent.width != swapchainImageSize.width ||
        imageExtent.height != swapchainImageSize.height ||
        imageFormat != graphic::Singleton().SwapchainCreateInfo().imageFormat;
    vulkanWrapper::imageMemory imageMemory;
    if (blit) { // image needs blit
      image = vulkanWrapper::stagingBuffer::AliasedImage2d_CurrentThread(
          imageFormat, imageExtent);
      if (image) { // image exists
        VkImageMemoryBarrier imageMemoryBarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            nullptr,
            0,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_PREINITIALIZED,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            image,
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                             nullptr, 1, &imageMemoryBarrier);
      } else { // image not exists
        VkImageCreateInfo imageCreateInfo = {
            .imageType = VK_IMAGE_TYPE_2D,
            .format = imageFormat,
            .extent = {imageExtent.width, imageExtent.height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT};
        imageMemory.Create(imageCreateInfo,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VkBufferImageCopy region_copy = {
            .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
            .imageExtent = imageCreateInfo.extent};
        imageOperation::CmdCopyBufferToImage(
            commandBuffer, vulkanWrapper::stagingBuffer::Buffer_CurrentThread(),
            imageMemory.Image(), region_copy,
            {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED},
            {VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT,
             VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL});

        image = imageMemory.Image();
      } // blit image
      float srcAspectRatio =
          float(imageExtent.width) / float(imageExtent.height);
      float dstAspectRatio =
          float(swapchainImageSize.width) / float(swapchainImageSize.height);

      VkImageBlit region_blit = {
          .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
          .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
      };

      if (srcAspectRatio < dstAspectRatio) { // 添加左右黑边
        int32_t newImageWidth =
            int32_t(swapchainImageSize.height * srcAspectRatio);
        int32_t offsetX = (swapchainImageSize.width - newImageWidth) / 2;

        region_blit.srcOffsets[0] = {0, 0, 0};
        region_blit.srcOffsets[1] = {(int32_t)imageExtent.width,
                                     (int32_t)imageExtent.height, 1};

        region_blit.dstOffsets[0] = {offsetX, 0, 0};
        region_blit.dstOffsets[1] = {offsetX + newImageWidth,
                                     (int32_t)swapchainImageSize.height, 1};
      } else { // 添加上下黑边
        int32_t newImageHeight =
            int32_t(swapchainImageSize.width / srcAspectRatio);
        int32_t offsetY = (swapchainImageSize.height - newImageHeight) / 2;

        region_blit.srcOffsets[0] = {0, 0, 0};
        region_blit.srcOffsets[1] = {(int32_t)imageExtent.width,
                                     (int32_t)imageExtent.height, 1};

        region_blit.dstOffsets[0] = {0, offsetY, 0};
        region_blit.dstOffsets[1] = {(int32_t)swapchainImageSize.width,
                                     offsetY + newImageHeight, 1};
      }

      imageOperation::CmdBlitImage(
          commandBuffer, image,
          graphic::Singleton().SwapchainImage(
              graphic::Singleton().CurrentImageIndex()),
          region_blit,
          {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED},
          {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
           VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
          VK_FILTER_LINEAR);
    } else { // image needs no blit
      VkBufferImageCopy region_copy = {
          .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
          .imageExtent = {imageExtent.width, imageExtent.height, 1}};
      imageOperation::CmdCopyBufferToImage(
          commandBuffer, vulkanWrapper::stagingBuffer::Buffer_CurrentThread(),
          graphic::Singleton().SwapchainImage(
              graphic::Singleton().CurrentImageIndex()),

          region_copy,
          {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED},
          {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
           VK_IMAGE_LAYOUT_PRESENT_SRC_KHR});
    }
    commandBuffer.End();

    VkPipelineStageFlags waitDstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submitInfo = {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = semaphore_imageIsAvailable.Address(),
        .pWaitDstStageMask = &waitDstStage,
        .commandBufferCount = 1,
        .pCommandBuffers = commandBuffer.Address(),
    };
    graphic::Singleton().SubmitCommandBuffer_Graphics(submitInfo, fence);
    fence.WaitAndReset();
    graphic::Singleton().PresentImage();
  }

  std::vector<vulkanWrapper::commandBuffer *> buffers = {&commandBuffer};
  commandPool.FreeBuffers(buffers);

  printf("[ window ] INFO: BootScreen finished\n");
}

const rpwfUtils::renderPassWithFramebuffers &RenderPassAndFramebuffers() {
  static std::vector<vulkanWrapper::depthStencilAttachment> dsas_screenWithDS;
  static const rpwfUtils::renderPassWithFramebuffers &rpwf =
      rpwfUtils::CreateRenderPassWithFramebuffers(VK_FORMAT_D24_UNORM_S8_UINT,
                                                  dsas_screenWithDS);
  return rpwf;
}

const void CreatePipelineCube(vulkanWrapper::pipeline &pipeline,
                              vulkanWrapper::pipelineLayout &layout) {
  static vulkanWrapper::shader vert("src/Shaders/3d.vert.spv");
  static vulkanWrapper::shader frag("src/Shaders/3d.frag.spv");
  static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_3d[2] = {
      vert.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
      frag.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};
  auto Create = [&] {
    const VkExtent2D &windowSize =
        graphic::Singleton().SwapchainCreateInfo().imageExtent;
    graphicsPipelineCreateInfoPack pipelineCiPack;

    pipelineCiPack.vertexInputBindings.emplace_back(
        0, sizeof(vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    pipelineCiPack.vertexInputAttributes.emplace_back(
        0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, position));
    pipelineCiPack.vertexInputAttributes.emplace_back(
        1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, texCoord));

    pipelineCiPack.createInfo.layout = layout;
    pipelineCiPack.createInfo.renderPass =
        RenderPassAndFramebuffers().renderPass;
    pipelineCiPack.inputAssemblyStateCi.topology =
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineCiPack.viewports.emplace_back(0.f, 0.f, float(windowSize.width),
                                          float(windowSize.height), 0.f, 1.f);
    pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);

    // 开启背面剔除
    pipelineCiPack.rasterizationStateCi.cullMode = VK_CULL_MODE_BACK_BIT;
    pipelineCiPack.rasterizationStateCi.frontFace =
        VK_FRONT_FACE_CLOCKWISE; // 默认值，为0

    pipelineCiPack.multisampleStateCi.rasterizationSamples =
        VK_SAMPLE_COUNT_1_BIT;

    // 开启深度测试
    pipelineCiPack.depthStencilStateCi.depthTestEnable = VK_TRUE;
    pipelineCiPack.depthStencilStateCi.depthWriteEnable = VK_TRUE;
    pipelineCiPack.depthStencilStateCi.depthCompareOp =
        VK_COMPARE_OP_LESS; // 若新片元的深度更小，则通过测试

    pipelineCiPack.colorBlendAttachmentStates.push_back(
        {.colorWriteMask = 0b1111});
    pipelineCiPack.UpdateAllArrays();
    pipelineCiPack.createInfo.stageCount = 2;
    pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_3d;

    pipeline.Create(pipelineCiPack);
  };
  auto Destroy = [&] { pipeline.~pipeline(); };
  graphic::Singleton().AddCreateSwapchainCallback(Create);
  graphic::Singleton().AddDestroySwapchainCallback(Destroy);
  Create();
}

const void CreatePipelineOutline(vulkanWrapper::pipeline &pipeline,
                                 vulkanWrapper::pipelineLayout &layout) {
  static vulkanWrapper::shader vert("src/Shaders/outline.vert.spv");
  static vulkanWrapper::shader frag("src/Shaders/outline.frag.spv");
  static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_3d[] = {
      vert.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
      frag.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};
  auto Create = [&] {
    const VkExtent2D &windowSize =
        graphic::Singleton().SwapchainCreateInfo().imageExtent;
    graphicsPipelineCreateInfoPack pipelineCiPack;

    pipelineCiPack.vertexInputBindings.emplace_back(
        0, sizeof(vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    pipelineCiPack.vertexInputAttributes.emplace_back(
        0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, position));

    pipelineCiPack.createInfo.layout = layout;
    pipelineCiPack.createInfo.renderPass =
        RenderPassAndFramebuffers().renderPass;

    pipelineCiPack.inputAssemblyStateCi.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineCiPack.inputAssemblyStateCi.topology =
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    pipelineCiPack.inputAssemblyStateCi.primitiveRestartEnable = VK_FALSE;

    pipelineCiPack.viewports.emplace_back(0.f, 0.f, float(windowSize.width),
                                          float(windowSize.height), 0.f, 1.f);
    pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);

    pipelineCiPack.rasterizationStateCi.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineCiPack.rasterizationStateCi.polygonMode = VK_POLYGON_MODE_LINE;
    pipelineCiPack.rasterizationStateCi.lineWidth = 6.f;

    // 开启背面剔除
    pipelineCiPack.rasterizationStateCi.cullMode = VK_CULL_MODE_BACK_BIT;
    pipelineCiPack.rasterizationStateCi.frontFace =
        VK_FRONT_FACE_CLOCKWISE; // 默认值，为0

    pipelineCiPack.multisampleStateCi.rasterizationSamples =
        VK_SAMPLE_COUNT_1_BIT;

    // 开启深度测试
    pipelineCiPack.depthStencilStateCi.depthTestEnable = VK_TRUE;
    pipelineCiPack.depthStencilStateCi.depthWriteEnable = VK_TRUE;
    pipelineCiPack.depthStencilStateCi.depthCompareOp =
        VK_COMPARE_OP_LESS; // 若新片元的深度更小，则通过测试

    // 开启深度偏移  TODO: 调整到合适的值
    pipelineCiPack.rasterizationStateCi.depthBiasEnable = VK_TRUE;
    pipelineCiPack.rasterizationStateCi.depthBiasConstantFactor = -0.5f;
    pipelineCiPack.rasterizationStateCi.depthBiasSlopeFactor = 0.f;

    pipelineCiPack.colorBlendAttachmentStates.push_back(
        {.colorWriteMask = 0b1111});
    pipelineCiPack.UpdateAllArrays();
    pipelineCiPack.createInfo.stageCount = 2;
    pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_3d;

    pipeline.Create(pipelineCiPack);
  };

  auto Destroy = [&] { pipeline.~pipeline(); };
  graphic::Singleton().AddCreateSwapchainCallback(Create);
  graphic::Singleton().AddDestroySwapchainCallback(Destroy);
  Create();
}

const void CreatePipelineSkybox(vulkanWrapper::pipeline &pipeline,
                                vulkanWrapper::pipelineLayout &layout) {
  static vulkanWrapper::shader vert("src/Shaders/skybox.vert.spv");
  static vulkanWrapper::shader frag("src/Shaders/skybox.frag.spv");
  static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_3d[] = {
      vert.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
      frag.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT),
  };
  auto Create = [&] {
    const VkExtent2D &windowSize =
        graphic::Singleton().SwapchainCreateInfo().imageExtent;
    graphicsPipelineCreateInfoPack pipelineCiPack;

    pipelineCiPack.vertexInputBindings.emplace_back(
        0, sizeof(vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    pipelineCiPack.vertexInputAttributes.emplace_back(
        0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, position));
    pipelineCiPack.vertexInputAttributes.emplace_back(
        1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, texCoord));

    pipelineCiPack.createInfo.layout = layout;
    pipelineCiPack.createInfo.renderPass =
        RenderPassAndFramebuffers().renderPass;

    pipelineCiPack.inputAssemblyStateCi.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineCiPack.inputAssemblyStateCi.topology =
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineCiPack.inputAssemblyStateCi.primitiveRestartEnable = VK_FALSE;

    pipelineCiPack.viewports.emplace_back(0.f, 0.f, float(windowSize.width),
                                          float(windowSize.height), 0.f, 1.f);
    pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);

    pipelineCiPack.rasterizationStateCi.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineCiPack.rasterizationStateCi.polygonMode = VK_POLYGON_MODE_FILL;

    pipelineCiPack.rasterizationStateCi.cullMode = VK_CULL_MODE_FRONT_BIT;
    pipelineCiPack.rasterizationStateCi.frontFace =
        VK_FRONT_FACE_CLOCKWISE; // 默认值，为0

    pipelineCiPack.multisampleStateCi.rasterizationSamples =
        VK_SAMPLE_COUNT_1_BIT;

    pipelineCiPack.depthStencilStateCi.depthTestEnable = VK_TRUE;
    pipelineCiPack.depthStencilStateCi.depthWriteEnable = VK_FALSE;
    pipelineCiPack.depthStencilStateCi.depthCompareOp =
        VK_COMPARE_OP_LESS_OR_EQUAL;

    pipelineCiPack.colorBlendAttachmentStates.push_back(
        {.colorWriteMask = 0b1111});
    pipelineCiPack.UpdateAllArrays();
    pipelineCiPack.createInfo.stageCount = 2;
    pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_3d;

    pipeline.Create(pipelineCiPack);
  };

  auto Destroy = [&] { pipeline.~pipeline(); };
  graphic::Singleton().AddCreateSwapchainCallback(Create);
  graphic::Singleton().AddDestroySwapchainCallback(Destroy);
  Create();
}

const void CreatePipelineCursor(vulkanWrapper::pipeline &pipeline,
                                vulkanWrapper::pipelineLayout &layout) {
  static vulkanWrapper::shader comp("src/Shaders/cursor.comp.spv");
  static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_3d[] = {
      comp.StageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT),
  };
  auto Create = [&] {
    const VkExtent2D &windowSize =
        graphic::Singleton().SwapchainCreateInfo().imageExtent;
    computePipelineCreateInfoPack pipelineCiPack;

    pipelineCiPack.createInfo.layout = layout;
    pipelineCiPack.createInfo.stage = shaderStageCreateInfos_3d[0];

    pipelineCiPack.UpdateAllArrays();

    pipeline.Create(pipelineCiPack);
  };

  auto Destroy = [&] { pipeline.~pipeline(); };
  graphic::Singleton().AddCreateSwapchainCallback(Create);
  graphic::Singleton().AddDestroySwapchainCallback(Destroy);
  Create();
}

void CreateSampler(vulkanWrapper::sampler &sampler) {
  VkSamplerCreateInfo info = imageOperation::DefaultSamplerCreateInfo();
  info.minFilter = VK_FILTER_NEAREST;
  info.magFilter = VK_FILTER_NEAREST;
  sampler.Create(info);
}

void window::run() {
  bool loading = true;
  bool *pLoading = &loading;
  std::thread BootScreenThread(BootScreen,
                               "res/vulkanCraft/texture/bootScreen.png",
                               VK_FORMAT_R8G8B8A8_UNORM, pLoading);
#ifndef NDEBUG
  auto id = BootScreenThread.get_id();
  std::cout << "[ window ] DEBUG: BootScreenThread id: " << id << std::endl;
#endif

  const static auto registerLogicUpdateCallback = [this]() {
    updatePerPeriod(std::chrono::seconds(1), [this](int dframe, double dt) {
      std::stringstream info;
      info.precision(1);
      info << windowTitle << "    " << std::fixed << dframe / dt << " FPS";
      glfwSetWindowTitle(glfwWindow, info.str().c_str());
    });

    static const constexpr int callbackInterval = 5;
    updatePerPeriod(
        std::chrono::milliseconds(callbackInterval), [this](int, double) {
          bool windowSpeeding =
              glfwGetKey(glfwWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
          bool movementSpeeding =
              glfwGetKey(glfwWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

          if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
            glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_RELEASE) {
            glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
            currentPosition.y +=
                10 * ((windowSpeeding) ? 5 : 1) * callbackInterval / 20.f;
            // printf("Down\n");
            MakeWindowWindowed(currentPosition, currentSize);
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_UP) == GLFW_PRESS) {
            currentPosition.y -=
                10 * ((windowSpeeding) ? 5 : 1) * callbackInterval / 20.f;
            // printf("Up\n");
            MakeWindowWindowed(currentPosition, currentSize);
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
            currentPosition.x -=
                10 * ((windowSpeeding) ? 5 : 1) * callbackInterval / 20.f;
            // printf("Left\n");
            MakeWindowWindowed(currentPosition, currentSize);
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            currentPosition.x +=
                10 * ((windowSpeeding) ? 5 : 1) * callbackInterval / 20.f;
            // printf("Right\n");
            MakeWindowWindowed(currentPosition, currentSize);
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS) {
            camera::Singleton().horizentalForward(
                .05f * (movementSpeeding ? 5 : 1) * callbackInterval / 20.f);
            // printf("W\n");
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS) {
            camera::Singleton().horizentalForward(
                -.05f * (movementSpeeding ? 5 : 1) * callbackInterval / 20.f);
            // printf("S\n");
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS) {
            camera::Singleton().horizentalRightward(
                -.05f * (movementSpeeding ? 5 : 1) * callbackInterval / 20.f);
            // printf("A\n");
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS) {
            camera::Singleton().horizentalRightward(
                .05f * (movementSpeeding ? 5 : 1) * callbackInterval / 20.f);
            // printf("D\n");
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_SPACE) == GLFW_PRESS) {
            camera::Singleton().verticalUpward(
                .05f * (movementSpeeding ? 5 : 1) * callbackInterval / 20.f);
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            camera::Singleton().verticalUpward(
                -.05f * (movementSpeeding ? 5 : 1) * callbackInterval / 20.f);
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_1) == GLFW_PRESS) {
            holdingItem = 0;
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_2) == GLFW_PRESS) {
            holdingItem = 1;
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_3) == GLFW_PRESS) {
            holdingItem = 2;
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_4) == GLFW_PRESS) {
            holdingItem = 3;
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_5) == GLFW_PRESS) {
            holdingItem = 4;
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_6) == GLFW_PRESS) {
            holdingItem = 5;
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_7) == GLFW_PRESS) {
            holdingItem = 6;
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_8) == GLFW_PRESS) {
            holdingItem = 7;
          }
          if (glfwGetKey(glfwWindow, GLFW_KEY_9) == GLFW_PRESS) {
            holdingItem = 8;
          }
          { // fov switch
            static bool pressed = false;
            static float fov = camera::Singleton().fov;
            if (glfwGetKey(glfwWindow, GLFW_KEY_C) == GLFW_PRESS &&
                pressed == false) {
              pressed = true;
              fov = camera::Singleton().fov;
              camera::Singleton().fov = 30.f;
            } else if (glfwGetKey(glfwWindow, GLFW_KEY_C) == GLFW_RELEASE &&
                       pressed == true) {
              pressed = false;
              camera::Singleton().fov = fov;
            }
          }
          {
            static bool pressed = false;
            if (glfwGetKey(glfwWindow, GLFW_KEY_P) == GLFW_PRESS &&
                pressed == false) {
              pressed = true;
              printf("[ window ] INFO: camera(%f, %f, %f) front:(%f, %f, %f)\n",
                     camera::Singleton().position.x,
                     camera::Singleton().position.y,
                     camera::Singleton().position.z,
                     camera::Singleton().front.x, camera::Singleton().front.y,
                     camera::Singleton().front.z);

            } else if (glfwGetKey(glfwWindow, GLFW_KEY_P) == GLFW_RELEASE &&
                       pressed == true) {
              pressed = false;
            }
          }
        });

    updatePerPeriod(std::chrono::milliseconds(10000), [this](int, double) {
      printf("Position: (%d, %d)\n", currentPosition.x, currentPosition.y);
      printf("Size: (%d, %d)\n", currentSize.width, currentSize.height);
      printf("Iconified: %s\n", iconified ? "true" : "false");
    });
  };

  const static auto registerGLFWCallback = [this]() {
    glfwSetWindowPosCallback(glfwWindow, [](GLFWwindow *window, int x, int y) {
      class window *self = (class window *)glfwGetWindowUserPointer(window);
      self->currentPosition = {x, y};
#ifndef NDEBUG
      printf("[ window ] DEBUG: glfwSetWindowPosCallback triggered: Pos(%d, "
             "%d)\n",
             x, y);
#endif
    });

    glfwSetWindowSizeCallback(glfwWindow, [](GLFWwindow *window, int width,
                                             int height) {
      class window *self = (class window *)glfwGetWindowUserPointer(window);
      self->currentSize = {uint32_t(width), uint32_t(height)};
      graphic::Singleton().WaitIdle();
      // VK_ERROR_OUT_OF_DATE_KHR would handle this
      // graphic::Singleton().RecreateSwapchain();
#ifndef NDEBUG
      printf("[ window ] DEBUG: glfwSetWindowSizeCallback triggered: Size(%d, "
             "%d)\n",
             width, height);
#endif
    });

    glfwSetWindowIconifyCallback(
        glfwWindow, [](GLFWwindow *window, int iconify) {
          class window *self = (class window *)glfwGetWindowUserPointer(window);
          self->iconified = iconify;
#ifndef NDEBUG
          printf("[ window ] DEBUG: glfwSetWindowIconifyCallback triggered: "
                 "Iconified(%d)\n",
                 iconify);
#endif
        });

    glfwSetCursorPosCallback(
        glfwWindow, [](GLFWwindow *window, double xpos, double ypos) {
          static double lastX = xpos, lastY = ypos;
          static bool firstMouse = true;
          if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
          }
          float xoffset = xpos - lastX;
          float yoffset = ypos - lastY;
          lastX = xpos;
          lastY = ypos;

          static float sensitivity = 0.5f;
          class window *self = (class window *)glfwGetWindowUserPointer(window);
          if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED &&
              glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) != GLFW_PRESS) {
            camera::Singleton().yaw += xoffset * sensitivity * .1f;
            camera::Singleton().pitch += yoffset * sensitivity * .1f;

            if (camera::Singleton().pitch > 89.0f)
              camera::Singleton().pitch = 89.0f;
            if (camera::Singleton().pitch < -89.0f)
              camera::Singleton().pitch = -89.0f;
            if (camera::Singleton().yaw > 180.0f)
              camera::Singleton().yaw -= 360.0f;
            if (camera::Singleton().yaw < -180.0f)
              camera::Singleton().yaw += 360.0f;
            camera::Singleton().updateCameraVectors();
          }
        });

    glfwSetMouseButtonCallback(glfwWindow, [](GLFWwindow *window, int button,
                                              int action, int mods) {
      class window *self = (class window *)glfwGetWindowUserPointer(window);
      if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        if (facing == uint32_t(Face::EMERGED)) {
#ifndef NDEBUG
          printf("[ window ] DEBUG: Emerged by block\n");
#endif
          return;
        }
        if (!aimingEntity) {
#ifndef NDEBUG
          printf("[ window ] DEBUG: Aiming air\n");
#endif
          return;
        }
        glm::ivec3 position = aimingEntity->position;
        switch (Face(facing)) {
        case Face::FRONT:
          position += glm::ivec3(0, 0, 1);
          break;
        case Face::BACK:
          position += glm::ivec3(0, 0, -1);
          break;
        case Face::LEFT:
          position += glm::ivec3(-1, 0, 0);
          break;
        case Face::RIGHT:
          position += glm::ivec3(1, 0, 0);
          break;
        case Face::TOP: // vulkan reverse y
          position -= glm::ivec3(0, 1, 0);
          break;
        case Face::BOTTOM:
          position -= glm::ivec3(0, -1, 0);
          break;
        default:
#ifndef NDEBUG
          printf("[ window ] DEBUG: Facing is UNDEFINED\n");
#endif
          return;
          break;
        }
        if (glm::ivec3(glm::round(camera::Singleton().position)) == position) {
#ifndef NDEBUG
          printf("[ window ] DEBUG: Blocked by camera\n");
#endif
          return;
        }
        block entity(position, holdingItem);
        self->worldInstance.setEntity(position, entity);
#ifndef NDEBUG
        printf("[ window ] DEBUG: Placing block: position(%d, %d, %d) "
               "index(%lu)\n",
               position.x, position.y, position.z, holdingItem);
#endif
        return;
      }
      if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (!aimingEntity) {
#ifndef NDEBUG
          printf("[ window ] DEBUG: Aiming air\n");
#endif
          return;
        }
#ifndef NDEBUG
        printf("[ window ] DEBUG: Removing block: position(%d, %d, %d)\n",
               aimingEntity->position.x, aimingEntity->position.y,
               aimingEntity->position.z);
#endif
        self->worldInstance.removeEntity(aimingEntity->position);
      }
    });

    glfwSetScrollCallback(
        glfwWindow, [](GLFWwindow *window, double xoffset, double yoffset) {
          class window *self = (class window *)glfwGetWindowUserPointer(window);
          camera::Singleton().fov -= yoffset;
          if (camera::Singleton().fov < 30.0f)
            camera::Singleton().fov = 30.0f;
          if (camera::Singleton().fov > 120.0f)
            camera::Singleton().fov = 120.0f;
        });
  };

  registerLogicUpdateCallback();
  registerGLFWCallback();

  vertex cubeVertices[] = {
      // Right face (CW order)
      {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},
      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},

      // Left face (CW order)
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
      {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},

      // Top face (CW order)
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},

      // Bottom face (CW order)
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}},
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}},

      // Front face (CW order)
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},

      // Back face (CW order)
      {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
      {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},

  };

  worldInstance.initializeWorld(0);

  uint16_t indices[] = {
      // CW order
      0,  1,  2,  2,  3,  0,  // Right
      4,  5,  6,  6,  7,  4,  // Left
      8,  9,  10, 10, 11, 8,  // Top
      12, 13, 14, 14, 15, 12, // Bottom
      16, 17, 18, 18, 19, 16, // Front
      20, 21, 22, 22, 23, 20, // Back
  };

  uint16_t edges[] = {// Front face
                      0, 1, 1, 2, 2, 3, 3, 0,
                      // Back face
                      4, 5, 5, 6, 6, 7, 7, 4,
                      // Left face
                      8, 9, 9, 10, 10, 11, 11, 8,
                      // Right face
                      12, 13, 13, 14, 14, 15, 15, 12,
                      // Top face
                      16, 17, 17, 18, 18, 19, 19, 16,
                      // Bottom face
                      20, 21, 21, 22, 22, 23, 23, 20};

  vulkanWrapper::vertexBuffer cubeVerticesBuffer(sizeof cubeVertices);
  cubeVerticesBuffer.TransferData(cubeVertices, sizeof cubeVertices);

  vulkanWrapper::storageBuffer instanceBuffer(
      sizeof(instance) * worldInstance.entities.size() * 8);
  // vulkanWrapper::vertexBuffer instanceBuffer(sizeof(glm::mat4) *
  // models.size());

  vulkanWrapper::indexBuffer cubeVertexIndexBuffer(sizeof indices);
  cubeVertexIndexBuffer.TransferData(indices, sizeof indices);

  vulkanWrapper::indexBuffer cubeEdgeIndexBuffer(sizeof edges);
  cubeEdgeIndexBuffer.TransferData(edges, sizeof edges);

  vulkanWrapper::uniformBuffer ubo_mvp(sizeof(MVP));

  vulkanWrapper::pipelineLayout cubePipelineLayout;
  vulkanWrapper::pipelineLayout outlinePipelineLayout;
  vulkanWrapper::pipelineLayout cursorPipelineLayout;
  vulkanWrapper::pipelineLayout skyboxPipelineLayout;

  vulkanWrapper::pipeline cubePipeline;
  vulkanWrapper::pipeline outlinePipeline;
  vulkanWrapper::pipeline cursorPipeline;
  vulkanWrapper::pipeline skyboxPipeline;

  vulkanWrapper::descriptorSetLayout descSetLayout_mainRender;
  vulkanWrapper::descriptorSetLayout descSetLayout_compute;
  vulkanWrapper::descriptorSetLayout descSetLayout_skybox;

  const rpwfUtils::renderPassWithFramebuffers &rpwf =
      RenderPassAndFramebuffers();
  const vulkanWrapper::renderPass &renderPass = rpwf.renderPass;
  const std::vector<vulkanWrapper::framebuffer> &framebuffers =
      rpwf.framebuffers;

  textureManager::Singleton().registerDynamicTexture2d(
      "res/vulkanCraft/texture/"
      "diamond_block.png");
  textureManager::Singleton().registerDynamicTexture2d(
      "res/vulkanCraft/texture/"
      "gold_block.png");
  textureManager::Singleton().registerDynamicTexture2d(
      "res/vulkanCraft/texture/"
      "emerald_block.png");
  textureManager::Singleton().registerDynamicTexture2d(
      "res/vulkanCraft/texture/"
      "redstone_block.png");
  textureManager::Singleton().registerTexture2d("res/vulkanCraft/texture/"
                                                "lapis_block.png");
  textureManager::Singleton().registerTexture2d("res/vulkanCraft/texture/"
                                                "oak_planks.png");
  textureManager::Singleton().registerTexture2d("res/vulkanCraft/texture/"
                                                "warped_planks.png");

  {   // alloc descset
    { // main render
      std::vector<VkDescriptorSetLayoutBinding> bindings;
      VkDescriptorSetLayoutBinding uboDescriptorSetLayoutBinding = {
          .binding = 0, // 描述符被绑定到0号binding
          .descriptorType =
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // 类型为uniform缓冲区
          .descriptorCount = 1,                  // 个数是1个
          .stageFlags =
              VK_SHADER_STAGE_VERTEX_BIT // 在顶点着色器阶段读取uniform缓冲区
      };
      VkDescriptorSetLayoutBinding instanceBufferDescSetLayoutBinding = {
          .binding = 2,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      };
      VkDescriptorSetLayoutBinding cubeTextureDescriptorSetLayoutBinding = {
          .binding = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount =
              uint32_t(textureManager::Singleton()
                           .Index()), //  desc Count won't go above 2.1G right?
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      };

      bindings.push_back(uboDescriptorSetLayoutBinding);
      bindings.push_back(instanceBufferDescSetLayoutBinding);
      bindings.push_back(cubeTextureDescriptorSetLayoutBinding);

      uint32_t bindingCount = bindings.size();
      VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
          .bindingCount = bindingCount, .pBindings = bindings.data()};
      descSetLayout_mainRender.Create(descriptorSetLayoutCreateInfo);
    }

    { // compute
      std::vector<VkDescriptorSetLayoutBinding> bindings;
      VkDescriptorSetLayoutBinding postProcessImageDescSetLayoutBinding = {
          .binding = 0,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      };
      VkDescriptorSetLayoutBinding maskImageDescSetLayoutBinding = {
          .binding = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      };

      bindings.push_back(postProcessImageDescSetLayoutBinding);
      bindings.push_back(maskImageDescSetLayoutBinding);
      const uint32_t bindingCount = bindings.size();
      VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
          .bindingCount = bindingCount, .pBindings = bindings.data()};
      descSetLayout_compute.Create(descriptorSetLayoutCreateInfo);
    }

    { // skybox
      std::vector<VkDescriptorSetLayoutBinding> skyboxImageDescSetLayoutBinding(
          2);
      skyboxImageDescSetLayoutBinding[0] = {
          .binding = 0, // 描述符被绑定到0号binding
          .descriptorType =
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // 类型为uniform缓冲区
          .descriptorCount = 1,                  // 个数是1个
          .stageFlags =
              VK_SHADER_STAGE_VERTEX_BIT // 在顶点着色器阶段读取uniform缓冲区
      };
      skyboxImageDescSetLayoutBinding[1] = {
          .binding = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount = 6, // skybox texture count
          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      };

      const uint32_t bindingCount = skyboxImageDescSetLayoutBinding.size();
      VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
          .bindingCount = bindingCount,
          .pBindings = skyboxImageDescSetLayoutBinding.data()};
      descSetLayout_skybox.Create(descriptorSetLayoutCreateInfo);
    }
  }

  VkPushConstantRange pushConstantRange_main = {
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .offset = 0,
      .size = sizeof(pushConstant_outline),
  };
  VkPushConstantRange pushConstantRange_cursor = {
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .offset = 0,
      .size = sizeof(pushConstant_cursor),
  };
  VkPushConstantRange pushConstantRange_skybox = {
      .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
      .offset = 0,
      .size = sizeof(pushConstant_skybox),
  };

  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo_cube = {
      .setLayoutCount = 1,
      .pSetLayouts = descSetLayout_mainRender.Address(),
      .pushConstantRangeCount = 0,
  };
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo_outline = {
      .setLayoutCount = 1,
      .pSetLayouts = descSetLayout_mainRender.Address(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange_main,
  };
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo_cursor = {
      .setLayoutCount = 1,
      .pSetLayouts = descSetLayout_compute.Address(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange_cursor,
  };
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo_skybox = {
      .setLayoutCount = 1,
      .pSetLayouts = descSetLayout_skybox.Address(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange_skybox,
  };
  cubePipelineLayout.Create(pipelineLayoutCreateInfo_cube);
  outlinePipelineLayout.Create(pipelineLayoutCreateInfo_outline);
  cursorPipelineLayout.Create(pipelineLayoutCreateInfo_cursor);
  skyboxPipelineLayout.Create(pipelineLayoutCreateInfo_skybox);

  CreatePipelineCube(cubePipeline, cubePipelineLayout);
  CreatePipelineOutline(outlinePipeline, outlinePipelineLayout);
  CreatePipelineCursor(cursorPipeline, cursorPipelineLayout);
  CreatePipelineSkybox(skyboxPipeline, skyboxPipelineLayout);

  std::vector<VkDescriptorPoolSize> descriptorPoolSizes_main = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
       uint32_t(textureManager::Singleton().Index())}, // maybe wont overflow
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},

  };
  std::vector<VkDescriptorPoolSize> descriptorPoolSizes_cursor = {
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
  };
  std::vector<VkDescriptorPoolSize> descriptorPoolSizes_skybox = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6},
  };
  vulkanWrapper::descriptorPool descriptorPool_main(1,
                                                    descriptorPoolSizes_main);
  vulkanWrapper::descriptorPool descriptorPool_cursor(
      1, descriptorPoolSizes_cursor);
  vulkanWrapper::descriptorPool descriptorPool_skybox(
      1, descriptorPoolSizes_skybox);

  vulkanWrapper::descriptorSet descSet_main;
  vulkanWrapper::descriptorSet descSet_cursor;
  vulkanWrapper::descriptorSet descSet_skybox;

  {
    std::vector<VkDescriptorSet> descSetAllocateTransfer = {descSet_main};
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
        descSetLayout_mainRender};
    descriptorPool_main.AllocateSets(descSetAllocateTransfer,
                                     descriptorSetLayouts);
    descSet_main = descSetAllocateTransfer[0];
  }
  {
    std::vector<VkDescriptorSet> descSetAllocateTransfer = {descSet_cursor};
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
        descSetLayout_compute};
    descriptorPool_cursor.AllocateSets(descSetAllocateTransfer,
                                       descriptorSetLayouts);
    descSet_cursor = descSetAllocateTransfer[0];
  }
  {
    std::vector<VkDescriptorSet> descSetAllocateTransfer = {descSet_skybox};
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
        descSetLayout_skybox};
    descriptorPool_skybox.AllocateSets(descSetAllocateTransfer,
                                       descriptorSetLayouts);
    descSet_skybox = descSetAllocateTransfer[0];
  }

  vulkanWrapper::sampler sampler;
  CreateSampler(sampler);

  // vulkanWrapper::texture2d
  // texture("/home/awwwsl/code/learn/cpp/learnVulkan/"
  //                                  "res/vulkanCraft/texture/lapis_block.png",
  //                                  VK_FORMAT_R8G8B8A8_UNORM,
  //                                  VK_FORMAT_R8G8B8A8_UNORM, true,
  //                                  VK_FILTER_NEAREST);
  vulkanWrapper::texture2d cursor("/home/awwwsl/code/learn/cpp/learnVulkan/res/"
                                  "vulkanCraft/texture/cursor.png",
                                  VK_FORMAT_R8G8B8A8_UNORM,
                                  VK_FORMAT_R8G8B8A8_UNORM, false,
                                  VK_FILTER_NEAREST);

  // 0 = +X, 1 = -X, 2 = +Y, 3 = -Y, 4 = +Z, 5 = -Z
  vulkanWrapper::dynamicTexture2d skybox(
      "/home/awwwsl/code/learn/cpp/learnVulkan/res/"
      "vulkanCraft/texture/skybox.png",
      VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, false,
      VK_FILTER_LINEAR);

  std::vector<VkDescriptorImageInfo> skyboxImageInfos(6);
  for (int i = 0; i < 6; i++) {
    VkDescriptorImageInfo skyboxImageInfo = {
        .sampler = sampler,
        .imageView = skybox.ImageViews()[i],
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    skyboxImageInfos[i] = skyboxImageInfo;
  }
  descSet_skybox.Write(skyboxImageInfos,
                       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
  std::vector<VkDescriptorBufferInfo> skyboxUBOInfos = {
      {
          .buffer = ubo_mvp,
          .offset = 0,
          .range = sizeof(MVP),
      },
  };
  descSet_skybox.Write(skyboxUBOInfos, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);

  VkDescriptorBufferInfo storageBufferInfo_instance = {
      .buffer = instanceBuffer,
      .offset = 0,
      .range = VK_WHOLE_SIZE,
  };
  VkDescriptorBufferInfo uniformBufferInfo_MVP = {
      .buffer = ubo_mvp,
      .offset = 0,
      .range = sizeof(MVP),
  };

  std::vector<VkDescriptorBufferInfo> uboBufferInfos = {uniformBufferInfo_MVP};
  std::vector<VkDescriptorBufferInfo> storageBufferInfos = {
      storageBufferInfo_instance};
  descSet_main.Write(uboBufferInfos, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
  descSet_main.Write(storageBufferInfos, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2);

  VkDescriptorImageInfo cursorImageInfo = {
      .sampler = sampler,
      .imageView = cursor.ImageView(),
      .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
  };
  std::vector<VkDescriptorImageInfo> cursorImageInfos = {cursorImageInfo};
  descSet_cursor.Write(cursorImageInfos,
                       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
  // descSet_main.Write(cursorImageInfos,
  //                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3);

  vulkanWrapper::fence fence(VK_FENCE_CREATE_SIGNALED_BIT);
  vulkanWrapper::semaphore semaphore_imageAvailable;
  vulkanWrapper::semaphore semaphore_renderFinished;
  vulkanWrapper::semaphore semaphore_computeFinished;

  vulkanWrapper::commandBuffer primaryCommandBuffer;

  vulkanWrapper::commandBuffer cubeCommandBuffer;
  vulkanWrapper::commandBuffer outlineCommandBuffer;
  vulkanWrapper::commandBuffer postProcessComputeBuffer;
  vulkanWrapper::commandBuffer skyboxCommandBuffer;

  std::vector<VkCommandBuffer> commandBufferTransports_primary = {
      primaryCommandBuffer,
      postProcessComputeBuffer,
  };
  std::vector<VkCommandBuffer> commandBufferTransports_secondary = {
      cubeCommandBuffer,
      outlineCommandBuffer,
      skyboxCommandBuffer,
  };

  vulkanWrapper::commandPool commandPool(
      graphic::Singleton().QueueFamilyIndex_Graphics(),
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  commandPool.AllocateBuffers(commandBufferTransports_primary,
                              VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  commandPool.AllocateBuffers(commandBufferTransports_secondary,
                              VK_COMMAND_BUFFER_LEVEL_SECONDARY);
  primaryCommandBuffer = commandBufferTransports_primary[0];
  postProcessComputeBuffer = commandBufferTransports_primary[1];

  cubeCommandBuffer = commandBufferTransports_secondary[0];
  outlineCommandBuffer = commandBufferTransports_secondary[1];
  skyboxCommandBuffer = commandBufferTransports_secondary[2];

  auto color = color::floatRGBA(101, 101, 101, 255);
  VkClearValue clearColor = {
      .color = {{std::get<0>(color), std::get<1>(color), std::get<2>(color),
                 std::get<3>(color)}}}; // 灰色
  VkClearValue depthClear = {.depthStencil = {1.f, 0}};
  std::vector<VkClearValue> clearValues = {clearColor, depthClear};

  loading = false;
  BootScreenThread.join();

  fence.Reset();

  uint32_t textureSelection = 0;
  while (!glfwWindowShouldClose(glfwWindow)) {
    while (glfwGetWindowAttrib(glfwWindow, GLFW_ICONIFIED)) {
      glfwWaitEvents();
    }

    std::vector<instance> instances = worldInstance.getInstances();
    instanceBuffer.TransferData(instances.data(),
                                sizeof(instance) * instances.size());

    textureSelection++;
    textureSelection %= 15;
    if (textureSelection == 0) {
      textureManager::Singleton().UpdateDynamicTextureView();
    }
    graphic::Singleton().SwapImage(semaphore_imageAvailable);
    uint32_t currentFramebufferIndex = graphic::Singleton().CurrentImageIndex();

    // calculate data
    MVP mvp;
    {
      glm::mat4 model = glm::mat4(1.f);
      model = glm::translate(model, glm::vec3(camera::Singleton().position));
      model = glm::scale(model, glm::vec3(50.f));
      mvp.model = model;
      mvp.view = camera::Singleton().getViewMatrix();
      mvp.projection = camera::Singleton().getProjectionMatrix(
          currentSize.width, currentSize.height);
    }

    aimingEntity = rayIntersection(camera::Singleton().position,
                                   camera::Singleton().front, 5.0f, &facing);

    // TransferData
    cubeVerticesBuffer.TransferData(cubeVertices, sizeof cubeVertices);
    ubo_mvp.TransferData(&mvp, sizeof(MVP));

    std::vector<VkDescriptorImageInfo> imageInfos(
        textureManager::Singleton().Index());
    for (uint64_t i = 0; i < textureManager::Singleton().Index(); i++) {
      VkDescriptorImageInfo textureImageInfo = {
          .sampler = sampler,
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      };
      if (textureManager::Singleton().staticTextures2d.find(i) !=
          textureManager::Singleton().staticTextures2d.end()) {
        textureImageInfo.imageView =
            textureManager::Singleton().staticTextures2d.at(i)->ImageView();
      } else {
        textureImageInfo.imageView =
            textureManager::Singleton().dynamicTextures2d.at(i)->CurrentView();
      }
      imageInfos[i] = textureImageInfo;
    }
    descSet_main.Write(imageInfos, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                       1);

    VkDescriptorImageInfo swapchainImageInfo = {
        .sampler = nullptr,
        .imageView =
            graphic::Singleton().SwapchainImageView(currentFramebufferIndex),
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
    };
    std::vector<VkDescriptorImageInfo> swapchainImageInfos = {
        swapchainImageInfo};
    descSet_cursor.Write(swapchainImageInfos, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                         0);

    VkCommandBufferInheritanceInfo inheritanceInfo_subpass0 = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        .renderPass = renderPass,
        .subpass = 0,
        .framebuffer = framebuffers[currentFramebufferIndex],
    };
    // VkCommandBufferInheritanceInfo inheritanceInfo_subpass1 = {
    //     .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
    //     .renderPass = renderPass,
    //     .subpass = 1,
    //     .framebuffer = framebuffers[i],
    // };

    // main render
    {
      VkDeviceSize offset = 0;
      outlineCommandBuffer.Begin(
          VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
          inheritanceInfo_subpass0);
      if (aimingEntity != nullptr) {
        struct pushConstant_outline push = {
            .aimingBlockModel = glm::translate(
                glm::mat4(1.f), glm::vec3(aimingEntity->position)),
            .colorRGBA = {0.f, 0.f, 0.f, 1.f},
        };

        // outlines
        vkCmdBindPipeline(outlineCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          outlinePipeline);
        vkCmdBindDescriptorSets(
            outlineCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            outlinePipelineLayout, 0, 1, descSet_main.Address(), 0, nullptr);
        vkCmdBindVertexBuffers(outlineCommandBuffer, 0, 1,
                               cubeVerticesBuffer.Address(), &offset);
        vkCmdBindIndexBuffer(outlineCommandBuffer, cubeEdgeIndexBuffer, 0,
                             VK_INDEX_TYPE_UINT16);
        vkCmdPushConstants(outlineCommandBuffer, outlinePipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(pushConstant_outline), &push);
        vkCmdDrawIndexed(outlineCommandBuffer, 48, 1, 0, 0, 0);
      }
      outlineCommandBuffer.End();

      { // cube
        cubeCommandBuffer.Begin(
            VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
            inheritanceInfo_subpass0);
        vkCmdBindPipeline(cubeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          cubePipeline);

        vkCmdBindVertexBuffers(cubeCommandBuffer, 0, 1,
                               cubeVerticesBuffer.Address(), &offset);
        vkCmdBindIndexBuffer(cubeCommandBuffer, cubeVertexIndexBuffer, 0,
                             VK_INDEX_TYPE_UINT16);
        vkCmdBindDescriptorSets(
            cubeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            cubePipelineLayout, 0, 1, descSet_main.Address(), 0, nullptr);
        vkCmdDrawIndexed(cubeCommandBuffer, 36, instances.size(), 0, 0, 0);
        cubeCommandBuffer.End();
      }

      { // skybox
        skyboxCommandBuffer.Begin(
            VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
            inheritanceInfo_subpass0);
        vkCmdBindPipeline(skyboxCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          skyboxPipeline);
        vkCmdBindVertexBuffers(skyboxCommandBuffer, 0, 1,
                               cubeVerticesBuffer.Address(), &offset);
        vkCmdBindIndexBuffer(skyboxCommandBuffer, cubeVertexIndexBuffer, 0,
                             VK_INDEX_TYPE_UINT16);
        vkCmdBindDescriptorSets(
            skyboxCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            skyboxPipelineLayout, 0, 1, descSet_skybox.Address(), 0, nullptr);
        for (uint32_t i = 0; i < 6; i++) {
          struct pushConstant_skybox push = {.facing = i};
          vkCmdPushConstants(skyboxCommandBuffer, skyboxPipelineLayout,
                             VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                             sizeof(pushConstant_skybox), &push);

          vkCmdDrawIndexed(skyboxCommandBuffer, 6, 1, i * 6, 0, 0);
        }
        skyboxCommandBuffer.End();
      }

      { // primary
        primaryCommandBuffer.Begin();
        renderPass.CmdBegin(
            primaryCommandBuffer, framebuffers[currentFramebufferIndex],
            {{}, {framebuffers[currentFramebufferIndex].Size()}}, clearValues,
            VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        std::vector<VkCommandBuffer> subCommandBuffers = {
            cubeCommandBuffer,
            outlineCommandBuffer,
            skyboxCommandBuffer,
        };
        vkCmdExecuteCommands(primaryCommandBuffer, subCommandBuffers.size(),
                             subCommandBuffers.data());

        renderPass.CmdEnd(primaryCommandBuffer);
        primaryCommandBuffer.End();
      }
    }

    { // post process
      struct pushConstant_cursor push = {
          .imageExtent = graphic::Singleton().SwapchainCreateInfo().imageExtent,
          .maskExtentMultiplied = cursor.Extent(),
          .cursorScale = 2,
      };
      push.maskExtentMultiplied.width *= push.cursorScale;
      push.maskExtentMultiplied.height *= push.cursorScale;

      postProcessComputeBuffer.Begin(
          VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
      vkCmdBindPipeline(postProcessComputeBuffer,
                        VK_PIPELINE_BIND_POINT_COMPUTE, cursorPipeline);
      vkCmdBindDescriptorSets(
          postProcessComputeBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
          cursorPipelineLayout, 0, 1, descSet_cursor.Address(), 0, nullptr);
      vkCmdPushConstants(postProcessComputeBuffer, cursorPipelineLayout,
                         VK_SHADER_STAGE_COMPUTE_BIT, 0,
                         sizeof(pushConstant_cursor), &push);

      VkImageMemoryBarrier barrier = {
          .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
          .srcAccessMask = 0,
          .dstAccessMask = 0,
          .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
          .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
          .srcQueueFamilyIndex =
              VK_QUEUE_FAMILY_IGNORED, // 如果是同一个队列，可以设置为忽略
          .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .image = graphic::Singleton().SwapchainImage(
              currentFramebufferIndex), // 目标图像
          .subresourceRange =
              {
                  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                  .baseMipLevel = 0,
                  .levelCount = 1,
                  .baseArrayLayer = 0,
                  .layerCount = 1,
              },
      };

      vkCmdDispatch(postProcessComputeBuffer, push.maskExtentMultiplied.width,
                    push.maskExtentMultiplied.height, 1);
      vkCmdPipelineBarrier(
          postProcessComputeBuffer,
          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // 等待计算着色器完成
          VK_PIPELINE_STAGE_TRANSFER_BIT, // 转换到传输阶段以支持呈现
          0,                              // 不需要依赖
          0, nullptr,                     // 不需要任何其他内存障碍
          0, nullptr,                     // 不需要其他依赖
          1, &barrier                     // 布局转换的障碍
      );
      postProcessComputeBuffer.End();
    }

    graphic::Singleton().SubmitCommandBuffer_Graphics(
        primaryCommandBuffer, semaphore_imageAvailable,
        semaphore_renderFinished, fence,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    fence.WaitAndReset();

    VkPipelineStageFlags pipelineStageFlag =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkCommandBuffer commandBufferVK = postProcessComputeBuffer;
    VkSubmitInfo computeSubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = semaphore_renderFinished.Address(),
        .pWaitDstStageMask = &pipelineStageFlag,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBufferVK,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = semaphore_computeFinished.Address(),
    };
    graphic::Singleton().SubmitCommandBuffer_Compute(computeSubmitInfo, fence);

    graphic::Singleton().PresentImage(semaphore_computeFinished);
    glfwPollEvents();
    updateLogic();

    fence.WaitAndReset();
  }
  printf("Exit\n");
  TerminateWindow();
}; // namespace learnVulkan

void window::updateLogic() {
  for (int i = 0; i < logicUpdateCallbacks.size(); i++) {
    auto &info = logicUpdateCallbacks[i];
    double currentTime = glfwGetTime();
    double timePassed = currentTime - info.lastTime;
    if (timePassed >= info.interval.count()) {
      info.callback(info.framePassed, timePassed);
      info.framePassed = 0;
      info.lastTime += info.interval.count();
    } else {
      info.framePassed++;
    }
  }
}

void window::TerminateWindow() {
  graphic::Singleton().WaitIdle();
  textureManager::Singleton().DestroyAllTexture();
  vulkanWrapper::stagingBuffer::ClearBuffers();
  graphic::Singleton().ClearDestroySwapchainCallbacks();
  graphic::Singleton().ClearDestroyDeviceCallbacks();
  // graphicsBase::Singleton().Terminate();
  glfwDestroyWindow(glfwWindow);
}

} // namespace learnVulkan
