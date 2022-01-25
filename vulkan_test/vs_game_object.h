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


		glm::mat4 mat4()
		{
			auto transform = glm::translate(glm::mat4{1.f}, translation);
			transform = glm::rotate(transform, rotation.y, {0.f, 1.f, 0.f});
			transform = glm::rotate(transform, rotation.x, {1.f, 0.f, 0.f});
			transform = glm::rotate(transform, rotation.z, {0.f, 0.f, 1.f});
			transform = glm::scale(transform, scale);
			return transform;
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
