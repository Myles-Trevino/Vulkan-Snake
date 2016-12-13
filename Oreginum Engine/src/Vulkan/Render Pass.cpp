#include "../Oreginum/Core.hpp"
#include "Swapchain.hpp"
#include "Render Pass.hpp"

Oreginum::Vulkan::Render_Pass::Render_Pass(const Device& device) : device(&device) 
{
	//Attachments
	vk::AttachmentDescription color_attachment_description;
	color_attachment_description.setFormat(Swapchain::FORMAT.format);
	color_attachment_description.setSamples(vk::SampleCountFlagBits::e1);
	color_attachment_description.setLoadOp(vk::AttachmentLoadOp::eClear);
	color_attachment_description.setStoreOp(vk::AttachmentStoreOp::eStore);
	color_attachment_description.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
	color_attachment_description.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	color_attachment_description.setInitialLayout(vk::ImageLayout::eUndefined);
	color_attachment_description.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	//Subpasses
	vk::AttachmentReference color_attachment_reference;
	color_attachment_reference.setAttachment(0);
	color_attachment_reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription subpass_description;
	subpass_description.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpass_description.setInputAttachmentCount(0);
	subpass_description.setPInputAttachments(nullptr);
	subpass_description.setColorAttachmentCount(1);
	subpass_description.setPColorAttachments(&color_attachment_reference);
	subpass_description.setPResolveAttachments(nullptr);
	subpass_description.setPDepthStencilAttachment(nullptr);
	subpass_description.setPreserveAttachmentCount(0);
	subpass_description.setPPreserveAttachments(nullptr);

	//Dependencies
	vk::SubpassDependency dependency;
	dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
	dependency.setDstSubpass(NULL);
	dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	dependency.setSrcAccessMask(vk::AccessFlags{});
	dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead |
		vk::AccessFlagBits::eColorAttachmentWrite);

	//Render pass
	std::array<vk::AttachmentDescription, 1> attachments{color_attachment_description};
	std::array<vk::SubpassDescription, 1> subpasses{subpass_description};
	std::array<vk::SubpassDependency, 1> dependencies{dependency};
	vk::RenderPassCreateInfo render_pass_information;
	render_pass_information.setAttachmentCount(static_cast<uint32_t>(attachments.size()));
	render_pass_information.setPAttachments(attachments.data());
	render_pass_information.setSubpassCount(static_cast<uint32_t>(subpasses.size()));
	render_pass_information.setPSubpasses(subpasses.data());
	render_pass_information.setDependencyCount(static_cast<uint32_t>(dependencies.size()));
	render_pass_information.setPDependencies(dependencies.data());

	if(device.get().createRenderPass(&render_pass_information,
		nullptr, render_pass.get()) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan render pass.");
}

Oreginum::Vulkan::Render_Pass::~Render_Pass()
{ if(render_pass.unique() && *render_pass) device->get().destroyRenderPass(*render_pass); }

void Oreginum::Vulkan::Render_Pass::swap(Render_Pass *other)
{
	std::swap(this->device, other->device);
	std::swap(this->render_pass, other->render_pass);
}