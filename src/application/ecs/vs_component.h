//
// Created by vstok on 2022-02-18.
//

#pragma once
#include "engine/renderer/vs_utils.h"
#include "vs_ecs.h"
// lib
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
// std
#include <unordered_map>

namespace vs {
struct vs_component {
  using id_t = unsigned int;
  using map = std::unordered_map<id_t, vs_component>;

  virtual id_t createComponent() =0;

  // copy constructors
  vs_component &operator=(vs_component &&) = default;
  vs_component(vs_component &&) = default;
  vs_component(const vs_component &) = delete;
  vs_component &operator=(const vs_component &) = delete;

  id_t getId() const { return id_; };
  id_t getOwner() const { return entity_id_; }

private:
  explicit vs_component(id_t ent_id) : id_(ent_id){};
  id_t entity_id_;
  id_t id_;
  uint16_t version;
};

} // namespace vs

namespace std {
template <> struct hash<vs::vs_component> {
  std::size_t operator()(vs::vs_component const &component) const {
    std::size_t seed = 0;
    vs::hash_combine(seed, component.getId());
    return seed;
  }
};
} // namespace std