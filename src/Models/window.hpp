#pragma once

#define GLFW_INCLUDE_VULKAN

#include "instance.hpp"
#include "world.hpp"

#include <GLFW/glfw3.h>

#include <chrono>
#include <functional>
#include <string>

namespace learnVulkan {

class window {
  struct LogicUpdateCallbackInfo {
    std::function<void(int framePassed, double timePassed)> callback;
    std::chrono::duration<double> interval;
    double lastTime;
    int framePassed;
  };
  static inline std::vector<LogicUpdateCallbackInfo> logicUpdateCallbacks;

private:
  world worldInstance;

  GLFWwindow *glfwWindow;
  GLFWmonitor *glfwMonitor;

  bool iconified = false;

  block *rayIntersection(const glm::vec3 start, const glm::vec3 direction,
                         const float maxDistance = 5.0f, int *facing = nullptr);

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
  inline void TerminateWindow();

  std::function<void(int framePassed, double timePassed)> callback;

public:
  window();

  std::string windowTitle = "Untitled";

  const constexpr static VkExtent2D defaultSize = {1920, 1080};

  VkOffset2D currentPosition;
  VkExtent2D currentSize;

  bool initialize();
  void run();
};

} // namespace learnVulkan
