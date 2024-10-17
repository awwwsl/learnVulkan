#include "chunk.hpp"
#include "Vulkan/storageBuffer.hpp"
#include <glm/fwd.hpp>
static uint32_t instanceBufferIndex;

chunk::chunk(glm::ivec3 chunkPosition) : chunkPosition(chunkPosition) {
  instanceBlocks = new block *[chunkSize.x * chunkSize.y * chunkSize.z];
  for (int32_t i = 0; i < chunkSize.x * chunkSize.y * chunkSize.z; i++) {
    instanceBlocks[i] = nullptr; // HACK: could optimize
  }
  this->instanceBuffer = new vulkanWrapper::storageBuffer(
      sizeof(instance) * chunkSize.x * chunkSize.y * chunkSize.z);
#ifndef NDEBUG
  printf("[ chunk ] DEBUG: chunk created at (%d, %d, %d)\n", chunkPosition.x,
         chunkPosition.y, chunkPosition.z);
#endif
}

chunk::~chunk() {
  if (instanceBuffer != VK_NULL_HANDLE) {
    instanceBuffer->~storageBuffer();
  }
  for (int64_t i = 0; i < chunkSize.x * chunkSize.y * chunkSize.z; i++) {
    if (instanceBlocks[i] != nullptr) {
      delete instanceBlocks[i];
    }
  }
  delete[] instanceBlocks;
#ifndef NDEBUG
  printf("[ chunk ] DEBUG: chunk destroyed at (%d, %d, %d,)\n", chunkPosition.x,
         chunkPosition.y, chunkPosition.z);
#endif
}

block *chunk::getBlock(glm::ivec3 position) {
  if (position.x < 0 || position.y < 0 || position.z < 0 ||
      position.x >= chunkSize.x || position.y >= chunkSize.y ||
      position.z >= chunkSize.z) {
#ifndef NDEBUG
    printf("[ chunk ] ERROR: getBlock(position: %d, %d, %d) out of bounds\n",
           position.x, position.y, position.z);
#endif
    return nullptr;
  }
  return instanceBlocks[position.x * chunkSize.y * chunkSize.z +
                        position.y * chunkSize.z + position.z];
}

void chunk::setBlock(glm::ivec3 position, block *e) {
  if (position.x < 0 || position.y < 0 || position.z < 0 ||
      position.x >= chunkSize.x || position.y >= chunkSize.y ||
      position.z >= chunkSize.z) {
#ifndef NDEBUG
    printf("[ chunk ] ERROR: setBlock(position) out of bounds\n");
#endif
    return;
  }
  if (instanceBlocks[position.x * chunkSize.y * chunkSize.z +
                     position.y * chunkSize.z + position.z] != nullptr) {
    delete instanceBlocks[position.x * chunkSize.y * chunkSize.z +
                          position.y * chunkSize.z + position.z];
    blockCount--;
  }
  instanceBlocks[position.x * chunkSize.y * chunkSize.z +
                 position.y * chunkSize.z + position.z] = e;
#ifndef NDEBUG
  printf("[ chunk ] DEBUG: setBlock at localPosition(%d, %d, %d) chunk(%d, %d, "
         "%d)\n",
         position.x, position.y, position.z, chunkPosition.x, chunkPosition.y,
         chunkPosition.z);
#endif

  altered = true;
}

void chunk::removeBlock(glm::ivec3 position) {
  if (position.x < 0 || position.y < 0 || position.z < 0 ||
      position.x >= chunkSize.x || position.y >= chunkSize.y ||
      position.z >= chunkSize.z) {
#ifndef NDEBUG
    printf("[ chunk ] ERROR: removeBlock(position) out of bounds\n");
#endif
    return;
  }
  if (instanceBlocks[position.x * chunkSize.y * chunkSize.z +
                     position.y * chunkSize.z + position.z] == nullptr) {
#ifndef NDEBUG
    printf("[ chunk ] ERROR: removeBlock(position) no block at there\n");
#endif
    return;
  }
  delete instanceBlocks[position.x * chunkSize.y * chunkSize.z +
                        position.y * chunkSize.z + position.z];
  instanceBlocks[position.x * chunkSize.y * chunkSize.z +
                 position.y * chunkSize.z + position.z] = nullptr;
  blockCount--;
  altered = true;
}

bool NotInFrustum(glm::ivec3 chunkPosition, camera &cam) {
  std::array<glm::vec4, 6> frustumPlanes = cam.frustumPlanes();

  chunkPosition *= chunk::chunkSize;
  for (uint32_t i = 0; i < 6; i++) {
    if (glm::dot(frustumPlanes[i], glm::vec4(chunkPosition.x, chunkPosition.y,
                                             chunkPosition.z, 1.0f)) < 0.f &&
        glm::dot(frustumPlanes[i],
                 glm::vec4(chunkPosition.x + chunk::chunkSize.x,
                           chunkPosition.y, chunkPosition.z, 1.0f)) < 0.f &&
        glm::dot(frustumPlanes[i],
                 glm::vec4(chunkPosition.x,
                           chunkPosition.y + chunk::chunkSize.y,
                           chunkPosition.z, 1.0f)) < 0.f &&
        glm::dot(frustumPlanes[i],
                 glm::vec4(chunkPosition.x, chunkPosition.y,
                           chunkPosition.z + chunk::chunkSize.z, 1.0f)) < 0.f &&
        glm::dot(frustumPlanes[i],
                 /**/ glm::vec4(chunkPosition.x + chunk::chunkSize.x,
                                chunkPosition.y + chunk::chunkSize.y,
                                chunkPosition.z, 1.0f)) < 0.f &&
        glm::dot(frustumPlanes[i],
                 glm::vec4(chunkPosition.x,
                           chunkPosition.y + chunk::chunkSize.y,
                           chunkPosition.z + chunk::chunkSize.z, 1.0f)) < 0.f &&
        glm::dot(frustumPlanes[i],
                 glm::vec4(chunkPosition.x + chunk::chunkSize.x,
                           chunkPosition.y,
                           chunkPosition.z + chunk::chunkSize.z, 1.0f)) < 0.f &&
        glm::dot(frustumPlanes[i],
                 glm::vec4(chunkPosition.x + chunk::chunkSize.x,
                           chunkPosition.y + chunk::chunkSize.y,
                           chunkPosition.z + chunk::chunkSize.z, 1.0f)) < 0.f) {
      return true;
    }
  }
  return false;
}
bool chunk::needRender(camera &cam) {
  if (!renderEnabled)
    return false;
  // if chunk is too far
  if (chunkPosition.x * chunkSize.x + chunkSize.x <
          cam.position.x - cam.farPlane ||
      chunkPosition.y * chunkSize.y + chunkSize.y <
          cam.position.y - cam.farPlane ||
      chunkPosition.z * chunkSize.z + chunkSize.z <
          cam.position.z - cam.farPlane ||
      chunkPosition.x * chunkSize.x > cam.position.x + cam.farPlane ||
      chunkPosition.y * chunkSize.y > cam.position.y + cam.farPlane ||
      chunkPosition.z * chunkSize.z > cam.position.z + cam.farPlane) {
    return false;
  }
  // Frustum culling
  if (NotInFrustum(chunkPosition, cam)) {
    return false;
  }

  return true;
}
