#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#ifndef LIMIT_FRAME_RATE
#define LIMIT_FRAME_RATE true
#endif

#include "../Utils/VkResultThrowable.hpp"
#include "../Utils/color.hpp"

#include "../Vulkan/vulkanWrapper.hpp"

#include "graphic.hpp"
#include "graphicPlus.hpp"
#include "window.hpp"

#include <sstream>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace learnVulkan {

struct vertex {
  glm::vec2 position;
  glm::vec4 color;
};

window::window() {}

bool window::initialize() {
  // glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

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

  while (true) {
    printf("[ window ] HINT: Please select a device: ");
    int deviceIndex = 0;
    scanf("%d", &deviceIndex);

    if (deviceIndex < 0 ||
        deviceIndex >= graphic::Singleton().AvailablePhysicalDeviceCount()) {
      printf("[ window ] ERROR: Invalid device index\n");
      continue;
    }
    if (graphic::Singleton().DeterminePhysicalDevice(deviceIndex, true,
                                                     false)) {
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

const renderPassWithFramebuffers &RenderPassAndFramebuffers() {
  static const renderPassWithFramebuffers &rpwf =
      graphic::Singleton().CreateRpwf_Screen();
  return rpwf;
}

const void CreateLayout(vulkanWrapper::pipelineLayout &layout) {
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
  layout.Create(pipelineLayoutCreateInfo);
}

const void CreatePipeline(vulkanWrapper::pipeline &pipeline,
                          vulkanWrapper::pipelineLayout &layout) {
  static vulkanWrapper::shader vert(
      "src/Shaders/vertexBufferTriangle.vert.spv");
  static vulkanWrapper::shader frag(
      "src/Shaders/vertexBufferTriangle.frag.spv");
  static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_triangle[2] = {
      vert.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
      frag.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};
  auto Create = [&] {
    const VkExtent2D &windowSize =
        graphic::Singleton().SwapchainCreateInfo().imageExtent;
    graphicsPipelineCreateInfoPack pipelineCiPack;

    // 数据来自0号顶点缓冲区，输入频率是逐顶点输入
    pipelineCiPack.vertexInputBindings.emplace_back(
        0, sizeof(vertex), VK_VERTEX_INPUT_RATE_VERTEX);
    // location为0，数据来自0号顶点缓冲区，vec2对应VK_FORMAT_R32G32_SFLOAT，用offsetof计算position在vertex中的起始位置
    pipelineCiPack.vertexInputAttributes.emplace_back(
        0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex, position));
    // location为1，数据来自0号顶点缓冲区，vec4对应VK_FORMAT_R32G32B32A32_SFLOAT，用offsetof计算color在vertex中的起始位置
    pipelineCiPack.vertexInputAttributes.emplace_back(
        1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex, color));

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
  graphic::Singleton().AddCreateSwapchainCallback(Create);
  graphic::Singleton().AddDestroySwapchainCallback(Destroy);
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
#ifndef NDEBUG
    printf(
        "[ window ] DEBUG: glfwSetWindowPosCallback triggered: Pos(%d, %d)\n",
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
    printf(
        "[ window ] DEBUG: glfwSetWindowSizeCallback triggered: Size(%d, %d)\n",
        width, height);
#endif
  });

  glfwSetWindowIconifyCallback(glfwWindow, [](GLFWwindow *window, int iconify) {
    class window *self = (class window *)glfwGetWindowUserPointer(window);
    self->iconified = iconify;
#ifndef NDEBUG
    printf("[ window ] DEBUG: glfwSetWindowIconifyCallback triggered: "
           "Iconified(%d)\n",
           iconify);
#endif
  });

  vertex vertices[] = {
      {{0.f, -0.5f}, {1.f, 0.f, 0.f, 1.f}},
      {{0.5f, 0.5f}, {0.f, 1.f, 0.f, 1.f}},
      {{-0.5f, 0.5f}, {0.f, 0.f, 1.f, 1.f}},
  };
  vulkanWrapper::vertexBuffer verticesBuffer(sizeof vertices);
  verticesBuffer.TransferData(vertices, sizeof vertices);

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
      graphic::Singleton().QueueFamilyIndex_Graphics(),
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  commandPool.AllocateBuffers(commandBuffer);

  auto color = color::floatRGBA(101, 101, 101, 255);
  VkClearValue clearColor = {.color = {std::get<0>(color), std::get<1>(color),
                                       std::get<2>(color),
                                       std::get<3>(color)}}; // 灰色
  std::vector<VkClearValue> clearValues = {clearColor};

  fence.Reset();
  while (!glfwWindowShouldClose(glfwWindow)) {

    while (glfwGetWindowAttrib(glfwWindow, GLFW_ICONIFIED)) {
      glfwWaitEvents();
    }

    graphic::Singleton().SwapImage(semaphore_imageAvailable);
    uint32_t i = graphic::Singleton().CurrentImageIndex();

    commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    renderPass.CmdBegin(commandBuffer, framebuffers[i],
                        {{}, {framebuffers[i].Size()}}, clearValues,
                        VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline_triangle);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, verticesBuffer.Address(),
                           &offset);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    renderPass.CmdEnd(commandBuffer);
    commandBuffer.End();

    graphic::Singleton().SubmitCommandBuffer_Graphics(
        commandBuffer, semaphore_imageAvailable, semaphore_renderFinished,
        fence);
    graphic::Singleton().PresentImage(semaphore_renderFinished);

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
  graphic::Singleton().WaitIdle();
  graphic::Singleton().ClearDestroySwapchainCallbacks();
  graphic::Singleton().ClearDestroyDeviceCallbacks();
  // graphicsBase::Singleton().Terminate();
  glfwDestroyWindow(glfwWindow);
}

} // namespace learnVulkan
