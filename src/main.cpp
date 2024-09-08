#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <vulkan/vulkan.h>

GLFWwindow *window;

void initialize() {
  // glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

  glfwInit();
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  window = glfwCreateWindow(800, 600, "Learn Vulkan", nullptr, nullptr);
}

void destroy() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

int main(int argc, char *argv[]) {
  initialize();

  if (window == NULL) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  std::this_thread::sleep_for(std::chrono::seconds(5));

  destroy();
  return 0;
}
