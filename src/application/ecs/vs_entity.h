//
// Created by vstok on 2022-02-18.
//
#pragma once
#include "vs_component.h"

namespace vs {

struct vs_entity {
  using id_t = unsigned int;
//copy constructors
    vs_entity &operator=(vs_entity &&) = default;
    vs_entity(vs_entity &&) = default;
    vs_entity(const vs_entity &) = delete;
    vs_entity &operator=(const vs_entity &) = delete;

    // constructor
  static vs_entity createEntity() {
    static id_t current_id = 0;
    return vs_entity{current_id++};
  }

  virtual //id
   id_t getId() const { return id_; }




private:
  ;
  id_t id_;
  uint16_t version;
 // std::vector<id_t> components; //component id

protected:
  explicit vs_entity(id_t ent_id) : id_(ent_id){}
};
} // namespace vs
