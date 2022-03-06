//
// Created by vstok on 2022-02-21.
//

#pragma once

#include "vs_component.h"
namespace vs {

struct vs_transform_component : vs_component {
  glm::vec3 position{};
  glm::vec3 scale{1.f, 1.f, 1.f};
  glm::vec3 rotation{};

 id_t createComponent() override;

  // Matrix corresponds to Translate * Ry * Rx * Rz * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4();

  glm::mat3 normal_matrix();

};
} // namespace vs
