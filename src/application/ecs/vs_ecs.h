//
// Created by vstok on 2022-02-18.
//

#pragma once

#include "vs_component.h"
#include "vs_entity.h"
#include <unordered_map>

namespace vs {
enum class vs_component_type : uint8_t {
  MODEL_COMPONENT = 0,
  MOVEMENT_COMPONENT = 1,
  TEXTURE_COMPONENT = 2,
  TRANSFORM_COMPONENT = 3,
  COLOR_COMPONENT = 4,
  POINT_LIGHT_COMPONENT = 5,
  RIGID_BODY_COMPONENT = 6
};
class vs_ecs {
  using id_t = unsigned int;

public:
  vs_ecs *getInstance() {
    if (!instance)
      instance = new vs_ecs();
    return instance;
  }
  /// @brief Creates a new component of type and adds it to entity with id.
  /// @returns the new components id
  ///@param entity_id=id of the entity to add the component to.
  ///@param type=component type to create.
  id_t addComponentToEntity(id_t entity_id, vs_component_type type);
  id_t createComponent(id_t id, vs_component_type type);

private:
  vs_ecs *instance;

  vs_ecs() { instance = this; }

  std::unordered_map<id_t, vs_entity> entities;
//  std::unordered_map<id_t, vs_component> components;
};
} // namespace vs
