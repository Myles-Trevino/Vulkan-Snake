#include "Core.hpp"
#include "Render Pass.hpp"

void Oreginum::Vulkan::Render_Pass::destroy()
{ if(render_pass) vkDestroyRenderPass(device->get(), render_pass, nullptr); }

void Oreginum::Vulkan::Render_Pass::initialize(const Device *device)
{
	this->device = device;
	destroy();

	//Attachments
	VkAttachmentDescription color_attachment_description;
	color_attachment_description.flags = NULL;
	color_attachment_description.format = Oreginum::Vulkan::Core::SWAPCHAIN_FORMAT.format;
	color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depth_attachment_description;
	depth_attachment_description.flags = NULL;
	depth_attachment_description.format = Oreginum::Vulkan::Core::DEPTH_FORMAT;
	depth_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment_description.initialLayout =
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depth_attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//Subpasses
	VkAttachmentReference color_attachment_reference;
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_reference;
	depth_attachment_reference.attachment = 1;
	depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass_description;
	subpass_description.flags = NULL;
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.inputAttachmentCount = 0;
	subpass_description.pInputAttachments = nullptr;
	subpass_description.colorAttachmentCount = 1;
	subpass_description.pColorAttachments = &color_attachment_reference;
	subpass_description.pResolveAttachments = nullptr;
	subpass_description.pDepthStencilAttachment = &depth_attachment_reference;
	subpass_description.preserveAttachmentCount = 0;
	subpass_description.pPreserveAttachments = nullptr;

	//Dependencies
	std::array<VkSubpassDependency, 2> dependencies;
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	//Render pass
	std::array<VkAttachmentDescription, 2> attachments
	{color_attachment_description, depth_attachment_description};
	VkRenderPassCreateInfo render_pass_information;
	render_pass_information.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_information.pNext = nullptr;
	render_pass_information.flags = NULL;
	render_pass_information.attachmentCount = static_cast<uint32_t>(attachments.size());
	render_pass_information.pAttachments = attachments.data();
	render_pass_information.subpassCount = 1;
	render_pass_information.pSubpasses = &subpass_description;
	render_pass_information.dependencyCount = static_cast<uint32_t>(dependencies.size());
	render_pass_information.pDependencies = dependencies.data();

	vkCreateRenderPass(device->get(), &render_pass_information, nullptr, &render_pass);
}