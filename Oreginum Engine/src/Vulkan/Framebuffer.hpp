#pragma once
#include "Device.hpp"
#include "Render Pass.hpp"
#include "Image.hpp"
#include "Swapchain.hpp"

namespace Oreginum::Vulkan
{
	class Framebuffer
	{
	public:
		Framebuffer(){}
		Framebuffer(const Device& device, const Swapchain& swapchain,
			const Render_Pass& render_pass, const Image& image);
		Framebuffer *Framebuffer::operator=(Framebuffer other)
		{ swap(&other); return this; }
		~Framebuffer();

		const vk::Framebuffer& get() const { return *framebuffer; }

	private:
		const Device *device;
		std::shared_ptr<vk::Framebuffer> framebuffer = std::make_shared<vk::Framebuffer>();

		void swap(Framebuffer *other);
	};
}