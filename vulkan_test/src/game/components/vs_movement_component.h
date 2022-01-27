#pragma once
#include "game/entities/vs_game_object.h"
#include "engine/renderer/vs_window.h"

namespace vs
{
	static float mouse_speed_scroll_modifier = 0.f;

	class vs_movement_component
	{
	public:
		vs_movement_component(GLFWwindow* window);

		struct key_mappings
		{
			int left = GLFW_KEY_A;
			int right = GLFW_KEY_D;
			int forward = GLFW_KEY_W;
			int back = GLFW_KEY_S;
			int up = GLFW_KEY_E;
			int down = GLFW_KEY_Q;
			int look_left = GLFW_KEY_LEFT;
			int look_right = GLFW_KEY_RIGHT;
			int look_up = GLFW_KEY_UP;
			int look_down = GLFW_KEY_DOWN;
			int escape = GLFW_KEY_ESCAPE;
			int f = GLFW_KEY_F;
		};

		struct input
		{
			glm::vec3 pos_vec3;
			double x{0};
			double y{0};
			double z{0};
		};

		void init(GLFWwindow* window);
		void moveInPlaneXZ(GLFWwindow* window, float dt, vs_game_object& game_object);

		//window callbacks
		static void window_in_focus_callback(GLFWwindow* window, int focused);
		static void mouse_scroll_callback(GLFWwindow*, double xoffset, double yoffset);


		key_mappings keys{};
		input last_mouse_input{};
		float move_speed{3.0f};
		float keyboard_look_speed{1.5f};
		float mouse_speed{1.7f};
	};
}
