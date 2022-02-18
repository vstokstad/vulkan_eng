//
// Created by vstok on 2022-02-18.
//
#pragma once
#include "vs_component.h"

struct vs_entity {
  static vs_entity createEntity() {
    static id_t current_id = 0;
    return vs_entity{current_id++};
  }
  using id_t = unsigned int;
  vs_entity &operator=(vs_entity &&) = default;
  vs_entity(vs_entity &&) = default;
  vs_entity(const vs_entity &) = delete;
  vs_entity &operator=(const vs_entity &) = delete;

  [[nodiscard]] id_t getId() const { return id_; }



private:
  explicit vs_entity(id_t ent_id) : id_(ent_id){};
  const id_t id_;
};
