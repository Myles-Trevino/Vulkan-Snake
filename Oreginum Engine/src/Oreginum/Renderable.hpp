#pragma once
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/glm.hpp>
#include "../Vulkan/Pipeline.hpp"

namespace Oreginum
{
	class Renderable
	{
	public:
		struct Uniforms{ glm::fmat4 matrix; glm::fvec3 color; };

		struct Model_Uniforms
		{
			glm::fmat4 model, view, projection;
			glm::fvec3 camera;
		};

		enum Type{PRIMITIVE_2D, SPRITE, PRIMITIVE_3D, MODEL, ENVIRONMENT};

		virtual void initialize_descriptor(){};
		virtual void draw(const Vulkan::Descriptor_Set& descriptor_set,
			const Vulkan::Command_Buffer& command_buffer, uint32_t descriptor_offset){};
		virtual void draw(const Vulkan::Command_Buffer& command_buffer,
			uint32_t descriptor_offset){};
		virtual void update() = 0;

		virtual const void* get_uniforms() const = 0;
		virtual const uint32_t get_uniforms_size() const { return sizeof(Uniforms); };
		virtual int get_type() const { return PRIMITIVE_2D; }
		virtual uint32_t get_meshes() const { return 1; }
	};
}