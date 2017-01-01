#include "Core.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <STB IMAGE/stb_image.h>

Oreginum::Texture::Texture(const std::vector<std::string>& paths,
	Format type, bool cubemap) : type(type)
{
	glm::ivec2 resolution;
	std::vector<stbi_uc *> datas;
	for(int i{}; i < paths.size(); ++i)
	{
		Data data;
		data.data = stbi_load(paths[i].c_str(), &data.resolution.x,
			&data.resolution.y, &data.channels, STBI_rgb_alpha);
		if(!data.data) Core::error("Could not load image \""+paths[i]+"\".");

		datas.push_back(data.data);
		if(i == 0) resolution = data.resolution; else if(resolution != data.resolution)
			Core::error("Could not load image array because "
				"\""+paths[i]+"\" is a different resolution.");
	}

	image = Vulkan::Image{Renderer::get_device(), Renderer::get_temporary_command_buffer(),
		resolution, datas, get_format(), cubemap};

	for(stbi_uc *d : datas) stbi_image_free(d);
}

vk::Format Oreginum::Texture::get_format()
{
	return (type == SRGB) ? Vulkan::Image::SRGB_TEXTURE_FORMAT : (type == LINEAR) ?
		Vulkan::Image::LINEAR_TEXTURE_FORMAT : Vulkan::Image::HDR_TEXTURE_FORMAT;
}