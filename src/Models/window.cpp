#include "framebuffer.hpp"
#define GLFW_INCLUDE_VULKAN

#ifndef LIMIT_FRAME_RATE
#define LIMIT_FRAME_RATE true
#endif

#include "../Utils/VkResultThrowable.hpp"

#include "commandBuffer.hpp"
#include "commandPool.hpp"
#include "fence.hpp"
#include "graphic.hpp"
#include "graphicsPipelineCreateInfoPack.hpp"
#include "pipeline.hpp"
#include "pipelineLayout.hpp"
#include "semaphore.hpp"
#include "shader.hpp"
#include "window.hpp"

#include <sstream>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <GLFW/glfw3.h>

namespace learnVulkan::models {

window::window() {}

bool window::initialize() {
  // glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

  if (!glfwInit()) {
    printf("[ window ] Failed to initialize GLFW\n");
    return false;
  }

  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  glfwMonitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *glfwMonitorMode = glfwGetVideoMode(glfwMonitor);
  glfwWindow = glfwCreateWindow(defaultSize.width, defaultSize.height,
                                windowTitle.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(glfwWindow, this);

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
    printf("[ window ] Vulkan is not available on this machine!\n");
    glfwTerminate();
    return false;
  }

  for (size_t i = 0; i < extensionCount; i++) {
    graphicsBase::Singleton().AddInstanceExtension(extensionNames[i]);
  }
#endif
  graphicsBase::Singleton().AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  graphicsBase::Singleton().UseLatestApiVersion();
  if (graphicsBase::Singleton().CreateInstance()) {
    printf("[ window ] Failed to create Vulkan instance\n");
    return false;
  }

  VkSurfaceKHR surface = VK_NULL_HANDLE;
  if (VkResultThrowable result =
          glfwCreateWindowSurface(graphicsBase::Singleton().Instance(),
                                  glfwWindow, nullptr, &surface)) {
    printf("[ window ] ERROR: Failed to create a "
           "window surface!\nError code: %d\n",
           int32_t(result));
    glfwTerminate();
    return false;
  }
  graphicsBase::Singleton().Surface(surface);

  if (!graphicsBase::Singleton().GetPhysicalDevices()) {
    for (int i = 0;
         i < graphicsBase::Singleton().AvailablePhysicalDeviceCount(); i++) {
      auto physicalDevice =
          graphicsBase::Singleton().AvailablePhysicalDevice(i);
      VkPhysicalDeviceProperties properties;
      vkGetPhysicalDeviceProperties(physicalDevice, &properties);
      printf("[ window ] Device %d: %s\n", i, properties.deviceName);
    }
  }

  while (true) {
    printf("[ window ] Please select a device: ");
    int deviceIndex = 0;
    scanf("%d", &deviceIndex);

    if (deviceIndex < 0 ||
        deviceIndex >=
            graphicsBase::Singleton().AvailablePhysicalDeviceCount()) {
      printf("[ window ] Invalid device index\n");
      continue;
    }
    if (graphicsBase::Singleton().DeterminePhysicalDevice(deviceIndex, true,
                                                          false)) {
      printf("[ window ] Device not qualified for vulkan graphics queue\n");
      continue;
    }
    if (graphicsBase::Singleton().CreateDevice()) {
      printf("[ window ] Failed to create logical device\n");
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

  if (graphicsBase::Singleton().CreateSwapchain(LIMIT_FRAME_RATE))
    return false;

  return true;
}

const renderPassWithFramebuffers &RenderPassAndFramebuffers() {
  static const renderPassWithFramebuffers &rpwf =
      graphicsBase::Singleton().CreateRpwf_Screen();
  return rpwf;
}

const void CreateLayout(vulkanWrapper::pipelineLayout &layout) {
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
  layout.Create(pipelineLayoutCreateInfo);
}

const void CreatePipeline(vulkanWrapper::pipeline &pipeline,
                          vulkanWrapper::pipelineLayout &layout) {
  static vulkanWrapper::shader vert("src/Shaders/triangle.vert.spv");
  static vulkanWrapper::shader frag("src/Shaders/triangle.frag.spv");
  static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_triangle[2] = {
      vert.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
      frag.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};
  auto Create = [&] {
    const VkExtent2D &windowSize =
        graphicsBase::Singleton().SwapchainCreateInfo().imageExtent;
    graphicsPipelineCreateInfoPack pipelineCiPack;
    pipelineCiPack.createInfo.layout = layout;
    pipelineCiPack.createInfo.renderPass =
        RenderPassAndFramebuffers().renderPass;
    pipelineCiPack.inputAssemblyStateCi.topology =
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineCiPack.viewports.emplace_back(0.f, 0.f, float(windowSize.width),
                                          float(windowSize.height), 0.f, 1.f);
    pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);
    pipelineCiPack.multisampleStateCi.rasterizationSamples =
        VK_SAMPLE_COUNT_1_BIT;
    pipelineCiPack.colorBlendAttachmentStates.push_back(
        {.colorWriteMask = 0b1111});
    pipelineCiPack.UpdateAllArrays();
    pipelineCiPack.createInfo.stageCount = 2;
    pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_triangle;
    pipeline.Create(pipelineCiPack);
  };
  auto Destroy = [&] { pipeline.~pipeline(); };
  graphicsBase::Singleton().AddCreateSwapchainCallback(Create);
  graphicsBase::Singleton().AddDestroySwapchainCallback(Destroy);
  Create();
}

void window::run() {
  updatePerPeriod(std::chrono::seconds(1), [this](int dframe, double dt) {
    std::stringstream info;
    info.precision(1);
    info << windowTitle << "    " << std::fixed << dframe / dt << " FPS";
    glfwSetWindowTitle(glfwWindow, info.str().c_str());
  });

  updatePerPeriod(std::chrono::milliseconds(20), [this](int, double) {
    bool speeding = glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SHIFT);

    if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(glfwWindow, GLFW_TRUE);
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
      currentPosition.y += 10 * ((speeding) ? 5 : 1);
      printf("Down\n");
      MakeWindowWindowed(currentPosition, currentSize);
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_UP) == GLFW_PRESS) {
      currentPosition.y -= 10 * ((speeding) ? 5 : 1);
      printf("Up\n");
      MakeWindowWindowed(currentPosition, currentSize);
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
      currentPosition.x -= 10 * ((speeding) ? 5 : 1);
      printf("Left\n");
      MakeWindowWindowed(currentPosition, currentSize);
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
      currentPosition.x += 10 * ((speeding) ? 5 : 1);
      printf("Right\n");
      MakeWindowWindowed(currentPosition, currentSize);
    }
  });

  updatePerPeriod(std::chrono::milliseconds(1000), [this](int, double) {
    printf("Position: (%d, %d)\n", currentPosition.x, currentPosition.y);
    printf("Size: (%d, %d)\n", currentSize.width, currentSize.height);
    printf("Iconified: %s\n", iconified ? "true" : "false");
  });

  glfwSetWindowPosCallback(glfwWindow, [](GLFWwindow *window, int x, int y) {
    class window *self = (class window *)glfwGetWindowUserPointer(window);
    self->currentPosition = {x, y};
  });

  glfwSetWindowSizeCallback(
      glfwWindow, [](GLFWwindow *window, int width, int height) {
        class window *self = (class window *)glfwGetWindowUserPointer(window);
        self->currentSize = {uint32_t(width), uint32_t(height)};
        graphicsBase::Singleton().WaitIdle();
        graphicsBase::Singleton().RecreateSwapchain();
      });

  glfwSetWindowIconifyCallback(glfwWindow, [](GLFWwindow *window, int iconify) {
    class window *self = (class window *)glfwGetWindowUserPointer(window);
    self->iconified = iconify;
  });

  const renderPassWithFramebuffers &rpwf = RenderPassAndFramebuffers();
  const vulkanWrapper::renderPass &renderPass = rpwf.renderPass;
  const std::vector<vulkanWrapper::framebuffer> &framebuffers =
      rpwf.framebuffers;
  vulkanWrapper::pipelineLayout layout_triangle;
  vulkanWrapper::pipeline pipeline_triangle;

  CreateLayout(layout_triangle);
  CreatePipeline(pipeline_triangle, layout_triangle);

  vulkanWrapper::fence fence(VK_FENCE_CREATE_SIGNALED_BIT);
  vulkanWrapper::semaphore semaphore_imageAvailable;
  vulkanWrapper::semaphore semaphore_renderFinished;

  vulkanWrapper::commandBuffer commandBuffer;
  vulkanWrapper::commandPool commandPool(
      graphicsBase::Singleton().QueueFamilyIndex_Graphics(),
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  commandPool.AllocateBuffers(commandBuffer);

  VkClearValue clearColor = {.color = {1.f, 0.f, 0.f, 1.f}}; // 红色
  std::vector<VkClearValue> clearValues = {clearColor};

  fence.Reset();
  while (!glfwWindowShouldClose(glfwWindow)) {

    while (glfwGetWindowAttrib(glfwWindow, GLFW_ICONIFIED)) {
      glfwWaitEvents();
    }

    VkExtent2D windowSize =
        graphicsBase::Singleton().SwapchainCreateInfo().imageExtent;

    graphicsBase::Singleton().SwapImage(semaphore_imageAvailable);
    uint32_t i = graphicsBase::Singleton().CurrentImageIndex();

    commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    renderPass.CmdBegin(commandBuffer, framebuffers[i], {{}, windowSize},
                        clearValues, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline_triangle);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    renderPass.CmdEnd(commandBuffer);
    commandBuffer.End();

    graphicsBase::Singleton().SubmitCommandBuffer_Graphics(
        commandBuffer, semaphore_imageAvailable, semaphore_renderFinished,
        fence);
    graphicsBase::Singleton().PresentImage(semaphore_renderFinished);

    glfwPollEvents();
    updateLogic();

    fence.WaitAndReset();
  }
  printf("Exit\n");
  TerminateWindow();
};

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
  graphicsBase::Singleton().WaitIdle();
  graphicsBase::Singleton().ClearDestroySwapchainCallbacks();
  graphicsBase::Singleton().ClearDestroyDeviceCallbacks();
  // graphicsBase::Singleton().Terminate();
  glfwDestroyWindow(glfwWindow);
}

} // namespace learnVulkan::models
