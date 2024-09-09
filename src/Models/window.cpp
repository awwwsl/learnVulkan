#pragma once

#define GLFW_INCLUDE_VULKAN

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <GLFW/glfw3.h>

#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <thread>
#include <vector>

#include "graphic.cpp"

namespace learnVulkan::models {

class window {
  struct CallbackInfo {
    std::function<void(int framePassed, double timePassed)> callback;
    std::chrono::duration<double> interval;
    double lastTime;
    int framePassed;
  };
  static inline std::vector<CallbackInfo> callbacks;

private:
  GLFWwindow *glfwWindow;
  GLFWmonitor *glfwMonitor;

  VkOffset2D position = {0, 0};
  VkExtent2D size = {1920, 1080};

  void updatePerPeriod(
      std::chrono::duration<double> interval,
      std::function<void(int framePassed, double timePassed)> callback) {
    callbacks.push_back({callback, interval, glfwGetTime(), 0});
  }

  void updateLogic() {
    for (int i = 0; i < callbacks.size(); i++) {
      auto &info = callbacks[i];
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

  void MakeWindowFullScreen() {
    const GLFWvidmode *mode = glfwGetVideoMode(glfwMonitor);
    glfwSetWindowMonitor(glfwWindow, glfwMonitor, 0, 0, mode->width,
                         mode->height, mode->refreshRate);
  }
  void MakeWindowWindowed(VkOffset2D position, VkExtent2D size) {
    const GLFWvidmode *pMode = glfwGetVideoMode(glfwMonitor);
    glfwSetWindowMonitor(glfwWindow, nullptr, position.x, position.y,
                         size.width, size.height, pMode->refreshRate);
  }
  std::function<void(int framePassed, double timePassed)> callback;

public:
  window() {}

  bool initialize() {
    // glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

    if (!glfwInit()) {
      printf("[ window ] Failed to initialize GLFW\n");
      return false;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *glfwMonitorMode = glfwGetVideoMode(glfwMonitor);
    glfwWindow = glfwCreateWindow(size.width, size.height, windowTitle.c_str(),
                                  nullptr, nullptr);
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
    graphicsBase::Singleton().AddDeviceExtension(
        VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    graphicsBase::Singleton().UseLatestApiVersion();
    if (graphicsBase::Singleton().CreateInstance()) {
      printf("[ window ] Failed to create Vulkan instance\n");
      return false;
    }

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (VkResult result =
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

    return true;
  }
  std::string windowTitle = "Untitled";

  void run() {
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
        position.y += 10 * ((speeding) ? 5 : 1);
        printf("Down\n");
        MakeWindowWindowed(position, size);
      }
      if (glfwGetKey(glfwWindow, GLFW_KEY_UP) == GLFW_PRESS) {
        position.y -= 10 * ((speeding) ? 5 : 1);
        printf("Up\n");
        MakeWindowWindowed(position, size);
      }
      if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
        position.x -= 10 * ((speeding) ? 5 : 1);
        printf("Left\n");
        MakeWindowWindowed(position, size);
      }
      if (glfwGetKey(glfwWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        position.x += 10 * ((speeding) ? 5 : 1);
        printf("Right\n");
        MakeWindowWindowed(position, size);
      }
    });

    updatePerPeriod(std::chrono::milliseconds(1000), [this](int, double) {
      printf("Position: (%d, %d)\n", position.x, position.y);
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
  }
};

} // namespace learnVulkan::models
