#include "gameTexture.hpp"

#include "../Models/textureManager.hpp"

void vulkanWrapper::gameTexture::updateView() {
  if (auto it = textureManager::Singleton().dynamicTextures2d.find(index);
      it != textureManager::Singleton().dynamicTextures2d.end())
    it->second->UpdateView();
}

VkImageView vulkanWrapper::gameTexture::view() {
  if (auto it = textureManager::Singleton().staticTextures2d.find(index);
      it != textureManager::Singleton().staticTextures2d.end())
    return it->second->ImageView();
  if (auto it = textureManager::Singleton().dynamicTextures2d.find(index);
      it != textureManager::Singleton().dynamicTextures2d.end())
    return it->second->CurrentView();
  throw std::runtime_error("[ gameTexture ] ERROR: Texture not found");
}
