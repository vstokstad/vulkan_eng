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
	vs_app::vs_app()
	{
		loadGameObjects();
	}

	vs_app::~vs_app()
	{
	}

	void vs_app::run()
	{
		vs_simple_render_system simple_render_system{device_, renderer_.getSwapChainRenderPass()};
		vs_camera camera{};
		camera.setViewTarget({-1.f, -2.f, 2.f}, {0.f, 0.f, 2.5f});

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
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

			if (auto command_buffer = renderer_.beginFrame())
			{
				renderer_.beginSwapChainRenderPass(command_buffer);
				simple_render_system.renderGameObjects(command_buffer, game_objects_, camera);
				renderer_.endSwapChainRenderPass(command_buffer);
				renderer_.endFrame();
			}
		}
		vkDeviceWaitIdle(device_.device());
	}


	void vs_app::loadGameObjects()
	{
		std::shared_ptr<vs_model> model = vs_model::createModelFromFile(
			device_, "models/smooth_vase.obj");
		auto game_object = vs_game_object::createGameObject();
		game_object.model = model;
		game_object.transform.translation = {.0f, .0f, 2.5f};
		game_object.transform.scale = glm::vec3(3.f);
		game_objects_.push_back(std::move(game_object));
	}
}
