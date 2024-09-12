#include <vector>

namespace learnVulkan::models {

struct Vertex {
  float position[3];
  float color[3];
};

std::vector<Vertex> vertexBuffer = {
    {{+0.0f, -0.5f, +0.0f}, {+1.0f, +0.0f, +0.0f}},
    {{+0.5f, +0.5f, +0.0f}, {+0.0f, +1.0f, +0.0f}},
    {{-0.5f, +0.5f, +0.0f}, {+0.0f, +0.0f, +1.0f}},
};

std::vector<int> indexBuffer = {0, 1, 2};

} // namespace learnVulkan::models
