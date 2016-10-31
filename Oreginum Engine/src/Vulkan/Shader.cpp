#include <fstream>
#include "../Oreginum/Core.hpp"
#include "Shader.hpp"

Oreginum::Vulkan::Shader::~Shader()
{ for(VkShaderModule m : modules) vkDestroyShaderModule(device->get(), m, nullptr); }

void Oreginum::Vulkan::Shader::add(const std::string& shader, VkShaderStageFlagBits stage)
{
	modules.push_back(create_shader_module(shader));

	VkPipelineShaderStageCreateInfo shader_stage_information;
	shader_stage_information.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_information.pNext = nullptr;
	shader_stage_information.flags = NULL;
	shader_stage_information.stage = stage;
	shader_stage_information.module = modules.back();
	shader_stage_information.pName = "main";
	shader_stage_information.pSpecializationInfo = nullptr;

	information.push_back(shader_stage_information);
}

VkShaderModule Oreginum::Vulkan::Shader::create_shader_module(const std::string& shader)
{
	std::ifstream file{"Resources/Shaders/"+shader+".spv", std::ios::ate | std::ios::binary};
	if(!file.is_open()) Oreginum::Core::error("Could not open shader \""+shader+"\".");
	size_t size{static_cast<size_t>(file.tellg())};
	file.seekg(0);
	std::vector<char> data(size);
	file.read(data.data(), size);
	file.close();

	VkShaderModuleCreateInfo shader_module_information;
	shader_module_information.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_information.pNext = nullptr;
	shader_module_information.flags = NULL;
	shader_module_information.codeSize = data.size();
	shader_module_information.pCode = reinterpret_cast<const uint32_t *>(data.data());

	VkShaderModule shader_module;
	if(vkCreateShaderModule(device->get(), &shader_module_information,
		nullptr, &shader_module) != VK_SUCCESS)
		Oreginum::Core::error("Could not create Vulkan shader module \""+shader+"\".");

	return shader_module;
}