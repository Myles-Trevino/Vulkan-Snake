#define NOMINMAX
#include <windows.h>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/gtc/matrix_transform.hpp>
#include "Oreginum/Core.hpp"
#include "Vulkan/Instance.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/Command Buffer.hpp"
#include "Vulkan/Swapchain.hpp"
#include "Vulkan/Image.hpp"
#include "Vulkan/Buffer.hpp"
#include "Vulkan/Descriptor Set.hpp"
#include "Vulkan/Render Pass.hpp"
#include "Vulkan/Shader.hpp"
#include "Vulkan/Framebuffer.hpp"
#include "Vulkan/Pipeline.hpp"
#include "Vulkan/Semaphore.hpp"

/*
	Reduce the number of device passes
	See about consolidating command pool and command buffer classes.
	Rename Command Pool to Command Buffer Pool.
	See about removing single time command pool.

	See about allocating image and buffer memory in larger chunks.
	Revise image layouts.
	Look into using multiple queues.
	Look into multithreding command buffer creation.
	Use push constants for uniforms.
	Optimize image creation.
*/

int WinMain(HINSTANCE current_instance, HINSTANCE previous_instance, LPSTR arguments, int show)
{
	//Core
	Oreginum::Core::initialize("Oreginum Engine Vulkan Test", {900, 900}, true);

	//Vulkan
	Oreginum::Model model{"Resources/Models/Suzanne/Suzanne.dae"};

	struct Uniforms{ glm::fmat4 model, view, projection; glm::fvec3 camera_position; } uniforms;

	Oreginum::Vulkan::Instance instance{true};
	Oreginum::Vulkan::Surface surface{instance};
	Oreginum::Vulkan::Device device{instance, surface};

	Oreginum::Vulkan::Command_Pool command_pool{device,
		device.get_graphics_queue_family_index()};
	Oreginum::Vulkan::Command_Buffer command_buffer{device, command_pool};

	Oreginum::Vulkan::Swapchain swapchain{instance, surface, &device, command_buffer};

	Oreginum::Vulkan::Image depth_buffer{device, command_buffer, swapchain.get_extent(), 
		Oreginum::Vulkan::Core::DEPTH_FORMAT, vk::ImageAspectFlagBits::eDepth,
		vk::ImageUsageFlagBits::eDepthStencilAttachment};

	Oreginum::Vulkan::Buffer uniform_buffer{device, command_pool,
		vk::BufferUsageFlagBits::eUniformBuffer, &uniforms, sizeof(uniforms)};

	Oreginum::Vulkan::Descriptor_Pool descriptor_pool{device,
		{{vk::DescriptorType::eUniformBuffer, 1}}};
	Oreginum::Vulkan::Descriptor_Set descriptor_set{device, descriptor_pool,
		vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex};

	Oreginum::Vulkan::Render_Pass render_pass{device};

	Oreginum::Vulkan::Shader shader{device,
		{{"Basic Vertex", vk::ShaderStageFlagBits::eVertex},
		{"Basic Fragment", vk::ShaderStageFlagBits::eFragment}}};

	std::vector<Oreginum::Vulkan::Framebuffer> framebuffers;
	for(const auto& i : swapchain.get_images()) framebuffers.push_back(
		{device, render_pass, i, depth_buffer, swapchain.get_extent()});

	Oreginum::Vulkan::Buffer vertex_buffer{device, command_pool,
		vk::BufferUsageFlagBits::eVertexBuffer, &model.get_vertices(),
		sizeof(model.get_vertices()[0])*model.get_vertices().size()};

	Oreginum::Vulkan::Buffer index_buffer{device, command_pool,
		vk::BufferUsageFlagBits::eIndexBuffer, &model.get_indices(),
		sizeof(model.get_indices()[0])*model.get_indices().size()};

	Oreginum::Vulkan::Pipeline pipeline{device, shader, descriptor_set, render_pass};

	Oreginum::Vulkan::Semaphore image_acquired{device};

	//Program
	while(Oreginum::Core::update())
	{
		//Get image index
		uint32_t image_index;
		vk::Result result{device.get().acquireNextImageKHR(swapchain.get(),
			UINT64_MAX, image_acquired.get(), VK_NULL_HANDLE, &image_index)};

		//Render
		std::array<vk::ClearValue, 2> clear_values;
		clear_values[0].setColor(std::array<int, 4>{1, 0, 0, 1});
		clear_values[1].setDepthStencil({1, 0});

		vk::RenderPassBeginInfo render_pass_information{render_pass.get(),
			framebuffers[image_index].get(), {{0, 0, swapchain.get_extent()}},
			static_cast<uint32_t>(clear_values.size()), clear_values.data()};

		command_buffer.get().beginRenderPass(render_pass_information,
			vk::SubpassContents::eInline);

		command_buffer.get().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.get());

		command_buffer.get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			pipeline.get_layout(), 0, descriptor_set.get(), {});

		command_buffer.get().bindVertexBuffers(0, vertex_buffer.get(), {});

		vk::Viewport viewport{0, 0, swapchain.get_extent().width,
			swapchain.get_extent().height, 0, 1};
		command_buffer.get().setViewport(0, viewport);

		vk::Rect2D scissor{{0, 0}, swapchain.get_extent()};
		command_buffer.get().setScissor(0, scissor);

		command_buffer.get().bindIndexBuffer(index_buffer.get(), 0, vk::IndexType::eUint16);

		command_buffer.get().drawIndexed(
			static_cast<uint32_t>(model.get_indices().size()), 1, 0, 0, 0);

		command_buffer.get().endRenderPass();
		command_buffer.get().end();
	}
}