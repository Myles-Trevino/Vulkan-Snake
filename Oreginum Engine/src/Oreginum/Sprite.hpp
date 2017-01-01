#pragma once
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/gtx/transform.hpp>
#include "Renderable.hpp"
#include "Texture.hpp"
#include "../Vulkan/Sampler.hpp"

namespace Oreginum
{
	class Sprite : public Renderable
	{
	public:
		Sprite(){}
		Sprite(const std::string& path, const glm::fvec2& translation, const glm::fvec2& scale);

		void initialize_descriptor();
		void update();
		void draw(const Vulkan::Command_Buffer& command_buffer, uint32_t descriptor_offset);

		const void* get_uniforms() const { return &uniforms; }
		int get_type() const { return SPRITE; }
		bool is_textured() const { return true; }

	private:
		static constexpr std::array<float, 8> VERTICES{0, 0, 0, 1, 1, 1, 1, 0};
		static constexpr std::array<uint16_t, 6> INDICES{0, 1, 2, 2, 3, 0};
		static Vulkan::Buffer vertex_buffer, index_buffer;
		glm::fvec2 scale, translation;
		Texture texture;
		Vulkan::Sampler sampler;
		Vulkan::Descriptor_Set descriptor_set;
		Uniforms uniforms;
	};
}