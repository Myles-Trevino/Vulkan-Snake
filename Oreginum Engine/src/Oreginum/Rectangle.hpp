#pragma once
#include <array>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/gtx/transform.hpp>
#include "Window.hpp"
#include "Renderable.hpp"
#include "../Vulkan/Buffer.hpp"
#include "../Vulkan/Command Buffer.hpp"
#include "../Vulkan/Pipeline.hpp"

namespace Oreginum
{
	class Rectangle : public Renderable
	{
	public:
		Rectangle(){}
		Rectangle(const glm::fvec2& translation, const glm::fvec2& scale,
			const glm::fvec3& color = {});

		void update();
		virtual void draw(const Vulkan::Descriptor_Set& descriptor_set,
			const Vulkan::Command_Buffer& command_buffer, uint32_t descriptor_offset);
		void translate(const glm::fvec2& translation){ this->translation += translation; }
		void set_translation(const glm::fvec2& translation){ this->translation = translation; }
		void set_color(const glm::fvec3 color){ uniforms.color = color; }
		
		const void* get_uniforms() const { return &uniforms; }
		const glm::fvec2& get_translation() const { return translation; }
		const glm::fvec3& get_color() const { return uniforms.color; }

	private:
		static constexpr std::array<float, 8> VERTICES{0, 0, 0, 1, 1, 1, 1, 0};
		static constexpr std::array<uint16_t, 6> INDICES{0, 1, 2, 2, 3, 0};
		static Vulkan::Buffer vertex_buffer, index_buffer;
		glm::fvec2 scale, translation;
		Uniforms uniforms;
	};
}