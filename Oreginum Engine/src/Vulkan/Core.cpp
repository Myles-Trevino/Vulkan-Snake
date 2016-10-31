#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <STB Image/stb_image.h>
#include "../Oreginum/Core.hpp"
#include "../Oreginum/Window.hpp"
#include "Swapchain.hpp"
#include "Command Pool.hpp"
#include "Render Pass.hpp"
#include "Descriptor.hpp"
#include "Pipeline.hpp"
#include "Command Buffer.hpp"
#include "Semaphore.hpp"
#include "Fence.hpp"
#include "Framebuffer.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "Core.hpp"

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

		VkCommandBufferBeginInfo command_buffer_information;
		command_buffer_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_information.pNext = nullptr;
		command_buffer_information.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		command_buffer_information.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(virtual_frame->command_buffer.get(), &command_buffer_information);

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color = {0, 0, 0, 1};
		clear_values[1].depthStencil = {1, 0};

		VkRenderPassBeginInfo render_pass_begin_information;
		render_pass_begin_information.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_information.pNext = nullptr;
		render_pass_begin_information.renderPass = render_pass.get();
		render_pass_begin_information.framebuffer = virtual_frame->framebuffer.get();
		render_pass_begin_information.renderArea = {{0, 0}, swapchain.get_extent()};
		render_pass_begin_information.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_begin_information.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(virtual_frame->command_buffer.get(),
			&render_pass_begin_information, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(virtual_frame->command_buffer.get(),
			VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get());

		VkViewport viewport{0, 0, static_cast<float>(swapchain.get_extent().width),
			static_cast<float>(swapchain.get_extent().height), 0, 1};
		VkRect2D scissor{{0, 0}, swapchain.get_extent()};

		vkCmdSetViewport(virtual_frame->command_buffer.get(), 0, 1, &viewport);
		vkCmdSetScissor(virtual_frame->command_buffer.get(), 0, 1, &scissor);

		VkDeviceSize offsets{0};
		std::array<VkBuffer, 1> buffers{vertex_buffer.get()};
		vkCmdBindVertexBuffers(virtual_frame->command_buffer.get(), 0,
			static_cast<uint32_t>(buffers.size()), buffers.data(), &offsets);

		vkCmdBindIndexBuffer(virtual_frame->command_buffer.get(),
			index_buffer.get(), 0, VK_INDEX_TYPE_UINT16);

		std::array<VkDescriptorSet, 1> descriptor_set{descriptor.get()};
		vkCmdBindDescriptorSets(virtual_frame->command_buffer.get(),
			VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_layout(), 0, 
			static_cast<uint32_t>(descriptor_set.size()), descriptor_set.data(), 0, nullptr);

		vkCmdDrawIndexed(virtual_frame->command_buffer.get(),
			static_cast<uint32_t>(model->get_indices().size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(virtual_frame->command_buffer.get());

		if(vkEndCommandBuffer(virtual_frame->command_buffer.get()) != VK_SUCCESS)
			Oreginum::Core::error("Could not record a Vulkan command buffer.");
	}

	void create_swapchain_dependants()
	{
		//Depth stencil
		depth_image.initialize(&device, {swapchain.get_extent().width,
			swapchain.get_extent().height}, Oreginum::Vulkan::Core::DEPTH_FORMAT, 
			VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		depth_image.transition(Oreginum::Vulkan::Core::DEPTH_FORMAT, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, NULL,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	void recreate_swapchain()
	{
		if(device.get()) vkDeviceWaitIdle(device.get());
		swapchain.initialize(&instance, &device);
		create_swapchain_dependants();
	}
}

VkCommandBuffer Oreginum::Vulkan::Core::begin_single_time_commands()
{
	VkCommandBufferAllocateInfo command_buffer_allocate_information;
	command_buffer_allocate_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_information.pNext = nullptr;
	command_buffer_allocate_information.commandPool = command_pool.get();
	command_buffer_allocate_information.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_information.commandBufferCount = 1;

	VkCommandBufferBeginInfo command_buffer_begin_information;
	command_buffer_begin_information.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_information.pNext = nullptr;
	command_buffer_begin_information.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	command_buffer_begin_information.pInheritanceInfo = nullptr;

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(device.get(), &command_buffer_allocate_information, &command_buffer);

	vkBeginCommandBuffer(command_buffer, &command_buffer_begin_information);

	return command_buffer;
}

void Oreginum::Vulkan::Core::end_single_time_commands(VkCommandBuffer command_buffer)
{
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_information;
	submit_information.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_information.pNext = nullptr;
	submit_information.waitSemaphoreCount = 0;
	submit_information.pWaitSemaphores = nullptr;
	submit_information.commandBufferCount = 1;
	submit_information.pCommandBuffers = &command_buffer;
	submit_information.signalSemaphoreCount = 0;
	submit_information.pSignalSemaphores = nullptr;

	vkQueueSubmit(device.get_graphics_queue(), 1, &submit_information, VK_NULL_HANDLE);
	vkQueueWaitIdle(device.get_graphics_queue());

	vkFreeCommandBuffers(device.get(), command_pool.get(), 1, &command_buffer);
}

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
	descriptor.create_layout(&device, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_VERTEX_BIT);
	Shader shader{&device};
	shader.add("Basic Vertex", VK_SHADER_STAGE_VERTEX_BIT);
	shader.add("Basic Fragment", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipeline.initialize(&device, shader, descriptor, render_pass);

	command_pool.initialize(&device, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, device.get_graphics_queue_family_index());
	for(auto& v : virtual_frames)
	{
		v.command_buffer.initialize(&device, &command_pool);
		v.available.initialize(&device);
		v.rendered.initialize(&device);
		v.fence.initialize(&device);
	}

	vertex_buffer.initialize(&device, model.get_vertices().data(),
		sizeof(model.get_vertices()[0])*
		model.get_vertices().size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	index_buffer.initialize(&device, model.get_indices().data(), sizeof(model.get_indices()[0])*
		model.get_indices().size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	uniform_buffer.initialize(&device, nullptr, uniform_buffer_object_size,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	descriptor.create_pool(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	descriptor.allocate_set();
	descriptor.write(uniform_buffer.get(), uniform_buffer_object_size,
		0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	create_swapchain_dependants();
}

void Oreginum::Vulkan::Core::destroy(){ if(device.get()) vkDeviceWaitIdle(device.get()); }

uint32_t Oreginum::Vulkan::Core::find_memory(uint32_t type, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(device.get_gpu(), &memory_properties);

	for(uint32_t i{}; i < memory_properties.memoryTypeCount; ++i)
		if((type & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties))
			return i;

	Oreginum::Core::error("Could not find suitable Vulkan memory.");
}

void Oreginum::Vulkan::Core::render()
{
	//Recreate swapchain on resize
	if(Window::was_resized()) recreate_swapchain();

	//Virtual frame
	static size_t virtual_frame_index{};
	virtual_frame_index = (virtual_frame_index+1)%virtual_frames.size();
	Virtual_Frame *virtual_frame = &virtual_frames[virtual_frame_index];
	std::array<VkFence, 1> fences{virtual_frame->fence.get()};
	vkWaitForFences(device.get(), static_cast<uint32_t>(fences.size()),
		fences.data(), VK_FALSE, UINT64_MAX);
	vkResetFences(device.get(), static_cast<uint32_t>(fences.size()), fences.data());

	//Swapchain image
	uint32_t image_index;
	VkResult result{vkAcquireNextImageKHR(device.get(), swapchain.get(), UINT64_MAX,
		virtual_frame->available.get(), VK_NULL_HANDLE, &image_index)};
	if(result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) recreate_swapchain();
	else if(result != VK_SUCCESS)
		Oreginum::Core::error("Could not aquire a Vulkan swapchain image.");

	//Prepare frame
	prepare_frame(virtual_frame, swapchain.get_image_views()[image_index]);

	//Submit render buffer
	std::array<VkSemaphore, 1> render_wait_semaphores{virtual_frame->available.get()};
	VkPipelineStageFlags wait_destination_stage_mask
	{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	std::array<VkCommandBuffer, 1> command_buffers{virtual_frame->command_buffer.get()};
	std::array<VkSemaphore, 1> signal_semaphores{virtual_frame->rendered.get()};
	VkSubmitInfo submit_information;
	submit_information.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_information.pNext = nullptr;
	submit_information.waitSemaphoreCount = static_cast<uint32_t>(render_wait_semaphores.size());
	submit_information.pWaitSemaphores = render_wait_semaphores.data();
	submit_information.pWaitDstStageMask = &wait_destination_stage_mask;
	submit_information.commandBufferCount = static_cast<uint32_t>(command_buffers.size());
	submit_information.pCommandBuffers = command_buffers.data();
	submit_information.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size());
	submit_information.pSignalSemaphores = signal_semaphores.data();

	if(vkQueueSubmit(device.get_graphics_queue(), 1,
		&submit_information, virtual_frame->fence.get())
		!= VK_SUCCESS) Oreginum::Core::error("Could not submit Vulkan render queue.");

	//Submit present queue
	std::array<VkSemaphore, 1> submit_wait_semaphores{virtual_frame->rendered.get()};
	std::array<VkSwapchainKHR, 1> swapchains{swapchain.get()};
	VkPresentInfoKHR present_information;
	present_information.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_information.pNext = nullptr;
	present_information.waitSemaphoreCount =
		static_cast<uint32_t>(submit_wait_semaphores.size());
	present_information.pWaitSemaphores = submit_wait_semaphores.data();
	present_information.swapchainCount = static_cast<uint32_t>(swapchains.size());
	present_information.pSwapchains = swapchains.data();
	present_information.pImageIndices = &image_index;
	present_information.pResults = nullptr;

	result = vkQueuePresentKHR(device.get_graphics_queue(), &present_information);
	if(result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) recreate_swapchain();
	else if(result != VK_SUCCESS)
		Oreginum::Core::error("Could not submit Vulkan presentation queue.");
}