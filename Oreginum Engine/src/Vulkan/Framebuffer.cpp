#include "../Oreginum/Core.hpp"
#include "Framebuffer.hpp"

void Oreginum::Vulkan::Framebuffer::destroy()
{ if(framebuffer) vkDestroyFramebuffer(device->get(), framebuffer, nullptr); }

void Oreginum::Vulkan::Framebuffer::initialize(const Device *device,
	const Render_Pass& render_pass, Image_View image_view,
	Image_View depth_image_view, VkExtent2D extent)
{
	this->device = device;
	destroy();

	std::array<VkImageView, 2> attachments{image_view.get(), depth_image_view.get()};

	VkFramebufferCreateInfo framebuffer_information;
	framebuffer_information.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_information.pNext = nullptr;
	framebuffer_information.flags = NULL;
	framebuffer_information.renderPass = render_pass.get();
	framebuffer_information.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebuffer_information.pAttachments = attachments.data();
	framebuffer_information.width = extent.width;
	framebuffer_information.height = extent.height;
	framebuffer_information.layers = 1;

	if(vkCreateFramebuffer(device->get(), &framebuffer_information, nullptr, &framebuffer) !=
		VK_SUCCESS) Oreginum::Core::error("Could not create a Vulkan framebuffer.");
}