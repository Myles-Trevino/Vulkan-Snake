#pragma once
#include <array>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/gtx/transform.hpp>
#include "../Vulkan/Pipeline.hpp"
#include "Renderable.hpp"

namespace Oreginum
{
	class Cuboid : public Renderable
	{
	public:
		Cuboid(){}
		Cuboid(const glm::fvec3& translation, const glm::fvec3& scale,
			const glm::fvec3& color = {}, bool center = true);

		void update();
		void draw(const Vulkan::Descriptor_Set& descriptor_set, const Vulkan::Pipeline& pipeline,
			const Vulkan::Command_Buffer& command_buffer, uint32_t offset);
		void translate(const glm::fvec3& translation){ this->translation += translation; }
		void set_translation(const glm::fvec3& translation){ this->translation = translation; }
		void rotate(float radians, const glm::fvec3& axis)
		{ rotation *= glm::rotate(radians, axis); }

		const Uniforms& get_uniforms() const { return uniforms; }
		const glm::fvec3 get_translation() const { return translation; }

	private:
		static constexpr std::array<float, 24> VERTICES{0, 0, 0, 0, 1,
			0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1};
		static constexpr std::array<uint16_t, 36> INDICES{0, 1, 2, 2, 3, 0, 3, 2, 6, 6, 7,
			3, 7, 6, 5, 5, 4, 7, 4, 5, 1, 2, 0, 4, 4, 0, 3, 3, 7, 4, 1, 5, 6, 6, 2, 1};
		static Vulkan::Buffer vertex_buffer, index_buffer;
		glm::fvec3 translation, scale;
		glm::fmat4 rotation;
		Uniforms uniforms;
		bool center;
	};
}