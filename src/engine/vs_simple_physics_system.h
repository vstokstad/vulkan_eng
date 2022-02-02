#pragma once
// std
#include <memory>
// lib
#include <reactphysics3d/reactphysics3d.h>

namespace vs {
class frame_info;
class vs_simple_physics_system {
public:
  vs_simple_physics_system();
  ~vs_simple_physics_system();

  vs_simple_physics_system(const vs_simple_physics_system &) = delete;

  vs_simple_physics_system &operator==(const vs_simple_physics_system &) = delete;

 void update(const frame_info &frame_info);

  reactphysics3d::PhysicsCommon physics_common{};
  reactphysics3d::PhysicsWorld *physics_world;
};
} // namespace vs
