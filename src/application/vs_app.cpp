#include "vs_app.h"
#include "profiler.h"
#include "vs_asset.h"
#include "vs_camera.h"
#include "vs_camera_movement_component.h"
#include "vs_point_light_render_system.h"
#include "vs_simple_render_system.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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
          .build();
}

vs_app::~vs_app() {}

void vs_app::run() {

  /** UBO BUFFERS **/
  std::vector<std::unique_ptr<vs_buffer>> ubo_buffers{
      vs_swap_chain::MAX_FRAMES_IN_FLIGHT};
  for (auto &ubo_buffer : ubo_buffers) {
    ubo_buffer = std::make_unique<vs_buffer>(
        device_, sizeof(global_ubo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    if (ubo_buffer->map() != VK_SUCCESS) {
      throw std::runtime_error("global ubo could not be mapped");
    }
  }

  // could be abstracted to a Master render system instead.
  auto global_set_layout =
      vs_descriptor_set_layout::vs_builder(device_)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  /**DESCRIPTOR SETS**/
  std::vector<VkDescriptorSet> global_descriptor_sets(
      vs_swap_chain::MAX_FRAMES_IN_FLIGHT);

  for (int i = 0; i < global_descriptor_sets.size(); ++i) {
    auto buffer_info = ubo_buffers[i]->descriptorInfo();
    vs_descriptor_writer(*global_set_layout, *global_descriptor_pool_)
        .writeBuffer(0, &buffer_info)
        .build(global_descriptor_sets[i]);
  }

  /** RENDER SYSTEMS **/
  // simple models renderer
  vs_simple_render_system simple_render_system{
      device_, renderer_.getSwapChainRenderPass(),
      global_set_layout->getDescriptorSetLayout()};

  // point light system
  vs_point_light_render_system point_light_render_system{
      device_, renderer_.getSwapChainRenderPass(),
      global_set_layout->getDescriptorSetLayout()};

  /** CAMERA & PLAYER/VIEWER OBJECTS **/
  vs_camera camera{};
  auto camera_object = vs_game_object::createGameObject();
  camera_object.transform_comp.translation.z = -3.f;
  vs_camera_movement_component movement_controller{window_.getGLFWwindow()};

  /** Spawn test scene **/
  createWorld();

  /** FRAME TIME **/
  auto current_time = std::chrono::high_resolution_clock::now();
  /** MAIN LOOP **/
  while (!window_.shouldClose()) {
    glfwPollEvents();

    // frame time
    auto new_time = std::chrono::high_resolution_clock::now();
    float frame_time =
        std::chrono::duration<float, std::chrono::seconds::period>(new_time -
                                                                   current_time)
            .count();
    current_time = new_time;

    // Camera perspective step (if resizing etc.)
    float aspect = renderer_.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(60.f), aspect, 0.01f, 1000.f);

    // Update
    // player movement
    movement_controller.moveInPlaneXZ(window_.getGLFWwindow(), frame_time,
                                      camera_object);

    camera.setViewYXZ(camera_object.transform_comp.translation,
                      camera_object.transform_comp.rotation);

    /** UPDATE & RENDER **/
    if (auto command_buffer = renderer_.beginFrame()) {

      /* if (frameTime > 0.25f)
         frameTime = .25f;
         //TODO implement a better game loop
      */

      // start frame & create frame info
      int frame_index = renderer_.getFrameIndex();

      frame_info frame{frame_index,    frame_time,
                       command_buffer, global_descriptor_sets[frame_index],
                       game_objects_,  lights_};

      // global ubo
      global_ubo ubo{};
      ubo.projection = camera.getProjection();
      ubo.view = camera.getView();
      ubo.inv_view_mat = camera.getInverseView();
      point_light_render_system.update(frame, ubo);

      // write updates to buffer
      ubo_buffers[frame_index]->writeToIndex(&ubo, frame_index);
      ubo_buffers[frame_index]->flush();

      /** Render **/
      renderer_.beginSwapChainRenderPass(command_buffer);

      //  render systems
      simple_render_system.renderGameObjects(frame);
      point_light_render_system.render(frame);

      // END
      renderer_.endSwapChainRenderPass(command_buffer);
      renderer_.endFrame();
    }
  }
  vkDeviceWaitIdle(device_.device());
}
void vs_app::createWorld() {

  auto floor = vs_game_object::createGameObject();
  floor.model_comp = vs_model_component::createModelFromFile(
      device_, "assets/models/quad.obj");
  floor.transform_comp.scale = {5.f, 1.f, 5.f};
  floor.transform_comp.translation = {0.1f, 0.1f, 1.f};
  game_objects_.emplace(floor.getId(), std::move(floor));

  auto vase = vs_game_object::createGameObject();
  vase.model_comp = vs_model_component::createModelFromFile(
      device_, "assets/models/smooth_vase.obj");
  vase.transform_comp.scale = {3.f, 1.5f, 3.f};
  vase.transform_comp.translation = {0.f, 0.1f, 0.f};
  game_objects_.emplace(vase.getId(), std::move(vase));

  /** SPINNING POINT LIGHTS **/
  createSpinningPointLights();
}
void vs_app::loadVikingRoom() {

  auto game_object = vs_game_object::createGameObject();
  game_object.model_comp = vs_model_component::createModelFromFile(
      device_, "assets/models/viking_room.obj");
  game_object.transform_comp.rotation = {glm::half_pi<float>(),
                                         glm::half_pi<float>(), 0.0f};
  game_object.transform_comp.translation = {0, 1.f, 0};
  // game_object.model_texture = vs_texture::createTextureFromFile(
  //   device_, "assets/textures/viking_room.png");

  game_objects_.emplace(game_object.getId(), std::move(game_object));
}
void vs_app::createSpinningPointLights() {
  // https://www.color-hex.com/color-palette/5361
  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f}, {.1f, .1f, 1.f}, {.1f, 1.f, .1f},
      {1.f, 1.f, .1f}, {.1f, 1.f, 1.f}, {1.f, 1.f, 1.f} //
  };

  for (int i = 0; i < lightColors.size(); ++i) {
    auto point_light = vs_game_object::createPointLight(.1f);
    point_light.color = lightColors[i];
    auto rotate_light = glm::rotate(
        glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(),
        {0.f, -1.f, 0.f});

    point_light.transform_comp.translation =
        glm::vec3(rotate_light * glm::vec4(-1.f, -1.f, -1.f, 1.f));

    lights_.emplace(point_light.getId(), std::move(point_light));
  }
}
} // namespace vs
