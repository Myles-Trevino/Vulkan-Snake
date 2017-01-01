#include <Assimp/Importer.hpp>
#include <Assimp/scene.h>
#include <Assimp/postprocess.h>
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/matrix_inverse.hpp>
#include "Core.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"
#include "Model.hpp"
#include <iostream>

Oreginum::Model::Model(const std::string& model, const glm::fvec3& translation)
	: translation(translation)
{
	sampler = {Renderer::get_device()};
	Assimp::Importer importer;
	const aiScene *const scene{importer.ReadFile(model, aiProcess_FlipUVs)};
	if(!scene) Oreginum::Core::error("Could not load model \""+model+"\".");

	for(uint32_t i{}; i < scene->mNumMeshes; ++i)
	{
		Mesh mesh;

		aiString texture;
		scene->mMaterials[scene->mMeshes[i]->mMaterialIndex]->
			GetTexture(aiTextureType_DIFFUSE, 0, &texture);
		mesh.texture = {model.substr(0, model.find_last_of("/")+1)+texture.C_Str(), Texture::LINEAR};

		mesh.vertices.resize(scene->mMeshes[i]->mNumVertices);
		for(uint32_t j{}; j < mesh.vertices.size(); ++j)
		{
			aiVector3D& vertex{scene->mMeshes[i]->mVertices[j]},
				uv{scene->mMeshes[i]->mTextureCoords[0][j]},
				normal{scene->mMeshes[i]->mNormals[j]};
			mesh.vertices[j] = {vertex.x, vertex.y, vertex.z,
				uv.x, uv.y, normal.x, normal.y, normal.z};
		}

		mesh.indices.resize(scene->mMeshes[i]->mNumFaces*3);
		for(unsigned j{}, k{}; j < scene->mMeshes[i]->mNumFaces; ++j, k += 3)
		{
			aiFace& face{scene->mMeshes[i]->mFaces[j]};
			if(face.mNumIndices != 3)
				Oreginum::Core::error("The model \""+model+"\" is not properly triangulated.");
			mesh.indices[k] = face.mIndices[0];
			mesh.indices[k+1] = face.mIndices[1];
			mesh.indices[k+2] = face.mIndices[2];
		}

		mesh.vertex_buffer = {Renderer::get_device(), Renderer::get_temporary_command_buffer(),
			vk::BufferUsageFlagBits::eVertexBuffer, sizeof(Vertex)*mesh.vertices.size(),
			mesh.vertices.data()};
		mesh.index_buffer = {Renderer::get_device(), Renderer::get_temporary_command_buffer(),
			vk::BufferUsageFlagBits::eIndexBuffer, sizeof(uint32_t)*mesh.indices.size(),
			mesh.indices.data()};

		meshes.push_back(mesh);
	}
}

void Oreginum::Model::initialize_descriptor()
{
	for(auto& m : meshes)
	{
		m.descriptor_set = {Renderer::get_device(), Renderer::get_descriptor_pool(),
		{{vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eVertex},
		{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}};

		vk::DescriptorBufferInfo buffer_information
		{Renderer::get_uniform_buffer().get(), 0, Renderer::get_padded_uniform_size()};
		vk::DescriptorImageInfo image_information
		{sampler.get(), m.texture.get_image().get_view(), vk::ImageLayout::eShaderReadOnlyOptimal};

		m.descriptor_set.write({{vk::DescriptorType::eUniformBufferDynamic, &buffer_information,
			nullptr}, {vk::DescriptorType::eCombinedImageSampler, nullptr, &image_information}});
	}
}

void Oreginum::Model::update()
{
	uniforms.projection = Camera::get_projection();
	uniforms.view = Camera::get_view();
	uniforms.model = glm::translate(translation)*rotation*glm::rotate(
		glm::fmat4{}, glm::radians(90.0f), {1, 0, 0})*glm::scale(scale);

	uniforms.camera = Camera::get_position();
}

void Oreginum::Model::draw(const Vulkan::Command_Buffer& command_buffer, uint32_t descriptor_offset)
{
	command_buffer.get().bindPipeline(vk::PipelineBindPoint::eGraphics,
		Renderer::get_model_pipeline().get());
	for(auto& m : meshes)
	{
		command_buffer.get().bindVertexBuffers(0, m.vertex_buffer.get(), {0});
		command_buffer.get().bindIndexBuffer(m.index_buffer.get(), 0, vk::IndexType::eUint32);
		command_buffer.get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			Renderer::get_model_pipeline().get_layout(), 0,
			{m.descriptor_set.get()}, {descriptor_offset});
		command_buffer.get().drawIndexed(static_cast<uint32_t>(m.indices.size()), 1, 0, 0, 0);
	}
}