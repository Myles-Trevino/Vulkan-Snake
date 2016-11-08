#include "../Oreginum/Core.hpp"
#include "Descriptor Set.hpp"

Oreginum::Vulkan::Descriptor_Set::Descriptor_Set(const Device& device,
	const Descriptor_Pool& pool, vk::DescriptorType type,
	vk::ShaderStageFlags stage_flags) : device(device)
{
	//Create layout
	vk::DescriptorSetLayoutBinding descriptor_set_layout_binding{0, type, 1, stage_flags};

	vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_information
	{{}, 1, &descriptor_set_layout_binding};

	if(device.get().createDescriptorSetLayout(&descriptor_set_layout_information,
		nullptr, &layout) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not create a Vulkan descriptor set layout.");

	//Allocate set
	vk::DescriptorSetAllocateInfo descriptor_set_allocate_information{pool.get(), 1, &layout};

	if(device.get().allocateDescriptorSets(&descriptor_set_allocate_information,
		&descriptor_set) != vk::Result::eSuccess)
		Oreginum::Core::error("Could not allocate a Vulkan descriptor set.");
}

Oreginum::Vulkan::Descriptor_Set::~Descriptor_Set()
{ device.get().destroyDescriptorSetLayout(layout); }

void Oreginum::Vulkan::Descriptor_Set::write(vk::Buffer buffer)
{
	vk::DescriptorBufferInfo descriptor_buffer_information{buffer};

	vk::WriteDescriptorSet write_descriptor_set{descriptor_set, 0, 0,
		1, type, nullptr, &descriptor_buffer_information, nullptr};

	device.get().updateDescriptorSets(write_descriptor_set, {});
}