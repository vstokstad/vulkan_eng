#include "vs_app.h"

#include "vs_simple_render_system.h"

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std


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


		while (!window_.shouldClose())
		{
			glfwPollEvents();

			if (auto command_buffer = renderer_.beginFrame())
			{
				renderer_.beginSwapChainRenderPass(command_buffer);
				simple_render_system.renderGameObjects(command_buffer, game_objects_);
				renderer_.endSwapChainRenderPass(command_buffer);
				renderer_.endFrame();
			}
		}
		vkDeviceWaitIdle(device_.device());
	}

	// temporary helper function, creates a 1x1x1 cube centered at offset
	std::unique_ptr<vs_model> createCubeModel(vs_device& device, glm::vec3 offset)
	{
		std::vector<vs_model::vertex> vertices{

			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

		};
		for (auto& v : vertices)
		{
			v.position += offset;
		}
		return std::make_unique<vs_model>(device, vertices);
	}

	void vs_app::loadGameObjects()
	{
		std::shared_ptr<vs_model> model = createCubeModel(device_, {.0f, .0f, .0f});
		auto cube = vs_game_object::createGameObject();
		cube.model = model;
		cube.transform.translation = {.0f, .0f, .5f};
		cube.transform.scale = {.5f, .5f, .5f};
		game_objects_.push_back(std::move(cube));
	}
}
