
#include "vs_game_object.h"
#include "vs_simple_physics_system.h"
#include <glm/gtc/quaternion.hpp>

namespace vs {
glm::mat4 transform_component::mat4() {
  const float c3 = glm::cos(rotation.z);
  const float s3 = glm::sin(rotation.z);
  const float c2 = glm::cos(rotation.x);
  const float s2 = glm::sin(rotation.x);
  const float c1 = glm::cos(rotation.y);
  const float s1 = glm::sin(rotation.y);
  return glm::mat4{{
                       scale.x * (c1 * c3 + s1 * s2 * s3),
                       scale.x * (c2 * s3),
                       scale.x * (c1 * s2 * s3 - c3 * s1),
                       0.0f,
                   },
                   {
                       scale.y * (c3 * s1 * s2 - c1 * s3),
                       scale.y * (c2 * c3),
                       scale.y * (c1 * c3 * s2 + s1 * s3),
                       0.0f,
                   },
                   {
                       scale.z * (c2 * s1),
                       scale.z * (-s2),
                       scale.z * (c1 * c2),
                       0.0f,
                   },
                   {translation.x, translation.y, translation.z, 1.0f}};
}

glm::mat3 transform_component::normal_matrix() {
  const float c3 = glm::cos(rotation.z);
  const float s3 = glm::sin(rotation.z);
  const float c2 = glm::cos(rotation.x);
  const float s2 = glm::sin(rotation.x);
  const float c1 = glm::cos(rotation.y);
  const float s1 = glm::sin(rotation.y);
  const glm::vec3 inv_scale = 1.0f / scale;

  return glm::mat3{
      {inv_scale.x * (c1 * c3 + s1 * s2 * s3), inv_scale.x * (c2 * s3),
       inv_scale.x * (c1 * s2 * s3 - c3 * s1)},
      {
          inv_scale.y * (c3 * s1 * s2 - c1 * s3),
          inv_scale.y * (c2 * c3),
          inv_scale.y * (c1 * c3 * s2 + s1 * s3),
      },
      {
          inv_scale.z * (c2 * s1),
          inv_scale.z * (-s2),
          inv_scale.z * (c1 * c2),
      },

  };
}

vs_game_object vs_game_object::createPointLight(float intensity, float radius,
                                                glm::vec3 color) {
  vs_game_object object = vs_game_object::createGameObject();
  object.color = color;
  object.transform_comp.scale.x = radius;
  object.point_light_comp = std::make_unique<point_light_component>();
  object.point_light_comp->light_intensity = intensity;
  return object;
}
void vs_game_object::addPhysicsComponent(
    vs_simple_physics_system *physicssystem,
    reactphysics3d::CollisionShapeName shape) {

  rigid_body_comp = std::make_unique<rigid_body_component>(
      transform_comp, physicssystem, shape, transform_comp.scale);
}

rigid_body_component::rigid_body_component(
    transform_component transform_comp, vs_simple_physics_system *physicssystem,
    reactphysics3d::CollisionShapeName shape, glm::vec3 collider_size) {
  {

    switch (shape) {
    case reactphysics3d::CollisionShapeName::SPHERE:
      collision_shape =
          physicssystem->physics_common.createSphereShape(collider_size.x);
      break;
    case reactphysics3d::CollisionShapeName::CAPSULE:
      collision_shape = physicssystem->physics_common.createCapsuleShape(
          collider_size.x, collider_size.y);
      break;
    case reactphysics3d::CollisionShapeName::BOX:
      collider_size *= 0.5f;
      collision_shape =
          physicssystem->physics_common.createBoxShape(reactphysics3d::Vector3(
              collider_size.x, collider_size.y, collider_size.z));
      break;
    default:
      collision_shape =
          physicssystem->physics_common.createBoxShape(reactphysics3d::Vector3(
              collider_size.x, collider_size.y, collider_size.z));
      break;
    }

    glm::quat rot_quat = glm::quat_cast(transform_comp.mat4());
    transform = reactphysics3d::Transform(
        {transform_comp.translation.x, transform_comp.translation.y-0.1f,
         transform_comp.translation.z},
        {rot_quat.x, rot_quat.y, rot_quat.z, rot_quat.w});

    rigidBody = physicssystem->physics_world->createRigidBody(transform);
    reactphysics3d::Collider *collider = rigidBody->addCollider(
        collision_shape, reactphysics3d::Transform::identity());

    transform.setOrientation({rot_quat.x, rot_quat.y, rot_quat.z, rot_quat.w});

    transform.setPosition({transform_comp.translation.x,
                           transform_comp.translation.y,
                           transform_comp.translation.z});

    collider->getMaterial().setBounciness(0);
    rigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
    rigidBody->enableGravity(true);
    rigidBody->setIsActive(true);
  };
}
} // namespace vs
