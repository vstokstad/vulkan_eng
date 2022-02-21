//
// Created by Vilhelm Stokstad on 2022-01-31.
//

#include "vs_simple_physics_system.h"
#include "vs_frame_info.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// std

namespace vs {

vs_simple_physics_system::vs_simple_physics_system() {

  // Create the world settings
  reactphysics3d::PhysicsWorld::WorldSettings settings;
  settings.defaultVelocitySolverNbIterations = 20;
  settings.isSleepingEnabled = false;
  settings.defaultBounciness = 0.2f;
  settings.gravity = reactphysics3d::Vector3(0.f, 9.81f, 0.f);
  physics_world = physics_common.createPhysicsWorld(settings);
  physics_world->setIsDebugRenderingEnabled(false);
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


    reactphysics3d::Transform t = obj.rigid_body_comp->rigidBody->getTransform();

    glm::vec3 rb_position = {t.getPosition().x, t.getPosition().y,
                             t.getPosition().z};

    /** THANKS TO Magnus Auvinen @ MachineGames for helping see this clearly! **/
    glm::quat rb_quatRot = glm::quat(t.getOrientation().w, t.getOrientation().x,
                                     t.getOrientation().y, t.getOrientation().z);


    obj.transform_comp.rotation = eulerAngles(rb_quatRot);
    obj.transform_comp.translation = rb_position;
  }
}
} // namespace vs
