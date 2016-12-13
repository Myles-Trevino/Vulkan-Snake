#include "../Oreginum/Core.hpp"
#include "Framebuffer.hpp"

Oreginum::Vulkan::Framebuffer::Framebuffer(const Device& device, const Swapchain& swapchain, 
	const Render_Pass& render_pass, const Image& image) : device(&device)
{
	std::array<vk::ImageView, 1> attachments{image.get_view()};

	vk::FramebufferCreateInfo framebuffer_information{{}, render_pass.get(),
		static_cast<uint32_t>(attachments.size()), attachments.data(),
		swapchain.get_extent().width, swapchain.get_extent().height, 1};

	if(device.get().createFramebuffer(&framebuffer_information,
		nullptr, framebuffer.get()) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan framebuffer.");
}

Oreginum::Vulkan::Framebuffer::~Framebuffer()
{ if(framebuffer.unique() && *framebuffer) device->get().destroyFramebuffer(*framebuffer); }

void Oreginum::Vulkan::Framebuffer::swap(Framebuffer *other)
{
	std::swap(this->device, other->device);
	std::swap(this->framebuffer, other->framebuffer);
}