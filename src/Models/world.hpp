#pragma once

#include "block.hpp"
#include "instance.hpp"
#include "textureManager.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

struct glm_ivec3_hash {
  template <typename T> std::size_t operator()(const T &position) const {
    return std::hash<int64_t>()(position.x) ^ std::hash<int64_t>()(position.y) ^
           std::hash<int64_t>()(position.z);
  }
};

class world {

  static const constexpr glm::ivec3 initializeWorldSize = {16, 16, 16};

public:
  world();

  std::unordered_map<glm::ivec3, block, glm_ivec3_hash> entities;

  inline block *getEntity(int64_t x, int64_t y, int64_t z) {
    return getEntity(glm::ivec3(x, y, z));
  }
  block *getEntity(glm::ivec3 position);

  inline void setEntity(int64_t x, int64_t y, int64_t z, block e) {
    return setEntity(glm::ivec3(x, y, z), e);
  }
  void setEntity(glm::ivec3 position, block e);

  inline size_t removeEntity(int64_t x, int64_t y, int64_t z) {
    return removeEntity(glm::ivec3(x, y, z));
  }
  size_t removeEntity(glm::ivec3 position);

  inline void initializeWorld(uint32_t textureIndex) {
    int64_t x = initializeWorldSize.x, y = initializeWorldSize.y,
            z = initializeWorldSize.z;
    for (int64_t ix = 0; ix < x; ix++) {
      for (int64_t iy = 0; iy < y; iy++) {
        for (int64_t iz = 0; iz < z; iz++) {
          entities.emplace(glm::ivec3(ix, iy, iz),
                           block(glm::ivec3(ix, iy, iz), textureIndex));
        }
      }
    }
  }

  inline std::vector<glm::mat4> getModelMatrics() {
    std::vector<glm::mat4> models(entities.size());
    for (auto &entity : entities) {
      models.push_back(entity.second.getModelMatrix());
    }
    return models;
  }

  inline std::vector<instance> getInstances() {
    std::vector<instance> instances;
    instances.reserve(entities.size());
    for (auto &entity : entities) {
      struct instance inst;
      inst.model = entity.second.getModelMatrix();
      inst.textureIndex = entity.second.textureIndex;
      instances.push_back(inst);
    }
    return instances;
  }
};
