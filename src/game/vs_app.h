#pragma once

#include "vs_asset_manager.h"
#include "vs_descriptors.h"
#include "vs_device.h"
#include "vs_game_object.h"
#include "vs_renderer.h"
#include "vs_simple_physics_system.h"
#include "vs_window.h"

// std
#include <memory>
// lib
#include <reactphysics3d/reactphysics3d.h>


namespace vs {
class vs_model_component;

class vs_app {
public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 500;

  vs_app();
  ~vs_app();

  vs_app(const vs_app &) = delete;
  vs_app &operator==(const vs_app &) = delete;

  void run();

private:
  void loadGameObjects();

  vs_window window_{WIDTH, HEIGHT, "Vulkan App"};
  vs_device device_{window_};
  vs_renderer renderer_{window_, device_};
  vs_asset_manager asset_manager{device_};

  // physics
  // reactphysics3d::PhysicsCommon physics_common{};
  // reactphysics3d::PhysicsWorld*physics;

  // order of declaration matters (pool need to be constructed after device and
  // destroyed before device.)

  std::unique_ptr<vs_descriptor_pool> global_descriptor_pool_{};
  vs_game_object::map game_objects_;
  vs_game_object::map lights_;
  void createWorld(vs_simple_physics_system *physicssystem);
};
} // namespace vs
