#pragma once
#include "vs_model.h"
#include	<memory>

//libs
#include	<glm/gtc/matrix_transform.hpp>

namespace vs
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::vec3 rotation;


		// Matrix corresponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4()
		{
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{translation.x, translation.y, translation.z, 1.0f}
			};
		}
	};

	struct RigidBody2dComponent
	{
		glm::vec2 velocity;
		float mass{1.0f};
	};

	class vs_game_object
	{
	public:
		using id_t = unsigned int;

		static vs_game_object createGameObject()
		{
			static id_t current_id = 0;
			return vs_game_object{current_id++};
		}

		vs_game_object& operator=(vs_game_object&&) = default;
		vs_game_object(vs_game_object&&) = default;
		vs_game_object(const vs_game_object&) = delete;
		vs_game_object& operator=(const vs_game_object&) = delete;

		id_t getId() { return id_; }

		std::shared_ptr<vs_model> model{};
		glm::vec3 color{};
		TransformComponent transform{};
		RigidBody2dComponent rigid_body2d{};
	private:
		vs_game_object(id_t obj_id) : id_(obj_id)
		{
		};
		const id_t id_;
	};
}
