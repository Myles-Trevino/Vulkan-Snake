#pragma once
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/gtx/transform.hpp>
#include "Renderable.hpp"
#include "Texture.hpp"
#include "../Vulkan/Sampler.hpp"

namespace Oreginum
{
	class Environment : public Renderable
	{
	public:
		Environment(){}
		Environment(const std::string& path);

		void initialize_descriptor();
		void update();
		void draw(const Vulkan::Command_Buffer& command_buffer, uint32_t descriptor_offset);

		const void* get_uniforms() const { return &uniforms; }
		const uint32_t get_uniforms_size() const { return sizeof(Model_Uniforms); }
		int get_type() const { return ENVIRONMENT; }
		bool is_textured() const { return true; }

	private:
		static constexpr std::array<float, 8> VERTICES{-1, -1, -1, 1, 1, 1, 1, -1};
		static constexpr std::array<uint16_t, 6> INDICES{0, 1, 2, 2, 3, 0};
		static Vulkan::Buffer vertex_buffer, index_buffer;
		Texture texture;
		Vulkan::Sampler sampler;
		Vulkan::Descriptor_Set descriptor_set;
		Model_Uniforms uniforms;
	};
}