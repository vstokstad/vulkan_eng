#pragma once
#include "game/general/vs_camera.h"
#include "game/entities/vs_game_object.h"
#include <vulkan/vulkan.h>

namespace vs
{
#define MAX_LIGHTS 10

	struct point_light
	{
		glm::vec4 position{}; //ignore w
		glm::vec4 color{}; // w is intensity
	};

	struct global_ubo
	{
		glm::mat4 projection{1.f};
		glm::mat4 view{1.f};
		glm::vec4 ambient_light_color{1.f, 1.f, 1.f, 0.2f}; //w is intensity
		point_light point_lights[MAX_LIGHTS];
		int num_lights;
	};

	struct frame_info
	{
		int frame_index;
		float frame_time;
		VkCommandBuffer command_buffer;
		vs_camera& camera;
		VkDescriptorSet global_descriptor_set;
		vs_game_object::map& game_objects;
	};
}
