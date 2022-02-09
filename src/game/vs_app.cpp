#include "vs_app.h"
#include "vs_camera.h"
#include "vs_movement_component.h"
#include "vs_point_light_render_system.h"
#include "vs_simple_physics_system.h"
#include "vs_simple_render_system.h"
// libs
#define GLM_LANG_STL11_FORCED
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

// std
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace vs {
vs_app::vs_app() {
  global_descriptor_pool_ =
      vs_descriptor_pool::vs_builder(device_)
          .setMaxSets(vs_swap_chain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                       vs_swap_chain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                       vs_swap_chain::MAX_FRAMES_IN_FLIGHT)
          .build();
}

vs_app::~vs_app() {}

void vs_app::run() {

  loadGameObjects();

  /* UBO BUFFERS
   * *****************************************************************************/
  /******************************************************************************************/
  std::vector<std::unique_ptr<vs_buffer>> ubo_buffers{
      vs_swap_chain::MAX_FRAMES_IN_FLIGHT};
  for (int i = 0; i < ubo_buffers.size(); ++i) {
    ubo_buffers[i] = std::make_unique<vs_buffer>(
        device_, sizeof(global_ubo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    if (ubo_buffers[i]->map() != VK_SUCCESS) {
      throw std::runtime_error("global ubo could not be mapped");
    }
  }
  // could be abstracted to a Master render system instead.
  auto global_set_layout =
      vs_descriptor_set_layout::vs_builder(device_)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                      VK_SHADER_STAGE_ALL_GRAPHICS)
          .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                      VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  VkDescriptorImageInfo imageInfo{};

  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView =
      renderer_.getSwapChain()->getSwapChainTextureImageView();
  imageInfo.sampler = renderer_.getSwapChain()->getSwapChainTextureSampler();

  std::vector<VkDescriptorSet> global_descriptor_sets(
      vs_swap_chain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < global_descriptor_sets.size(); ++i) {
    auto buffer_info = ubo_buffers[i]->descriptorInfo();
    vs_descriptor_writer(*global_set_layout, *global_descriptor_pool_)
        .writeBuffer(0, &buffer_info)
        .writeImage(1, &imageInfo)
        .build(global_descriptor_sets[i]);
  }

  /* RENDER SYSTEMS
   * ****************************************************************************/
  /********************************************************************************************/

  // simple models renderer
  vs_simple_render_system simple_render_system{
      device_, renderer_.getSwapChainRenderPass(),
      global_set_layout->getDescriptorSetLayout()};

  // point light system
  vs_point_light_render_system point_light_render_system{
      device_, renderer_.getSwapChainRenderPass(),
      global_set_layout->getDescriptorSetLayout()};

  // phyics system
  vs_simple_physics_system physics_system{};
  // set up some rigid bodies
  for (auto &kv : game_objects_) {
    auto &obj = kv.second;
    if (obj.model_comp == nullptr)
      continue;
    obj.addPhysicsComponent(&physics_system,
                            reactphysics3d::CollisionShapeName::BOX);
  }
  // spawn a floor
  createWorld(&physics_system);

  /*CAMERA & PLAYER/VIEWER OBJECTS*********************************************/
  /****************************************************************************/
  vs_camera camera{};
  auto camera_objet = vs_game_object::createGameObject();
  camera_objet.transform_comp.translation.z = -8.f;
  vs_movement_component movement_controller{window_.getGLFWwindow()};

  /*FRAME TIME*/
  auto currentTime = std::chrono::high_resolution_clock::now();

  /*MAIN LOOP******************************************************************/
  /****************************************************************************/
  while (!window_.shouldClose()) {
    glfwPollEvents();

    // frame time
    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime -
                                                                   currentTime)
            .count();
    currentTime = newTime;

    /* if (frameTime > 0.25f)
       frameTime = .25f;
       //TODO implement a better game loop
 */
    // Camera perspective step (if resizing etc.)
    float aspect = renderer_.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(60.f), aspect, 0.1f, 1000.f);

    /*UPDATE & RENDER**********************************************************/
    /**************************************************************************/
    if (auto command_buffer = renderer_.beginFrame()) {
      // start frame & create frame info
      int frame_index = renderer_.getFrameIndex();

      frame_info frame{frame_index,
                       frameTime,
                       command_buffer,
                       camera,
                       global_descriptor_sets[frame_index],
                       game_objects_,
                       lights_};

      // Update
      // player movement
      movement_controller.moveInPlaneXZ(window_.getGLFWwindow(), frameTime,
                                        camera_objet);
      camera.setViewYXZ(camera_objet.transform_comp.translation,
                        camera_objet.transform_comp.rotation);

      // global ubo
      global_ubo ubo{};
      ubo.projection = camera.getProjection();
      ubo.view = camera.getView();
      ubo.ambient_light_color = {.8f, .8f, .8f, .2f};
      ubo.cam_pos = glm::vec4(camera_objet.transform_comp.translation, 1.0f);

      point_light_render_system.update(frame, ubo);

      // physics
      physics_system.update(frame);

      // write updates to buffer
      ubo_buffers[frame_index]->writeToIndex(&ubo, frame_index);
      ubo_buffers[frame_index]->flush();

      // Render
      //Begin
      renderer_.beginSwapChainRenderPass(command_buffer);

      // my frame rendering
      simple_render_system.renderGameObjects(frame);
      point_light_render_system.render(frame);

      //END
      renderer_.endSwapChainRenderPass(command_buffer);
      renderer_.endFrame();
    }
  }

  vkDeviceWaitIdle(device_.device());
}
void vs_app::createWorld(vs_simple_physics_system *physicssystem) {
  {
    auto floor = asset_manager.spawnGameObject("cube.obj", {0.0f, 5.f, 0.0f},
                                               {0.f, 0, 0}, {20.f, 1.f, 20.f});
    floor.addPhysicsComponent(physicssystem,
                              reactphysics3d::CollisionShapeName::BOX);

    floor.rigid_body_comp->rigidBody->enableGravity(false);
    floor.rigid_body_comp->rigidBody->setType(reactphysics3d::BodyType::STATIC);
    floor.rigid_body_comp->rigidBody->setIsActive(true);

    game_objects_.emplace(floor.getId(), std::move(floor));
  }
}
void vs_app::loadGameObjects() {

  //  createHairballScene();
  // { glm::radians(90.f), glm::radians(180.f), glm::radians(45.f) }
  auto game_object = asset_manager.spawnGameObject(
      "viking_room.obj", {0.f, 2.f, 0.f},
      {1.2f,0.f, 0.f},
      {5.f, 5.f, 5.f});

  game_objects_.emplace(game_object.getId(), std::move(game_object));

  /*** ROW OF VASES******/
  /*
    for (int i = 0; i < 10; ++i) {
      auto game_object = asset_manager.spawnGameObject("flat_vase.obj");
      game_object.transform_comp.translation = {i - 2.f, 5.f, 0.f};
      game_object.transform_comp.scale = glm::vec3(.5f, .5f, .5f);
      game_objects_.emplace(game_object.getId(), std::move(game_object));
    }
    */
  /** SPINNING POINT LIGHTS **/
  createSpinningPointLights();
}
void vs_app::createSpinningPointLights() {
  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f}, {.1f, .1f, 1.f}, {.1f, 1.f, .1f},
      {1.f, 1.f, .1f}, {.1f, 1.f, 1.f}, {1.f, 1.f, 1.f} //
  };

  for (int i = 0; i < lightColors.size(); ++i) {
    auto point_light = vs_game_object::createPointLight(5.f);
    point_light.color = lightColors[i];
    auto rotate_light = glm::rotate(
        glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(),
        {0.f, -1.f, 0.f});

    point_light.transform_comp.translation =
        glm::vec3{0.f, -2.f, 0.f} +
        glm::vec3(rotate_light * glm::vec4(-1.f, -1.f, -1.f, 1.f));

    lights_.emplace(point_light.getId(), std::move(point_light));
  }
}
void vs_app::createHairballScene() {
  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f}, {.1f, .1f, 1.f}, {.1f, 1.f, .1f},
      {1.f, 1.f, .1f}, {.1f, 1.f, 1.f}, {1.f, 1.f, 1.f} //
  };
  /*
    auto spawned =
        asset_manager.spawnGameObject("smooth_vase.obj", {1.f, 0.5f, 2.5f});
    game_objects_.emplace(spawned.getId(), std::move(spawned));
  */
  {
    for (int i = 0; i < 50; ++i) {
      auto rotate = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / 50,
                                {0.1f, -1.f, 0.2f});
      auto color_cube =
          asset_manager.spawnGameObject("colored_cube.obj", {0.f, -5.f, 0.f},
                                        {i * 1.f, -1.f, 0}, {0.2f, 0.2f, 0.2f});

      color_cube.transform_comp.translation =
          glm::vec3(rotate * glm::vec4(-3.f, -5.f, -1.5f, 1.f));

      game_objects_.emplace(color_cube.getId(), std::move(color_cube));
    }
  }
  if (asset_manager.isModelLoaded("hairball.obj")) {
    auto hairball = asset_manager.spawnGameObject(
        "hairball.obj", {0.0f, -20.f, 0.0f}, {0.f, 0.f, 0.f}, {5.f, 5.f, 5.f});
    game_objects_.emplace(hairball.getId(), std::move(hairball));
  }

  for (int i = 0; i < lightColors.size(); ++i) {

    auto point_light = vs_game_object::createPointLight(10.f);
    point_light.color = lightColors[i];
    auto rotate_light = glm::rotate(
        glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(),
        {0.f, -1.f, 0.f});

    point_light.transform_comp.translation =
        glm::vec3(rotate_light * glm::vec4(-3.f, -0.5f, -1.5f, 1.f));
    lights_.emplace(point_light.getId(), std::move(point_light));
  }
}
} // namespace vs
