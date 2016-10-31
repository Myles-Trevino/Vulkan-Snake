#pragma once
#include <vector>

namespace Oreginum
{
	class Model
	{
	public:
		struct Vertex{ float x, y, z, nx, ny, nz, u, v; };

		Model(const std::string& model);

		const std::vector<Vertex>& get_vertices() const { return vertices; }
		const std::vector<uint16_t>& get_indices() const { return indices; }
		const std::string& get_texture() const { return texture; }

	private:
		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;
		std::string texture;
	};
}