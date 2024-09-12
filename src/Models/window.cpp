#define GLFW_INCLUDE_VULKAN

#ifndef LIMIT_FRAME_RATE
#define LIMIT_FRAME_RATE true
#endif

#include "window.hpp"
#include "../Utils/VkResultThrowable.hpp"
#include "graphic.hpp"

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
      });

  glfwSetWindowIconifyCallback(glfwWindow, [](GLFWwindow *window, int iconify) {
    class window *self = (class window *)glfwGetWindowUserPointer(window);
    self->iconified = iconify;
  });

  while (!glfwWindowShouldClose(glfwWindow)) {
    glfwPollEvents();
    updateLogic();

    // if (glfwGetKey(glfwWindow, GLFW_KEY_F10) == GLFW_PRESS) {
    //   if (glfwGetWindowMonitor(glfwWindow) == nullptr) {
    //     MakeWindowFullScreen();
    //   } else {
    //     MakeWindowWindowed(position, size);
    //   }
    // }
  }
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

} // namespace learnVulkan::models
