#define GLFW_INCLUDE_VULKAN
#define VK_RESULT_THROW

#include "Models/window.hpp"

void destroy() {
  // vkDestroySurfaceKHR(instance, surface, nullptr);
  glfwTerminate();
}

int main(int argc, char *argv[]) {
#ifndef NDEBUG
  printf("Debug mode\n");
#else
  printf("Release mode\n");
#endif

  learnVulkan::window window;

  window.windowTitle = "Learn Vulkan";
  if (!window.initialize()) {
    return 1;
  }

  window.run();

  glfwTerminate();

  return 0;
}
