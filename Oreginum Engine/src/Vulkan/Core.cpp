#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <STB Image/stb_image.h>
#include "../Oreginum/Core.hpp"
#include "../Oreginum/Window.hpp"
#include "Swapchain.hpp"
#include "Render Pass.hpp"
#include "Descriptor Set.hpp"
#include "Pipeline.hpp"
#include "Command Buffer.hpp"
#include "Semaphore.hpp"
#include "Fence.hpp"
#include "Framebuffer.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "Core.hpp"

/*
namespace
{
	struct Virtual_Frame
	{
		Oreginum::Vulkan::Command_Buffer command_buffer;
		Oreginum::Vulkan::Semaphore available;
		Oreginum::Vulkan::Semaphore rendered;
		Oreginum::Vulkan::Fence fence;
		Oreginum::Vulkan::Framebuffer framebuffer;
	};

	bool debug;
	const Oreginum::Model *model;
	const void *uniform_buffer_object;
	size_t uniform_buffer_object_size;

	Oreginum::Vulkan::Instance instance;
	Oreginum::Vulkan::Device device;
	Oreginum::Vulkan::Swapchain swapchain;

	Oreginum::Vulkan::Command_Pool command_pool;

	Oreginum::Vulkan::Render_Pass render_pass;
	Oreginum::Vulkan::Descriptor descriptor;
	Oreginum::Vulkan::Pipeline pipeline;

	Oreginum::Vulkan::Buffer vertex_buffer;
	Oreginum::Vulkan::Buffer index_buffer;
	Oreginum::Vulkan::Buffer uniform_buffer;

	Oreginum::Vulkan::Image depth_image;

	std::array<Virtual_Frame, 3> virtual_frames;

	void prepare_frame(Virtual_Frame *virtual_frame, Oreginum::Vulkan::Image_View image_view)
	{
		//Update uniform buffer
		uniform_buffer.fill(uniform_buffer_object, uniform_buffer_object_size);

		virtual_frame->framebuffer.initialize(&device, render_pass,
			image_view, depth_image.get_view(), swapchain.get_extent());

		vk::CommandBufferBeginInfo command_buffer_information;
		command_buffer_information.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		command_buffer_information.setPInheritanceInfo(nullptr);

		virtual_frame->command_buffer.get().begin(command_buffer_information);

		std::array<vk::ClearValue, 2> clear_values;
		clear_values[0].setColor(std::array<int, 4>{1, 0, 0, 1});
		clear_values[1].setDepthStencil({1, 0});

		vk::RenderPassBeginInfo render_pass_begin_information;
		render_pass_begin_information.setRenderPass(render_pass.get());
		render_pass_begin_information.setFramebuffer(virtual_frame->framebuffer.get());
		render_pass_begin_information.setRenderArea({{0, 0}, swapchain.get_extent()});
		render_pass_begin_information.setClearValueCount(
			static_cast<uint32_t>(clear_values.size()));
		render_pass_begin_information.setPClearValues(clear_values.data());

		virtual_frame->command_buffer.get().beginRenderPass(
			render_pass_begin_information, vk::SubpassContents::eInline);

		virtual_frame->command_buffer.get().bindPipeline(
			vk::PipelineBindPoint::eGraphics, pipeline.get());

		virtual_frame->command_buffer.get().setViewport(0, vk::Viewport{0, 0,
			static_cast<float>(swapchain.get_extent().width),
			static_cast<float>(swapchain.get_extent().height), 0, 1});

		virtual_frame->command_buffer.get().setScissor(
			0, vk::Rect2D{{0, 0}, swapchain.get_extent()});

		virtual_frame->command_buffer.get().bindVertexBuffers(0, vertex_buffer.get(), {0});

		virtual_frame->command_buffer.get().bindIndexBuffer(
			index_buffer.get(), 0, vk::IndexType::eUint16);

		virtual_frame->command_buffer.get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			pipeline.get_layout(), 0, descriptor.get(), {});

		virtual_frame->command_buffer.get().drawIndexed(
			static_cast<uint32_t>(model->get_indices().size()), 1, 0, 0, 0);

		virtual_frame->command_buffer.get().endRenderPass();

		if(virtual_frame->command_buffer.get().end() != vk::Result::eSuccess)
			Oreginum::Core::error("Could not record a Vulkan command buffer.");
	}

	void create_swapchain_dependants()
	{
		//Depth stencil
		depth_image.initialize(&device, {swapchain.get_extent().width,
			swapchain.get_extent().height}, Oreginum::Vulkan::Core::DEPTH_FORMAT, 
			vk::ImageAspectFlagBits::eDepth, vk::ImageUsageFlagBits::eDepthStencilAttachment);
		depth_image.transition(Oreginum::Vulkan::Core::DEPTH_FORMAT, 
			vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal,
			vk::AccessFlagBits{}, vk::AccessFlagBits::eDepthStencilAttachmentRead |
			vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::ImageAspectFlagBits::eDepth);
	}

	void recreate_swapchain()
	{
		if(device.get()) vkDeviceWaitIdle(device.get());
		swapchain.initialize(&instance, &device);
		create_swapchain_dependants();
	}
}
*/

const vk::CommandBuffer& Oreginum::Vulkan::Core::begin_single_time_commands(
	const Device& device, const Command_Pool& command_pool)
{
	vk::CommandBufferAllocateInfo command_buffer_allocate_information;
	command_buffer_allocate_information.setCommandPool(command_pool.get());
	command_buffer_allocate_information.setLevel(vk::CommandBufferLevel::ePrimary);
	command_buffer_allocate_information.setCommandBufferCount(1);

	vk::CommandBufferBeginInfo command_buffer_begin_information;
	command_buffer_begin_information.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	command_buffer_begin_information.setPInheritanceInfo(nullptr);

	vk::CommandBuffer command_buffer;
	device.get().allocateCommandBuffers(&command_buffer_allocate_information, &command_buffer);

	command_buffer.begin(command_buffer_begin_information);

	return command_buffer;
}

void Oreginum::Vulkan::Core::end_single_time_commands(const Device& device,
	const Command_Pool& command_pool, const vk::CommandBuffer& command_buffer)
{
	command_buffer.end();

	vk::SubmitInfo submit_information;
	submit_information.setWaitSemaphoreCount(0);
	submit_information.setPWaitSemaphores(nullptr);
	submit_information.setCommandBufferCount(1);
	submit_information.setPCommandBuffers(&command_buffer);
	submit_information.setSignalSemaphoreCount(0);
	submit_information.setPSignalSemaphores(nullptr);

	device.get_graphics_queue().submit(submit_information, VK_NULL_HANDLE);
	device.get_graphics_queue().waitIdle();

	device.get().freeCommandBuffers(command_pool.get(), command_buffer);
}

uint32_t Oreginum::Vulkan::Core::find_memory(const Device& device,
	uint32_t type, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memory_properties(device.get_gpu().getMemoryProperties());

	for(uint32_t i{}; i < memory_properties.memoryTypeCount; ++i)
		if((type & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties))
			return i;

	Oreginum::Core::error("Could not find suitable Vulkan memory.");
}

/*
void Oreginum::Vulkan::Core::initialize(const Oreginum::Model& model,
const void *uniform_buffer_object,
size_t uniform_buffer_object_size, bool debug)
{
	::model = &model;
	::debug = debug;
	::uniform_buffer_object = uniform_buffer_object;
	::uniform_buffer_object_size = uniform_buffer_object_size;

	instance.initialize(debug);
	device.initialize(&instance);
	swapchain.initialize(&instance, &device);

	render_pass.initialize(&device);
	descriptor.create_layout(&device, 0, vk::DescriptorType::eUniformBuffer,
	vk::ShaderStageFlagBits::eVertex);
	Shader shader{&device};
	shader.add("Basic Vertex", vk::ShaderStageFlagBits::eVertex);
	shader.add("Basic Fragment", vk::ShaderStageFlagBits::eFragment);
	pipeline.initialize(&device, shader, descriptor, render_pass);

	command_pool.initialize(&device, vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
	vk::CommandPoolCreateFlagBits::eTransient, device.get_graphics_queue_family_index());
	for(auto& v : virtual_frames)
	{
	v.command_buffer.initialize(&device, &command_pool);
	v.available.initialize(&device);
	v.rendered.initialize(&device);
	v.fence.initialize(&device);
	}

	vertex_buffer.initialize(&device, vk::BufferUsageFlagBits::eVertexBuffer,
	model.get_vertices().data(), sizeof(model.get_vertices()[0])*
	model.get_vertices().size());
	index_buffer.initialize(&device, vk::BufferUsageFlagBits::eIndexBuffer,
	model.get_indices().data(), sizeof(model.get_indices()[0])*
	model.get_indices().size());
	uniform_buffer.initialize(&device, vk::BufferUsageFlagBits::eUniformBuffer,
	nullptr, uniform_buffer_object_size);

	descriptor.create_pool();
	descriptor.allocate_set();
	descriptor.write(uniform_buffer.get(), uniform_buffer_object_size, 0);

	create_swapchain_dependants();
}

void Oreginum::Vulkan::Core::destroy(){ if(device.get()) device.get().waitIdle(); }

void Oreginum::Vulkan::Core::render()
{
	//Recreate swapchain on resize
	if(Window::was_resized()) recreate_swapchain();

	//Virtual frame
	static size_t virtual_frame_index{};
	virtual_frame_index = (virtual_frame_index+1)%virtual_frames.size();
	Virtual_Frame *virtual_frame = &virtual_frames[virtual_frame_index];
	device.get().waitForFences(virtual_frame->fence.get(), VK_FALSE, UINT64_MAX);
	device.get().resetFences(virtual_frame->fence.get());

	//Swapchain image
	uint32_t image_index;
	vk::Result result{device.get().acquireNextImageKHR(swapchain.get(), UINT64_MAX,
		virtual_frame->available.get(), VK_NULL_HANDLE, &image_index)};
	if(result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR) 
		recreate_swapchain(); else if(result != vk::Result::eSuccess)
		Oreginum::Core::error("Could not aquire a Vulkan swapchain image.");

	//Prepare frame
	prepare_frame(virtual_frame, swapchain.get_image_views()[image_index]);

	//Submit render buffer
	std::array<vk::Semaphore, 1> render_wait_semaphores{virtual_frame->available.get()};
	vk::PipelineStageFlags mask{vk::PipelineStageFlagBits::eColorAttachmentOutput};
	std::array<vk::CommandBuffer, 1> command_buffers{virtual_frame->command_buffer.get()};
	std::array<vk::Semaphore, 1> signal_semaphores{virtual_frame->rendered.get()};
	vk::SubmitInfo submit_information;
	submit_information.setWaitSemaphoreCount(
		static_cast<uint32_t>(render_wait_semaphores.size()));
	submit_information.setPWaitSemaphores(render_wait_semaphores.data());
	submit_information.setPWaitDstStageMask(&mask);
	submit_information.setCommandBufferCount(static_cast<uint32_t>(command_buffers.size()));
	submit_information.setPCommandBuffers(command_buffers.data());
	submit_information.setSignalSemaphoreCount(static_cast<uint32_t>(signal_semaphores.size()));
	submit_information.setPSignalSemaphores(signal_semaphores.data());

	if(device.get_graphics_queue().submit(submit_information, virtual_frame->fence.get())
		!= vk::Result::eSuccess) Oreginum::Core::error("Could not submit Vulkan render queue.");

	//Submit present queue
	std::array<vk::Semaphore, 1> submit_wait_semaphores{virtual_frame->rendered.get()};
	std::array<vk::SwapchainKHR, 1> swapchains{swapchain.get()};
	vk::PresentInfoKHR present_information;
	present_information.setWaitSemaphoreCount(
		static_cast<uint32_t>(submit_wait_semaphores.size()));
	present_information.setPWaitSemaphores(submit_wait_semaphores.data());
	present_information.setSwapchainCount(static_cast<uint32_t>(swapchains.size()));
	present_information.setPSwapchains(swapchains.data());
	present_information.pImageIndices = &image_index;
	present_information.pResults = nullptr;

	result = device.get_graphics_queue().presentKHR(present_information);
	if(result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR) 
		recreate_swapchain(); else if(result != vk::Result::eSuccess)
		Oreginum::Core::error("Could not submit Vulkan presentation queue.");
}
*/