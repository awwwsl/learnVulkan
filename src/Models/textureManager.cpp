#include "textureManager.hpp"
#include "Vulkan/dynamicTexture2d.hpp"
#include "Vulkan/texture2d.hpp"

#include <cstdint>
#include <tuple>
#include <vulkan/vulkan_core.h>

textureManager::textureManager() = default;

void textureManager::UpdateDynamicTextureView() {
  for (auto &dynamicTexture : dynamicTextures2d) {
    dynamicTexture.second->UpdateView();
  }
}

void textureManager::DestroyAllTexture() {
  for (auto &staticTexture : staticTextures2d) {
    staticTexture.second->~texture2d();
  }
  for (auto &dynamicTexture : dynamicTextures2d) {
    dynamicTexture.second->~dynamicTexture2d();
  }
  staticTextures2d.clear();
  dynamicTextures2d.clear();
}
