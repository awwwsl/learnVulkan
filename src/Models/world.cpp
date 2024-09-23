#include "world.hpp"

#include <unordered_map>

world::world() = default;

block *world::getEntity(glm::ivec3 position) {
  if (entities.find(position) != entities.end()) {
    return &entities.find(position)->second;
  }
  return nullptr;
}

void world::setEntity(glm::ivec3 position, block e) {
  entities.emplace(position, e);
}
