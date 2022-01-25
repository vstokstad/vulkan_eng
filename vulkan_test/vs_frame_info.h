#pragma once
#include "vs_camera.h"

#include <vulkan/vulkan.h>

namespace vs
{
	struct frame_info
	{
		int frame_index;
		float frame_time;
		VkCommandBuffer command_buffer;
		vs_camera& camera;
	};
}
