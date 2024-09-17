#include <glm/ext/matrix_float4x4.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#ifndef LIMIT_FRAME_RATE
#define LIMIT_FRAME_RATE true
#endif

#include "../Utils/VkResultThrowable.hpp"
#include "../Utils/color.hpp"

#include "../Vulkan/vulkanWrapper.hpp"

#include "camera.hpp"
#include "entity.hpp"
#include "graphic.hpp"
#include "graphicPlus.hpp"
#include "window.hpp"

#include <sstream>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace learnVulkan {

struct vertex {
  glm::vec3 position;
  glm::vec2 texCoord;
};

struct alignas(16) MVP {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;
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

const void
CreateLayout(vulkanWrapper::pipelineLayout &pipelineLayout,
             vulkanWrapper::descriptorSetLayout &descriptorSetLayout) {
  VkDescriptorSetLayoutBinding descriptorSetLayoutBinding_3dMVP = {
      .binding = 0, // 描述符被绑定到0号binding
      .descriptorType =
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, // 类型为uniform缓冲区
      .descriptorCount = 1,                  // 个数是1个
      .stageFlags =
          VK_SHADER_STAGE_VERTEX_BIT // 在顶点着色器阶段读取uniform缓冲区
  };
  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo_3dMVP = {
      .bindingCount = 1, .pBindings = &descriptorSetLayoutBinding_3dMVP};
  descriptorSetLayout.Create(descriptorSetLayoutCreateInfo_3dMVP);
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
      .setLayoutCount = 1, .pSetLayouts = descriptorSetLayout.Address()};
  pipelineLayout.Create(pipelineLayoutCreateInfo);
}

const void CreatePipeline(vulkanWrapper::pipeline &pipeline,
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

    pipelineCiPack.vertexInputBindings.emplace_back(
        1, sizeof(glm::mat4), VK_VERTEX_INPUT_RATE_INSTANCE);

    pipelineCiPack.vertexInputAttributes.emplace_back(
        2, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 0);
    pipelineCiPack.vertexInputAttributes.emplace_back(
        3, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 1);
    pipelineCiPack.vertexInputAttributes.emplace_back(
        4, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 2);
    pipelineCiPack.vertexInputAttributes.emplace_back(
        5, 1, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec4) * 3);

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
    pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_3d;

    pipeline.Create(pipelineCiPack);
  };
  auto Destroy = [&] { pipeline.~pipeline(); };
  graphic::Singleton().AddCreateSwapchainCallback(Create);
  graphic::Singleton().AddDestroySwapchainCallback(Destroy);
  Create();
}

void window::run() {

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

#ifndef NDEBUG
        // printf("[ window ] DEBUG: glfwSetCursorPosCallback triggered: "
        //        "offset(%.2f,%.2f)\n",
        //        xoffset, yoffset);
        // printf("[ window ] DEBUG: camera(yaw: %.2f, pitch: %.2f)\n",
        //        camera::Singleton().yaw, camera::Singleton().pitch);
        // printf("[ window ] DEBUG: camera(front(%.2f,%.2f,%.2f), "
        //        "right(%.2f,%.2f,%.2f), up(%.2f,%.2f,%.2f))\n",
        //        camera::Singleton().front.x, camera::Singleton().front.y,
        //        camera::Singleton().front.z, camera::Singleton().right.x,
        //        camera::Singleton().right.y, camera::Singleton().right.z,
        //        camera::Singleton().up.x, camera::Singleton().up.y,
        //        camera::Singleton().up.z);
#endif
          }
        });
  };

  registerLogicUpdateCallback();
  registerGLFWCallback();

  vertex vertices[] = {
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

      // Left face (CW order)
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
      {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}},

      // Right face (CW order)
      {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}},
      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}},

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
  };

  glm::mat4 models[10][10][10];
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      for (int k = 0; k < 10; k++) {
        models[i][j][k] = glm::translate(glm::mat4(1.f),
                                         glm::vec3(i * 2.f, j * 2.f, k * 2.f));
      }
    }
  }

  uint16_t indices[] = {
      0,  1,  2,  2,  3,  0,  // Front face
      4,  5,  6,  6,  7,  4,  // Back face
      8,  9,  10, 10, 11, 8,  // Left face
      12, 13, 14, 14, 15, 12, // Right face
      16, 17, 18, 18, 19, 16, // Top face
      20, 21, 22, 22, 23, 20, // Bottom face
  };

  vulkanWrapper::vertexBuffer verticesBuffer(sizeof vertices);
  verticesBuffer.TransferData(vertices, sizeof vertices);

  vulkanWrapper::vertexBuffer instanceBuffer(sizeof(glm::mat4) * 1000);
  instanceBuffer.TransferData(models, sizeof models);

  vulkanWrapper::indexBuffer indexBuffer(sizeof indices);
  indexBuffer.TransferData(indices, sizeof indices);

  vulkanWrapper::uniformBuffer ubo_mvp(sizeof(glm::mat4) * 3);

  vulkanWrapper::pipelineLayout layout_cube;
  vulkanWrapper::pipeline pipeline_cube;

  vulkanWrapper::descriptorSetLayout descriptorSetLayout_3dMVP;

  const renderPassWithFramebuffers &rpwf = RenderPassAndFramebuffers();
  const vulkanWrapper::renderPass &renderPass = rpwf.renderPass;
  const std::vector<vulkanWrapper::framebuffer> &framebuffers =
      rpwf.framebuffers;

  CreateLayout(layout_cube, descriptorSetLayout_3dMVP);
  CreatePipeline(pipeline_cube, layout_cube);

  std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};
  vulkanWrapper::descriptorPool descriptorPool(1, descriptorPoolSizes);
  vulkanWrapper::descriptorSet descriptorSet_3dMVP;

  std::vector<VkDescriptorSet> descriptorSets = {descriptorSet_3dMVP};
  std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
      descriptorSetLayout_3dMVP};
  descriptorPool.AllocateSets(descriptorSets, descriptorSetLayouts);
  descriptorSet_3dMVP = descriptorSets[0];

  VkDescriptorBufferInfo bufferInfo = {
      .buffer = ubo_mvp,
      .offset = 0,
      .range = sizeof(MVP) // 或VK_WHOLE_SIZE
  };
  std::vector<VkDescriptorBufferInfo> bufferInfos = {bufferInfo};
  descriptorSet_3dMVP.Write(bufferInfos, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

  vulkanWrapper::fence fence(VK_FENCE_CREATE_SIGNALED_BIT);
  vulkanWrapper::semaphore semaphore_imageAvailable;
  vulkanWrapper::semaphore semaphore_renderFinished;

  vulkanWrapper::commandBuffer commandBuffer;
  vulkanWrapper::commandPool commandPool(
      graphic::Singleton().QueueFamilyIndex_Graphics(),
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  commandPool.AllocateBuffers(commandBuffer);

  auto color = color::floatRGBA(0, 0, 0, 255);
  VkClearValue clearColor = {.color = {std::get<0>(color), std::get<1>(color),
                                       std::get<2>(color),
                                       std::get<3>(color)}}; // 灰色
  std::vector<VkClearValue> clearValues = {clearColor};

  int frame = 0;
  fence.Reset();
  while (!glfwWindowShouldClose(glfwWindow)) {
    while (glfwGetWindowAttrib(glfwWindow, GLFW_ICONIFIED)) {
      glfwWaitEvents();
    }

    frame++;
    frame %= 90;
    graphic::Singleton().SwapImage(semaphore_imageAvailable);
    uint32_t i = graphic::Singleton().CurrentImageIndex();

    commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    renderPass.CmdBegin(commandBuffer, framebuffers[i],
                        {{}, {framebuffers[i].Size()}}, clearValues,
                        VK_SUBPASS_CONTENTS_INLINE);

    MVP mvp;
    mvp.model = glm::mat4(1.0f);
    mvp.view = camera::Singleton().getViewMatrix();
    mvp.projection = camera::Singleton().getProjectionMatrix(
        currentSize.width, currentSize.height);
    // TransferData
    verticesBuffer.TransferData(vertices, sizeof vertices);
    ubo_mvp.TransferData(&mvp, sizeof(MVP));

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline_cube);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, verticesBuffer.Address(),
                           &offset);
    vkCmdBindVertexBuffers(commandBuffer, 1, 1, instanceBuffer.Address(),
                           &offset);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            layout_cube, 0, 1, descriptorSet_3dMVP.Address(), 0,
                            nullptr);
    vkCmdDrawIndexed(commandBuffer, 36, 1000, 0, 0, 0);

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
