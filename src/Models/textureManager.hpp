#include "Vulkan/dynamicTexture2d.hpp"
#include "Vulkan/texture2d.hpp"

#include <cstdint>
#include <unordered_map>

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

  std::tuple<uint64_t, vulkanWrapper::texture2d *>
  registerTexture2d(const char *path);

  std::tuple<uint64_t, vulkanWrapper::dynamicTexture2d *>
  registerDynamicTexture2d(const char *path);

  inline uint64_t Index() const { return index; }

  void UpdateDynamicTextureView();

  void DestroyAllTexture();

  inline static textureManager &Singleton() {
    static textureManager instance;
    return instance;
  }
};
