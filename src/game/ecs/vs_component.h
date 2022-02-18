//
// Created by vstok on 2022-02-18.
//

#pragma once
#include "engine/renderer/vs_utils.h"
//lib
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
//std
#include <unordered_map>


namespace vs {
struct vs_component {
  using id_t = unsigned int;
  using map = std::unordered_map<id_t, vs_component>;

  id_t entity_id;
};

} // namespace vs

namespace std {
template <> struct hash<vs::vs_component> {
  std::size_t operator()(vs::vs_component const &component) const {
    std::size_t seed = 0;
    vs::hash_combine(seed, component.entity_id);
    return seed;
  }
};
} // namespace std