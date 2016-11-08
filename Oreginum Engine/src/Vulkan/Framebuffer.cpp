#include "../Oreginum/Core.hpp"
#include "Framebuffer.hpp"

Oreginum::Vulkan::Framebuffer::~Framebuffer(){ device.get().destroyFramebuffer(framebuffer); }

Oreginum::Vulkan::Framebuffer::Framebuffer(const Device& device,
	const Render_Pass& render_pass, const Image& image,
	const Image& depth_image, vk::Extent2D extent) : device(device)
{
	std::array<vk::ImageView, 2> attachments{image.get_view(), depth_image.get_view()};

	vk::FramebufferCreateInfo framebuffer_information{{}, render_pass.get(),
		static_cast<uint32_t>(attachments.size()), attachments.data(),
		extent.width, extent.height, 1};

	if(device.get().createFramebuffer(&framebuffer_information,
		nullptr, &framebuffer) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan framebuffer.");
}