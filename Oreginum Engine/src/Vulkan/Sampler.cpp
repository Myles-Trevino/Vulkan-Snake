#include "../Oreginum/Core.hpp"
#include "Sampler.hpp"

Oreginum::Vulkan::Sampler::Sampler(const Device& device, vk::Filter filter,
	vk::SamplerMipmapMode mipmap_mode, vk::SamplerAddressMode address_mode) : device(&device)
{
	vk::SamplerCreateInfo sampler_information{{}, filter, filter, mipmap_mode, address_mode,
		address_mode, address_mode, 0, VK_TRUE, 16, VK_FALSE, vk::CompareOp::eAlways, 0,
		0, vk::BorderColor::eFloatTransparentBlack, VK_FALSE};
	if(device.get().createSampler(&sampler_information, nullptr, sampler.get()) !=
		vk::Result::eSuccess || !*sampler) Core::error("Could not create a Vulkan sampler.");
}

void Oreginum::Vulkan::Sampler::swap(Sampler *other)
{
	std::swap(device, other->device);
	std::swap(sampler, other->sampler);
}