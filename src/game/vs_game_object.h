#pragma once
#include "vs_model_component.h"

// std
#include <memory>
#include <unordered_map>
// libs
#include <glm/gtc/matrix_transform.hpp>
#include <optional>

namespace vs {
struct transform_component {
  glm::vec3 translation{};
  glm::vec3 scale{1.f, 1.f, 1.f};
  glm::vec3 rotation;

  // Matrix corresponds to Translate * Ry * Rx * Rz * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4();

  glm::mat3 normal_matrix();
};

struct rigid_body_component {
  glm::vec3 velocity;
  float mass{1.0f};
};

struct point_light_component {
  float light_intensity = 1.0f;
};

class vs_game_object {
public:
  using id_t = unsigned int;
  using map = std::unordered_map<id_t, vs_game_object>;



  vs_game_object &operator=(vs_game_object &&) = default;
  vs_game_object(vs_game_object &&) = default;
  vs_game_object(const vs_game_object &) = delete;
  vs_game_object &operator=(const vs_game_object &) = delete;

  id_t getId() const { return id_; }

  static vs_game_object createPointLight(float intensity = 10.f,
                                       float radius = 0.1f,
                                       glm::vec3 color = glm::vec3(1.f));

  static vs_game_object createGameObject() {
    static id_t current_id = 0;
    return vs_game_object{current_id++};
  }



  glm::vec3 color{};
  transform_component transform_comp{};

  // optional pointer components
  std::shared_ptr<vs_model_component> model_comp{};
  std::shared_ptr<point_light_component> point_light_comp;
  std::shared_ptr<rigid_body_component> rigid_body_comp{};

private:
  static vs_game_object createPhysicsObject(glm::vec3 position = glm::vec3(0.f), glm::vec3 velocity= glm::vec3(0.f));
  vs_game_object(id_t obj_id) : id_(obj_id){};

  const id_t id_;
};
} // namespace vs
