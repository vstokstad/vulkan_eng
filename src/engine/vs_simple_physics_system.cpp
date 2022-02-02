//
// Created by Vilhelm Stokstad on 2022-01-31.
//

#include "vs_simple_physics_system.h"
#include "vs_frame_info.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <reactphysics3d/systems/BroadPhaseSystem.h>

// std

namespace vs {

vs_simple_physics_system::vs_simple_physics_system() {

  // Create the world settings
  reactphysics3d::PhysicsWorld::WorldSettings settings;
  settings.defaultVelocitySolverNbIterations = 20;
  settings.isSleepingEnabled = false;
  settings.gravity = reactphysics3d::Vector3(0.f, 9.81f, 0.f);
  physics_world = physics_common.createPhysicsWorld(settings);
}

vs_simple_physics_system::~vs_simple_physics_system() {
  physics_common.destroyPhysicsWorld(physics_world);
  physics_world = nullptr;
}

void vs_simple_physics_system::update(const frame_info &frame_info) {

  physics_world->update(frame_info.frame_time);

  for (auto &kv : frame_info.game_objects) {
    auto &obj = kv.second;
    if (obj.rigid_body_comp == nullptr)
      continue;

    obj.rigid_body_comp->rigidBody->updateMassPropertiesFromColliders();

    reactphysics3d::Transform t = obj.rigid_body_comp->rigidBody->getCollider(0)
                                      ->getLocalToWorldTransform();

    glm::vec3 rb_position = {t.getPosition().x, t.getPosition().y,
                             t.getPosition().z};
    glm::vec3 rb_rotation = {t.getOrientation().getVectorV().x,
                             t.getOrientation().getVectorV().y,
                             t.getOrientation().getVectorV().z};
    obj.transform_comp.rotation = rb_rotation;
    obj.transform_comp.translation = rb_position;
  }
}
} // namespace vs
