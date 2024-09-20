# learnVulkan

My implementation ~copy & paste~ of [EasyVulkan](https://easyvulkan.github.io/) tutorial

## Installation

1. Compile program

```bash
conan install . --build=missing --output=build
cmake --preset conan-release
cmake --build build
```

2. Compile shader in src/Shader

```bash
glslc shader.vert -o shader.vert.spv
glslc shader.frag -o shader.frag.spv
```

Or use the script

```bash
./genSpv.sh
```

## Run

```bash
./build/learnVulkan
```
