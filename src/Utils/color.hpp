#pragma once

#include <tuple>

#include <glm/glm.hpp>

class color {
public:
  inline static const constexpr std::tuple<float, float, float>
  floatRGB(const int r, const int g, const int b) {
    return std::make_tuple(r / 255.0f, g / 255.0f, b / 255.0f);
  }

  inline static const constexpr std::tuple<float, float, float, float>
  floatRGBA(const int r, const int g, const int b, const int a) {
    return std::make_tuple(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
  }

  inline static const constexpr std::tuple<float, float, float>
  floatRGB(const std::tuple<int, int, int> rgb) {
    return std::make_tuple(std::get<0>(rgb) / 255.0f, std::get<1>(rgb) / 255.0f,
                           std::get<2>(rgb) / 255.0f);
  }

  inline static const constexpr std::tuple<float, float, float, float>
  floatRGBA(const std::tuple<int, int, int, int> rgba) {
    return std::make_tuple(
        std::get<0>(rgba) / 255.0f, std::get<1>(rgba) / 255.0f,
        std::get<2>(rgba) / 255.0f, std::get<3>(rgba) / 255.0f);
  }
};
