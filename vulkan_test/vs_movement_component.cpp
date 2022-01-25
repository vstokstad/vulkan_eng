﻿#include "vs_movement_component.h"

#include <iostream>

namespace vs
{
	vs_movement_component::vs_movement_component(GLFWwindow* window)
	{
		init(window);
	}

	void vs_movement_component::init(GLFWwindow* window)
	{
		glfwSetScrollCallback(window, mouse_scroll_callback);
		glfwSetWindowFocusCallback(window, window_in_focus_callback);

		if (glfwRawMouseMotionSupported())
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			std::cout << "using raw mouse input" << std::endl;
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
		}
	}


	void vs_movement_component::moveInPlaneXZ(GLFWwindow* window, float dt, vs_game_object& game_object)
	{
		glm::vec3 rotate{0};
		if (glfwGetInputMode(window, GLFW_RAW_MOUSE_MOTION) == GLFW_TRUE)
		{
			//mouse rotation
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			rotate.x += static_cast<float>(ypos - last_mouse_input.y) * -1.f;
			rotate.y += static_cast<float>(xpos - last_mouse_input.x);
			last_mouse_input.y = ypos;
			last_mouse_input.x = xpos;


			if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
			{
				game_object.transform.rotation += mouse_speed * dt * glm::normalize(rotate);
			}
			rotate = {0.f, 0.f, 0.f};
		}

		//keyboard rotation
		if (glfwGetKey(window, keys.look_right) == GLFW_PRESS) rotate.y += 1.f;
		if (glfwGetKey(window, keys.look_left) == GLFW_PRESS) rotate.y -= 1.f;
		if (glfwGetKey(window, keys.look_up) == GLFW_PRESS) rotate.x += 1.f;
		if (glfwGetKey(window, keys.look_down) == GLFW_PRESS) rotate.x -= 1.f;

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
		{
			game_object.transform.rotation += keyboard_look_speed * dt * glm::normalize(rotate);
		}


		//limit pitch and yaw;
		game_object.transform.rotation.x = glm::clamp(game_object.transform.rotation.x, -1.5f, 1.5f);
		game_object.transform.rotation.y = glm::mod(game_object.transform.rotation.y, glm::two_pi<float>());

		float yaw = game_object.transform.rotation.y;
		const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
		const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
		const glm::vec3 upDir{0.f, -1.f, 0.f};

		//movement
		glm::vec3 moveDir{0.f};
		if (glfwGetKey(window, keys.forward) == GLFW_PRESS)moveDir += forwardDir;
		if (glfwGetKey(window, keys.back) == GLFW_PRESS)moveDir -= forwardDir;
		if (glfwGetKey(window, keys.right) == GLFW_PRESS)moveDir += rightDir;
		if (glfwGetKey(window, keys.left) == GLFW_PRESS)moveDir -= rightDir;
		if (glfwGetKey(window, keys.up) == GLFW_PRESS)moveDir += upDir;
		if (glfwGetKey(window, keys.down) == GLFW_PRESS)moveDir -= upDir;

		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
		{
			game_object.transform.translation += move_speed * dt * glm::normalize(moveDir);
		}


		//other
		if (glfwGetKey(window, keys.escape) == GLFW_PRESS)
		{
			std::cout << "pressed escape and window is closing" << std::endl;
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}
	}

	void vs_movement_component::window_in_focus_callback(GLFWwindow* window, int focused)
	{
		if (focused == GLFW_FALSE)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
			std::cout << "window not focused, activating cursor." << std::endl;
		}
		else if (focused == GLFW_TRUE)
		{
			if (glfwRawMouseMotionSupported())
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
			}
			std::cout << "window in focus, deactivating cursor." << std::endl;
		}
	}

	void vs_movement_component::mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		//TODO implement this using the getUserWindowPointer to get a ref back to our non-static class.
		//last_mouse_input.z = yoffset;
		std::cout << "mouse scroll callback with yoffset: " << yoffset << std::endl;
	}
}
