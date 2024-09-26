#include "textureManager.hpp"
#include "Vulkan/dynamicTexture2d.hpp"
#include "Vulkan/texture2d.hpp"

#include <cstdint>
#include <tuple>
#include <vulkan/vulkan_core.h>

textureManager::textureManager() = default;

std::tuple<uint64_t, vulkanWrapper::texture2d *>
textureManager::registerTexture2d(const char *path) {
  uint64_t thisIndex = index;
  index++;
  vulkanWrapper::texture2d *pTexture = new vulkanWrapper::texture2d(
      path, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, true,
      VK_FILTER_LINEAR);
  staticTextures2d.emplace(thisIndex, pTexture);
  return std::make_tuple(thisIndex, pTexture);
}

std::tuple<uint64_t, vulkanWrapper::dynamicTexture2d *>
textureManager::registerDynamicTexture2d(const char *path) {
  uint64_t thisIndex = index;
  index++;
  vulkanWrapper::dynamicTexture2d *pTexture =
      new vulkanWrapper::dynamicTexture2d(path, VK_FORMAT_R8G8B8A8_UNORM,
                                          VK_FORMAT_R8G8B8A8_UNORM, true,
                                          VK_FILTER_LINEAR);
  dynamicTextures2d.emplace(thisIndex, pTexture);
  return std::make_tuple(thisIndex, pTexture);
}

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
