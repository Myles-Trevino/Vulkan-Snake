#include "../Oreginum/Core.hpp"
#include "Core.hpp"
#include "Render Pass.hpp"

Oreginum::Vulkan::Render_Pass::Render_Pass(const Device& device) : device(device)
{
	//Attachments
	vk::AttachmentDescription color_attachment_description;
	color_attachment_description.setFormat(Oreginum::Vulkan::Core::SWAPCHAIN_FORMAT.format);
	color_attachment_description.setSamples(vk::SampleCountFlagBits::e1);
	color_attachment_description.setLoadOp(vk::AttachmentLoadOp::eClear);
	color_attachment_description.setStoreOp(vk::AttachmentStoreOp::eStore);
	color_attachment_description.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	color_attachment_description.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	color_attachment_description.setInitialLayout(vk::ImageLayout::eUndefined);
	color_attachment_description.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentDescription depth_attachment_description;
	depth_attachment_description.setFormat(Oreginum::Vulkan::Core::DEPTH_FORMAT);
	depth_attachment_description.setSamples(vk::SampleCountFlagBits::e1);
	depth_attachment_description.setLoadOp(vk::AttachmentLoadOp::eClear);
	depth_attachment_description.setStoreOp(vk::AttachmentStoreOp::eDontCare);
	depth_attachment_description.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	depth_attachment_description.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	depth_attachment_description.setInitialLayout
	(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	depth_attachment_description.setFinalLayout
	(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	//Subpasses
	vk::AttachmentReference color_attachment_reference;
	color_attachment_reference.setAttachment(0);
	color_attachment_reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference depth_attachment_reference;
	depth_attachment_reference.setAttachment(1);
	depth_attachment_reference.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass_description;
	subpass_description.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpass_description.setInputAttachmentCount(0);
	subpass_description.setPInputAttachments(nullptr);
	subpass_description.setColorAttachmentCount(1);
	subpass_description.setPColorAttachments(&color_attachment_reference);
	subpass_description.setPResolveAttachments(nullptr);
	subpass_description.setPDepthStencilAttachment(&depth_attachment_reference);
	subpass_description.setPreserveAttachmentCount(0);
	subpass_description.setPPreserveAttachments(nullptr);

	//Dependencies
	std::array<vk::SubpassDependency, 2> dependencies;
	dependencies[0].setSrcSubpass(VK_SUBPASS_EXTERNAL);
	dependencies[0].setDstSubpass(0);
	dependencies[0].setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
	dependencies[0].setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	dependencies[0].setSrcAccessMask(vk::AccessFlagBits::eMemoryRead);
	dependencies[0].setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
	dependencies[0].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

	dependencies[1].setSrcSubpass(0);
	dependencies[1].setDstSubpass(VK_SUBPASS_EXTERNAL);
	dependencies[1].setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	dependencies[1].setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	dependencies[1].setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
	dependencies[1].setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
	dependencies[1].setDependencyFlags(vk::DependencyFlagBits::eByRegion);

	//Render pass
	std::array<vk::AttachmentDescription, 2> attachments
	{color_attachment_description, depth_attachment_description};
	vk::RenderPassCreateInfo render_pass_information;
	render_pass_information.setAttachmentCount(static_cast<uint32_t>(attachments.size()));
	render_pass_information.setPAttachments(attachments.data());
	render_pass_information.setSubpassCount(1);
	render_pass_information.setPSubpasses(&subpass_description);
	render_pass_information.setDependencyCount(static_cast<uint32_t>(dependencies.size()));
	render_pass_information.setPDependencies(dependencies.data());

	if(device.get().createRenderPass(&render_pass_information,
		nullptr, &render_pass) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan render pass.");
}