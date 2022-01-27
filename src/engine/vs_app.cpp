#include "vs_app.h"
#include "vs_camera.h"
#include "vs_movement_component.h"
#include "vs_point_light_system.h"
#include "vs_simple_render_system.h"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

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

  loadGameObjects();
}

vs_app::~vs_app() {}

void vs_app::run() {
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
  auto global_set_layout = vs_descriptor_set_layout::vs_builder(device_)
                               .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                           VK_SHADER_STAGE_ALL_GRAPHICS)
                               .build();

  std::vector<VkDescriptorSet> global_descriptor_sets(
      vs_swap_chain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < global_descriptor_sets.size(); ++i) {
    auto buffer_info = ubo_buffers[i]->descriptorInfo();
    vs_descriptor_writer(*global_set_layout, *global_descriptor_pool_)
        .writeBuffer(0, &buffer_info)
        .build(global_descriptor_sets[i]);
  }

  /* RENDER SYSTEMS
   * ****************************************************************************/
  /********************************************************************************************/
  vs_simple_render_system simple_render_system{
      device_, renderer_.getSwapChainRenderPass(),
      global_set_layout->getDescriptorSetLayout()};
  // pointlight
  vs_point_light_system point_light_system{
      device_, renderer_.getSwapChainRenderPass(),
      global_set_layout->getDescriptorSetLayout()};

  /*CAMERA & PLAYER/VIEWER
   * OBJECTS***************************************************************/
  /*********************************************************************************************/
  vs_camera camera{};
  auto viewerObject = vs_game_object::createGameObject();
  viewerObject.transform.translation.z = -2.5f;
  vs_movement_component movement_controller{window_.getGLFWwindow()};

  /*FRAME TIME*/
  auto currentTime = std::chrono::high_resolution_clock::now();

  /*MAIN LOOP
   * ******************************************************************************************/
  /***********************************************************************************************/
  while (!window_.shouldClose()) {
    glfwPollEvents();

    // frame time / delta time
    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime -
                                                                   currentTime)
            .count();
    std::cout << "frame_time: " << frameTime << std::endl;
    currentTime = newTime;

    if (frameTime > 0.25f)
      frameTime = .25f;

    // Camera perspective step (if resizing etc.)
    float aspect = renderer_.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(90.f), aspect, 0.01f, 10000.f);

    /*UPDATE & RENDER
     * *************************************************************************************/
    /****************************************************************************************************/
    if (auto command_buffer = renderer_.beginFrame()) {
      // start frame & create frame info
      int frame_index = renderer_.getFrameIndex();
      frame_info frame{frame_index,
                       frameTime,
                       command_buffer,
                       camera,
                       global_descriptor_sets[frame_index],
                       game_objects_};

      // Update
      // player movement
      movement_controller.moveInPlaneXZ(window_.getGLFWwindow(), frameTime,
                                        viewerObject);
      camera.setViewYXZ(viewerObject.transform.translation,
                        viewerObject.transform.rotation);

      global_ubo ubo{};
      ubo.projection = camera.getProjection();
      ubo.view = camera.getView();
      ubo.ambient_light_color = {.1f, .1f, .8f * frameTime, .3f};
      point_light_system.update(frame, ubo);

      // write updates to buffer
      ubo_buffers[frame_index]->writeToIndex(&ubo, frame_index);
      ubo_buffers[frame_index]->flush();

      // Render
      renderer_.beginSwapChainRenderPass(command_buffer);
      simple_render_system.renderGameObjects(frame);
      point_light_system.render(frame);
      renderer_.endSwapChainRenderPass(command_buffer);
      renderer_.endFrame();
    }
  }

  vkDeviceWaitIdle(device_.device());
}

void vs_app::loadGameObjects() {

  std::shared_ptr<vs_model> model;
  {
    model = vs_model::createModelFromFile(device_, "models/hairball.obj",
                                          "models/");
    auto object = vs_game_object::createGameObject();
    object.model = model;
    object.transform.translation = {0.f, .0f, 0.f};
    object.transform.scale = glm::vec3(.5f, .5f, .5f);
    game_objects_.emplace(object.getId(), std::move(object));
  }
  {
    model = vs_model::createModelFromFile(device_, "models/cloud/cumulus00.obj",
                                          "models/cloud");
    auto object = vs_game_object::createGameObject();
    object.model = model;
    object.transform.translation = {0.f, -10.f, 0.f};
    object.transform.scale = glm::vec3(1.f, 1.5f, 1.5f);
    game_objects_.emplace(object.getId(), std::move(object));
  }

  /*** ROW OF VASES******/
  for (int i = 0; i < 5; ++i) {
    model = vs_model::createModelFromFile(device_, "models/smooth_vase.obj",
                                          "models/");
    auto game_object = vs_game_object::createGameObject();
    game_object.model = model;
    game_object.transform.translation = {i - 2.f, .5f, 0.f};
    game_object.transform.scale = glm::vec3(1.5f, 1.5f, 3.f);
    game_objects_.emplace(game_object.getId(), std::move(game_object));
  }

  /** SPINNING POINT LIGHTS **/

  // colored lights
  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f}, {.1f, .1f, 1.f}, {.1f, 1.f, .1f},
      {1.f, 1.f, .1f}, {.1f, 1.f, 1.f}, {1.f, 1.f, 1.f} //
  };

  for (int i = 0; i < lightColors.size(); ++i) {
    auto point_light = vs_game_object::makePointLight(0.5f);
    point_light.color = lightColors[i];
    auto rotate_light = glm::rotate(
        glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(),
        {0.f, -1.f, 0.f});

    point_light.transform.translation =
        glm::vec3(rotate_light * glm::vec4(-1.f, -1.f, -1.f, 1.f));

    game_objects_.emplace(point_light.getId(), std::move(point_light));
  }
  // fly hig lights
  for (int i = 0; i < 3; ++i) {
    auto light = vs_game_object::makePointLight(50);
    light.transform.translation = {-20.f * i, -20.f * i - 20.f,
                                   20.f * i + 20.f};
    game_objects_.emplace(light.getId(), std::move(light));
  }
}
} // namespace vs
