#pragma once
#include <string>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/glm.hpp>
#include "../Vulkan/Image.hpp"

namespace Oreginum
{
	class Texture
	{
	public:
		enum Format{SRGB, LINEAR, HDR};

		Texture(){}
		Texture(const std::string& path, Format type = SRGB)
			: Texture(std::vector<std::string>{path}){}
		Texture(const std::vector<std::string>& paths,
			Format type = SRGB, bool cubemap = false);

		const Vulkan::Image& get_image() const { return image; }

	private:
		struct Data
		{
			glm::ivec2 resolution;
			int channels;
			stbi_uc *data;
		};

		Vulkan::Image image;
		Format type;

		vk::Format get_format();
	};
}