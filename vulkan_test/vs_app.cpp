#include "vs_app.h"
#include "vs_movement_component.h"
#include "vs_simple_render_system.h"
#include "vs_camera.h"
//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std
#include <chrono>

namespace vs
{
	struct global_ubo
	{
		glm::mat4 projection_view{1.f};
		glm::vec3 light_direction = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
	};

	vs_app::vs_app()
	{
		loadGameObjects();
	}

	vs_app::~vs_app()
	{
	}

	void vs_app::run()
	{
		vs_buffer global_ubo_buffer{
			device_,
			sizeof(global_ubo),
			vs_swap_chain::MAX_FRAMES_IN_FLIGHT,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			device_.properties.limits.minUniformBufferOffsetAlignment,
		};

		global_ubo_buffer.map();


		vs_simple_render_system simple_render_system{device_, renderer_.getSwapChainRenderPass()};
		vs_camera camera{};

		auto viewerObject = vs_game_object::createGameObject();
		vs_movement_component movement_controller{window_.getGLFWwindow()};

		auto currentTime = std::chrono::high_resolution_clock::now();

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


			//Camera perspective step
			float aspect = renderer_.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

			if (auto command_buffer = renderer_.beginFrame())
			{
				//start frame
				int frame_index = renderer_.getFrameIndex();
				frame_info frame{
					frame_index, frameTime, command_buffer, camera
				};

				//Update
				global_ubo ubo{};
				ubo.projection_view = camera.getProjection() * camera.getView();
				global_ubo_buffer.writeToIndex((void*)&ubo, frame_index);
				if (global_ubo_buffer.flushIndex(frame_index) != VK_SUCCESS)
				{
					throw std::runtime_error("something is wrong while flushing UBO");
				};

				//Render
				renderer_.beginSwapChainRenderPass(command_buffer);
				simple_render_system.renderGameObjects(frame, game_objects_);
				renderer_.endSwapChainRenderPass(command_buffer);
				renderer_.endFrame();
			}
		}
		vkDeviceWaitIdle(device_.device());
	}


	void vs_app::loadGameObjects()
	{
		for (int i = 0; i < 10; ++i)
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
		}
	}
}
