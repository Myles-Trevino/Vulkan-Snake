#pragma once
#include "Device.hpp"
#include "Render Pass.hpp"
#include "Image.hpp"

namespace Oreginum::Vulkan
{
	class Framebuffer
	{
	public:
		Framebuffer(const Device& device, const Render_Pass& render_pass, 
			const Image& image, const Image& depth_image, vk::Extent2D extent);
		~Framebuffer();

		const vk::Framebuffer& get() const { return framebuffer; }

	private:
		const Device& device;

		vk::Framebuffer framebuffer;
	};
}