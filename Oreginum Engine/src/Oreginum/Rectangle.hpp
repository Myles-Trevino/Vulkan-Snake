#pragma once
#include <array>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtx/transform.hpp>
#include "Window.hpp"
#include "../Vulkan/Buffer.hpp"
#include "../Vulkan/Command Buffer.hpp"
#include "../Vulkan/Pipeline.hpp"

namespace Oreginum
{
	class Rectangle
	{
	public:
		Rectangle(){}
		Rectangle(const Vulkan::Device& device,
			const Vulkan::Command_Pool& temporary_command_pool,
			const glm::fvec2& scale, const glm::fvec2& position, const glm::fvec3& color);

		void translate(const glm::fvec2& translation)
		{ this->translation += translation; }
		void set_translation(const glm::fvec2& translation)
		{ this->translation = translation; }
		void set_color(const glm::fvec3 color){ this->color = color; }
		void draw(const Vulkan::Pipeline& pipeline,
			const Vulkan::Command_Buffer& command_buffer);

		const glm::fvec2& get_translation() const { return translation; }
		static uint32_t get_uniforms_size() { return sizeof(Uniforms); }

	private:
		static constexpr std::array<float, 8> VERTICES{0, 0, 0, 1, 1, 1, 1, 0};
		static constexpr std::array<uint16_t, 6> INDICES{0, 1, 2, 2, 3, 0};
		static Vulkan::Buffer vertex_buffer;
		static Vulkan::Buffer index_buffer;

		glm::fvec2 scale;
		glm::fvec2 translation;
		glm::fvec3 color;
		struct Uniforms{ glm::fmat4 matrix; glm::fvec3 color; } uniforms;
	};
}