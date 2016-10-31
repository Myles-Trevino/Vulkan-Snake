#pragma once
#include "Device.hpp"
#include "Render Pass.hpp"
#include "Image View.hpp"

namespace Oreginum::Vulkan
{
	class Framebuffer
	{
	public:
		Framebuffer(){};
		~Framebuffer(){ destroy(); };

		void initialize(const Device *device, const Render_Pass& render_pass, 
			Image_View image_view, Image_View depth_image_view,
			VkExtent2D extent);

		VkFramebuffer get() const { return framebuffer; }

	private:
		const Device *device;

		VkFramebuffer framebuffer;

		void destroy();
	};
}