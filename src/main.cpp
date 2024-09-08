#define GLFW_INCLUDE_VULKAN

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <GLFW/glfw3.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <thread>
#include <vector>

#include "Models/models.cpp"
#include "Models/window.cpp"

struct CallbackInfo {
  void (*callback)(int framePassed, double timePassed);
  std::chrono::duration<double> interval;
  double lastTime;
  int framePassed;
};
static inline std::vector<CallbackInfo> callbacks;

void destroy() {
  // vkDestroySurfaceKHR(instance, surface, nullptr);
  glfwTerminate();
}

int main(int argc, char *argv[]) {

  learnVulkan::models::window window;

  window.windowTitle = "Learn Vulkan yee";
  window.initialize();

  window.run();

  destroy();
  return 0;
}
