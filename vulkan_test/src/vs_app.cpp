#include "vs_app.h"
#include "vs_movement_component.h"
#include "render_systems/vs_simple_render_system.h"
#include "render_systems/vs_point_light_system.h"
#include "vs_camera.h"
//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std
#include <chrono>
#include <stdexcept>

namespace vs
{
	struct global_ubo
	{
		glm::mat4 projection{1.f};
		glm::mat4 view{1.f};
		glm::vec4 ambient_light_color{1.f, 1.f, 1.f, 0.2f}; //w is intensity
		glm::vec3 point_light_position{-1.f, -1.f, 0.f};
		alignas(16) glm::vec4 point_light_color{1.0f, 1.0f, 0.f, 1.f}; // w is light intensity
	};

	vs_app::vs_app()
	{
		global_descriptor_pool_ = vs_descriptor_pool::vs_builder(device_)
		                          .setMaxSets(vs_swap_chain::MAX_FRAMES_IN_FLIGHT)
		                          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, vs_swap_chain::MAX_FRAMES_IN_FLIGHT)
		                          .build();

		loadGameObjects();
	}

	vs_app::~vs_app()
	{
	}

	void vs_app::run()
	{
		/* UBO BUFFERS *****************************************************************************/
		/******************************************************************************************/
		std::vector<std::unique_ptr<vs_buffer>> ubo_buffers{vs_swap_chain::MAX_FRAMES_IN_FLIGHT};
		for (int i = 0; i < ubo_buffers.size(); ++i)
		{
			ubo_buffers[i] = std::make_unique<vs_buffer>(
				device_,
				sizeof(global_ubo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

			if (ubo_buffers[i]->map() != VK_SUCCESS)
			{
				throw std::runtime_error("global ubo could not be mapped");
			}
		}
		//could be abstracted to a Master render system instead.
		auto global_set_layout = vs_descriptor_set_layout::vs_builder(device_)
		                         .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		                         .build();

		std::vector<VkDescriptorSet> global_descriptor_sets(vs_swap_chain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < global_descriptor_sets.size(); ++i)
		{
			auto buffer_info = ubo_buffers[i]->descriptorInfo();
			vs_descriptor_writer(*global_set_layout, *global_descriptor_pool_)
				.writeBuffer(0, &buffer_info)
				.build(global_descriptor_sets[i]);
		}

		/* RENDER SYSTEMS ****************************************************************************/
		/********************************************************************************************/
		vs_simple_render_system simple_render_system{
			device_, renderer_.getSwapChainRenderPass(), global_set_layout->getDescriptorSetLayout()
		};
		vs_point_light_system point_light_system{
			device_, renderer_.getSwapChainRenderPass(), global_set_layout->getDescriptorSetLayout()
		};


		/*CAMERA & PLAYER/VIEWER OBJECTS***************************************************************/
		/*********************************************************************************************/
		vs_camera camera{};
		auto viewerObject = vs_game_object::createGameObject();
		viewerObject.transform.translation.z = -2.5f;
		vs_movement_component movement_controller{window_.getGLFWwindow()};


		/*FRAME TIME*/
		auto currentTime = std::chrono::high_resolution_clock::now();


		/*MAIN LOOP ******************************************************************************************/
		/***********************************************************************************************/
		while (!window_.shouldClose())
		{
			glfwPollEvents();

			//frame time / delta time
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			//player movement
			movement_controller.moveInPlaneXZ(window_.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			//Camera perspective step (if resizing etc.)
			float aspect = renderer_.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(90.f), aspect, 0.1f, 1000.f);


			/*UPDATE & RENDER *************************************************************************************/
			/****************************************************************************************************/
			if (auto command_buffer = renderer_.beginFrame())
			{
				//start frame & create frame info
				int frame_index = renderer_.getFrameIndex();
				frame_info frame{
					frame_index, frameTime, command_buffer, camera, global_descriptor_sets[frame_index], game_objects_
				};

				//Update
				global_ubo ubo{};
				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();
				ubo_buffers[frame_index]->writeToIndex(&ubo, frame_index);
				ubo_buffers[frame_index]->flush();

				//Render
				renderer_.beginSwapChainRenderPass(command_buffer);
				simple_render_system.renderGameObjects(frame);
				point_light_system.render(frame);
				renderer_.endSwapChainRenderPass(command_buffer);
				renderer_.endFrame();
			}
		}
		vkDeviceWaitIdle(device_.device());
	}


	void vs_app::loadGameObjects()
	{
		std::shared_ptr<vs_model> model = vs_model::createModelFromFile(
			device_, "models/quad.obj", "models/");
		auto floor = vs_game_object::createGameObject();
		floor.model = model;
		floor.transform.translation = {0.f, .5f, 0.f};
		floor.transform.scale = glm::vec3(3.f, 1.f, 3.f);
		game_objects_.emplace(floor.getId(), std::move(floor));


		for (int i = 0; i < 5; ++i)
		{
			model = vs_model::createModelFromFile(
				device_, "models/smooth_vase.obj", "models/");
			auto game_object = vs_game_object::createGameObject();
			game_object.model = model;
			game_object.transform.translation = {i - 2.f, .5f, 0.f};
			game_object.transform.scale = glm::vec3(3.f, 1.5f, 3.f);
			game_objects_.emplace(game_object.getId(), std::move(game_object));
		}


		/*for (int i = 0; i < 10; ++i)
		{
			std::shared_ptr<vs_model> model = vs_model::createModelFromFile(
				device_, "models/smooth_vase.obj");
			auto game_object = vs_game_object::createGameObject();
			game_object.model = model;
			game_object.transform.translation = {i + .0f, .5f, 2.5f + i};
			game_object.transform.scale = glm::vec3(1.f, 1.f, 1.f);
			game_objects_.push_back(std::move(game_object));

			std::shared_ptr<vs_model> flat_vase = vs_model::createModelFromFile(
				device_, "models/flat_vase.obj");
			auto vase = vs_game_object::createGameObject();
			vase.model = flat_vase;
			vase.transform.translation = {-i * .5f, .5f, 2.5f - i};
			vase.transform.scale = glm::vec3(1.f, 1.f, 1.f);
			game_objects_.push_back(std::move(vase));
		}*/
	}
}
