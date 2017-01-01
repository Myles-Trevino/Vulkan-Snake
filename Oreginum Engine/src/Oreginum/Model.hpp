#pragma once
#include <vector>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/gtx/transform.hpp>
#include "Renderable.hpp"
#include "Texture.hpp"

namespace Oreginum
{
	class Model : public Renderable
	{
	public:
		struct Vertex{ float x, y, z, u, v, nx, ny, nz; };

		Model(const std::string& model, const glm::fvec3& translation = {});

		void initialize_descriptor();
		void update();
		void draw(const Vulkan::Command_Buffer& command_buffer, uint32_t descriptor_offset);
		void rotate(float radians, const glm::fvec3& axis)
		{ rotation *= glm::rotate(radians, axis); }

		const void *get_uniforms() const { return &uniforms; }
		const uint32_t get_uniforms_size() const { return sizeof(Model_Uniforms); }
		int get_type() const { return MODEL; }
		uint32_t get_meshes() const { return static_cast<uint32_t>(meshes.size()); }

	private:
		struct Mesh
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
			Vulkan::Buffer vertex_buffer, index_buffer;
			Texture texture;
			Vulkan::Descriptor_Set descriptor_set;
		};

		Model_Uniforms uniforms;
		glm::fvec3 translation, scale{1};
		glm::fmat4 rotation;
		Vulkan::Sampler sampler;
		std::vector<Mesh> meshes;
	};
}