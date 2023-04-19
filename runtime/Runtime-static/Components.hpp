#pragma once
// Created by Omen on 04-12-2022.
//

#include "ENGINE/Application/ECS.hpp"
#include "DearImGui/Module.hpp"

#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <chrono>

namespace EndlessTunnel::DataComponents
{

	class Transform
	{
	  public:
		glm::vec3 position{};
		glm::vec3 rotation{};
		glm::vec3 scale = { 1.f, 1.f, 1.f };

		const SETU::ECS::Entity parent;

	  private:
		glm::mat4 transform = glm::mat4 (1.0f);

	  public:
		inline constexpr explicit Transform (const SETU::ECS::Entity entity = entt::null)
			  : parent (entity) {}

		void OnAwake (const SETU::ECS::Entity entity, const SETU::ECS::Registry &registry) {}

		void OnUpdate (const SETU::ECS::Entity entity, const SETU::ECS::Registry &registry) {
			transform = glm::translate (glm::mat4 (1.f), position)
			          * glm::scale (glm::mat4 (1.f), scale) * glm::toMat4 (glm::quat (rotation));
			if (parent != entt::null) {
				auto &parent_transform = registry.get<Transform> (parent);
				transform              = parent_transform.transform * transform;
			};
		}

		inline constexpr b8 OnEvent (
			SETU::Event_Base &event, const SETU::ECS::Entity entity,
			const SETU::ECS::Registry &registry
		) {
			return false;
		}

		void OnSleep (const SETU::ECS::Entity entity, const SETU::ECS::Registry &registry) {}

		inline constexpr const glm::mat4 &GetTransform () const noexcept { return transform; }
	};

	static constexpr u32 cube_mat_dimension = 5;

	struct CubeArranger {
		b8 matrix[cube_mat_dimension][cube_mat_dimension] = {
			{false, false, false, false, false},
			{false, false, false, false, false},
			{false, false, false, false, false},
			{false, false, false, false, false},
			{false, false, false, false, false}
		};

		inline constexpr void OnAwake (
			const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {}

		inline constexpr void OnUpdate (
			const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {}

		inline constexpr b8 OnEvent (
			SETU::Event_Base &event, const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {
			return false;
		}

		inline constexpr void OnSleep (
			const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {}
	};

	struct Cube {
		enum class TYPE : u8
		{
			NONE_BLOCKADE = 0,
			BLOCKADE      = 1,
			WALL          = 2,
			POINT         = 3
		};

		TYPE      Type;
		const u32 idx = 0;
		Cube (TYPE type, u32 idx)
			  : Type (type)
			  , idx (idx){};

		inline constexpr void OnAwake (
			const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {}

		inline constexpr void OnUpdate (
			const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {
			if (Type == TYPE::BLOCKADE || Type == TYPE::NONE_BLOCKADE) {
				auto &cube_arranger = registry.get<CubeArranger> (transform.parent);
				Type = (cube_arranger.matrix[idx / cube_mat_dimension][idx % cube_mat_dimension])
				         ? TYPE::BLOCKADE
				         : TYPE::NONE_BLOCKADE;
			}
		}

		inline constexpr b8 OnEvent (
			SETU::Event_Base &event, const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {
			return false;
		}

		inline constexpr void OnSleep (
			const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {}
	};

	class Camera
	{
	  private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		f32 m_FOV = -60.f;
	  public:
		inline constexpr void OnAwake (
			const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {}

		inline /*constexpr*/ void OnUpdate (
			const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {
			m_ViewMatrix = glm::inverse(transform.GetTransform());
			if (m_FOV < 0) {
				auto size = ImGui::GetMainViewport()->Size;
				f32  aspect = size.x/size.y;
				m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), aspect, 0.001f, 150.f);
			};
		}

		inline constexpr b8 OnEvent (
			SETU::Event_Base &event, const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {
			return false;
		}

		inline constexpr void OnSleep (
			const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {}

		[[nodiscard]] inline /*constexpr*/ glm::mat4 GetProjectionView() const {
			return m_ProjectionMatrix*m_ViewMatrix;
		}

	  public:
		inline constexpr void SetFOV (f32 fov) {
			m_FOV = -(fov > 0 ? fov:-fov);
		}
	};

	struct Script {
		u64 data;
		void (* const on_awake
		) (const SETU::ECS::Entity, Transform &, const SETU::ECS::Registry &, u64&);
		void (* const on_update
		) (const SETU::ECS::Entity, Transform &, const SETU::ECS::Registry &, u64&);
		b8 (* const on_event
		) (SETU::Event_Base &, const SETU::ECS::Entity, Transform &, const SETU::ECS::Registry &,
		   u64&);
		void (* const on_sleep
		) (const SETU::ECS::Entity, Transform &, const SETU::ECS::Registry &, u64&);

		inline constexpr void OnAwake (
			const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {
			if (on_awake != nullptr) on_awake (entity, transform, registry, data);
		}

		inline constexpr void OnUpdate (
			const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {
			if (on_update != nullptr) on_update (entity, transform, registry, data);
		}

		inline constexpr b8 OnEvent (
			SETU::Event_Base &event, const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {
			if (on_event != nullptr) return on_event (event, entity, transform, registry, data);
			else return false;
		}

		inline constexpr void OnSleep (
			const SETU::ECS::Entity entity, Transform &transform,
			const SETU::ECS::Registry &registry
		) noexcept {
			if (on_sleep != nullptr) on_sleep (entity, transform, registry, data);
		}

	};
}

// EOF: ENDLESSTUNNEL_COMPONENTS_HPP
