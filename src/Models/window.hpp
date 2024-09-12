#define GLFW_INCLUDE_VULKAN

#include "../Utils/VkResultThrowable.hpp"
#include "graphic.hpp"

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

namespace learnVulkan::models {

class window {
  struct LogicUpdateCallbackInfo {
    std::function<void(int framePassed, double timePassed)> callback;
    std::chrono::duration<double> interval;
    double lastTime;
    int framePassed;
  };
  static inline std::vector<LogicUpdateCallbackInfo> logicUpdateCallbacks;

private:
  GLFWwindow *glfwWindow;
  GLFWmonitor *glfwMonitor;

  VkOffset2D currentPosition;
  VkExtent2D currentSize;

  bool iconified = false;

  inline void updatePerPeriod(
      std::chrono::duration<double> interval,
      std::function<void(int framePassed, double timePassed)> callback) {
    logicUpdateCallbacks.push_back({callback, interval, glfwGetTime(), 0});
  }

  void updateLogic();

  inline void MakeWindowFullScreen() {
    const GLFWvidmode *mode = glfwGetVideoMode(glfwMonitor);
    glfwSetWindowMonitor(glfwWindow, glfwMonitor, 0, 0, mode->width,
                         mode->height, mode->refreshRate);
  }
  inline void MakeWindowWindowed(VkOffset2D position, VkExtent2D size) {
    const GLFWvidmode *pMode = glfwGetVideoMode(glfwMonitor);
    glfwSetWindowMonitor(glfwWindow, nullptr, position.x, position.y,
                         size.width, size.height, pMode->refreshRate);
  }
  std::function<void(int framePassed, double timePassed)> callback;

public:
  window();

  std::string windowTitle = "Untitled";

  const constexpr static VkExtent2D defaultSize = {1920, 1080};

  bool initialize();
  void run();
};

} // namespace learnVulkan::models
