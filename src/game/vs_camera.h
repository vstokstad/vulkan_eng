#pragma once


//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vs
{
	class vs_camera
	{
	public:
		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void setPerspectiveProjection(float fovy, float aspect, float near, float far);

		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
		void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4 getProjection() const { return projection_matrix; }
		const glm::mat4 getView() const { return view_matrix; }

	private:
		glm::mat4 projection_matrix{1.f};
		glm::mat4 view_matrix{1.f};
	};
}
