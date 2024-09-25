#define GLFW_INCLUDE_VULKAN
#define VK_RESULT_THROW

#include "Models/window.hpp"

#include <GLFW/glfw3.h>
#include <stdexcept>

void destroy() {
  // vkDestroySurfaceKHR(instance, surface, nullptr);
  glfwTerminate();
}

int main(int argc, char *argv[]) {
#ifndef NDEBUG
  printf("Debug mode\n");
  glfwSetErrorCallback([](int error, const char *description) {
    printf("[ GLFW ] Error: %s\n", description);
  });
#else
  printf("Release mode\n");
#endif

  try {
    learnVulkan::window window;

    window.windowTitle = "Learn Vulkan";
    if (!window.initialize()) {
      return 1;
    }
    window.run();
  } catch (const std::runtime_error &ex) {
    printf("%s", ex.what());
  }

  glfwTerminate();

  return 0;
}
