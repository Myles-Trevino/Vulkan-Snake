#include "../Oreginum/Core.hpp"
#include "Descriptor.hpp"

void Oreginum::Vulkan::Descriptor::destroy()
{
	if(pool) vkDestroyDescriptorPool(device->get(), pool, nullptr);
	if(layout) vkDestroyDescriptorSetLayout(device->get(), layout, nullptr);
}

void Oreginum::Vulkan::Descriptor::create_layout(const Device *device,
	uint32_t binding, VkDescriptorType type, VkShaderStageFlags stage_flags,
	uint32_t count, VkSampler *immutable_samplers)
{
	this->device = device;
	this->type = type;
	destroy();

	VkDescriptorSetLayoutBinding descriptor_set_layout_binding;
	descriptor_set_layout_binding.binding = binding;
	descriptor_set_layout_binding.descriptorType = type;
	descriptor_set_layout_binding.descriptorCount = count;
	descriptor_set_layout_binding.stageFlags = stage_flags;
	descriptor_set_layout_binding.pImmutableSamplers = immutable_samplers;

	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_information;
	descriptor_set_layout_information.sType =
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_set_layout_information.pNext = nullptr;
	descriptor_set_layout_information.flags = NULL;
	descriptor_set_layout_information.bindingCount = 1;
	descriptor_set_layout_information.pBindings = &descriptor_set_layout_binding;

	if(vkCreateDescriptorSetLayout(device->get(), &descriptor_set_layout_information,
		nullptr, &layout) != VK_SUCCESS)
		Oreginum::Core::error("Could not create a Vulkan descriptor set layout.");
}

void Oreginum::Vulkan::Descriptor::create_pool(uint32_t count)
{
	VkDescriptorPoolSize descriptor_pool_size;
	descriptor_pool_size.type = type;
	descriptor_pool_size.descriptorCount = count;

	VkDescriptorPoolCreateInfo descriptor_pool_information;
	descriptor_pool_information.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_information.pNext = nullptr;
	descriptor_pool_information.flags = NULL;
	descriptor_pool_information.maxSets = 1;
	descriptor_pool_information.poolSizeCount = 1;
	descriptor_pool_information.pPoolSizes = &descriptor_pool_size;

	if(vkCreateDescriptorPool(device->get(), &descriptor_pool_information, nullptr, &pool) !=
		VK_SUCCESS) Oreginum::Core::error("Could not create a Vulkan descriptor pool.");
}

void Oreginum::Vulkan::Descriptor::allocate_set()
{
	VkDescriptorSetAllocateInfo descriptor_set_allocate_information;
	descriptor_set_allocate_information.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_allocate_information.pNext = nullptr;
	descriptor_set_allocate_information.descriptorPool = pool;
	descriptor_set_allocate_information.descriptorSetCount = 1;
	descriptor_set_allocate_information.pSetLayouts = &layout;

	if(vkAllocateDescriptorSets(device->get(), &descriptor_set_allocate_information, &set) !=
		VK_SUCCESS) Oreginum::Core::error("Could not allocate a Vulkan descriptor set.");
}

void Oreginum::Vulkan::Descriptor::write(VkBuffer buffer, VkDeviceSize range, uint32_t binding,
	VkDescriptorType descriptor_type, uint32_t descriptor_count, uint32_t element,
	VkDeviceSize offset)
{
	VkDescriptorBufferInfo descriptor_buffer_information;
	descriptor_buffer_information.buffer = buffer;
	descriptor_buffer_information.offset = offset;
	descriptor_buffer_information.range = range;

	VkWriteDescriptorSet write_descriptor_set;
	write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_set.pNext = nullptr;
	write_descriptor_set.dstSet = set;
	write_descriptor_set.dstBinding = binding;
	write_descriptor_set.dstArrayElement = element;
	write_descriptor_set.descriptorCount = descriptor_count;
	write_descriptor_set.descriptorType = descriptor_type;
	write_descriptor_set.pImageInfo = nullptr;
	write_descriptor_set.pBufferInfo = &descriptor_buffer_information;
	write_descriptor_set.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(device->get(), 1, &write_descriptor_set, 0, nullptr);
}