#pragma once

#include "Vulkan/dynamicTexture2d.hpp"
#include "Vulkan/gameTexture.hpp"
#include "Vulkan/texture2d.hpp"

#include <cstdint>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

class textureManager {
  textureManager();

  void operator=(textureManager const &) = delete;
  textureManager(textureManager const &) = delete;
  textureManager(textureManager const &&) = delete;

  uint64_t index = 0;

  struct hash_uint64_t {
    std::size_t operator()(const uint64_t &k) const {
      return std::hash<uint64_t>()(k);
    }
  };

public:
  std::unordered_map<uint64_t, vulkanWrapper::texture2d *, hash_uint64_t>
      staticTextures2d;
  std::unordered_map<uint64_t, vulkanWrapper::dynamicTexture2d *, hash_uint64_t>
      dynamicTextures2d;
  std::unordered_map<std::string, uint64_t> textureNameMap;

  inline std::tuple<uint64_t, vulkanWrapper::gameTexture>
  registerGameTexture(const std::string identifier) {
    const std::string path = "res/vulkanCraft/texture/" + identifier + ".png";
    VkExtent2D extent;
    formatInfo formatInfo = formatInfo::FormatInfo(
        VK_FORMAT_R8G8B8A8_UNORM); // 根据指定的format_initial取得格式信息
    std::unique_ptr<uint8_t[]> pImageData =
        imageOperation::LoadFile_FileSystem(path.c_str(), extent, formatInfo);
    if (!pImageData) {
      throw std::runtime_error("Failed to load texture image!");
    }
    uint64_t thisIndex = index;
    index++;
    textureNameMap.emplace(path, thisIndex);
    if (extent.height % extent.width == 0 &&
        extent.height / extent.width > 0) { // is dynamic
      vulkanWrapper::dynamicTexture2d *pTexture =
          new vulkanWrapper::dynamicTexture2d(
              path.c_str(), VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
              true, VK_FILTER_LINEAR);
      dynamicTextures2d.emplace(thisIndex, pTexture);
      return std::make_tuple(thisIndex, vulkanWrapper::gameTexture(thisIndex));
    } else { // is static
      vulkanWrapper::texture2d *pTexture = new vulkanWrapper::texture2d(
          path.c_str(), VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,
          true, VK_FILTER_LINEAR);
      staticTextures2d.emplace(thisIndex, pTexture);
      return std::make_tuple(thisIndex, vulkanWrapper::gameTexture(thisIndex));
    }
  }

  inline uint64_t Index() const { return index; }

  void UpdateDynamicTextureView();

  void DestroyAllTexture();

  inline static textureManager &Singleton() {
    static textureManager instance;
    return instance;
  }
};
