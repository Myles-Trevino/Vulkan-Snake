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

		virtual void draw(const Vulkan::Descriptor_Set& descriptor_set,
			const Vulkan::Pipeline& pipeline, const Vulkan::Command_Buffer& command_buffer,
			uint32_t offset) = 0;
		virtual void update() = 0;
		virtual const Uniforms& get_uniforms() const = 0;
	};
}