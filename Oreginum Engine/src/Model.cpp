#include <Assimp/Importer.hpp>
#include <Assimp/scene.h>
#include <Assimp/postprocess.h>
#include "Core.hpp"
#include "Model.hpp"
#include <iostream>

Oreginum::Model::Model(const std::string& model)
{
	Assimp::Importer importer;
	const aiScene *const scene{importer.ReadFile(model, aiProcess_Triangulate)};
	if(!scene) Oreginum::Core::error("Could not load model \""+model+"\".");

	aiString texture;
	scene->mMaterials[scene->mMeshes[0]->mMaterialIndex]->
		GetTexture(aiTextureType_DIFFUSE, 0, &texture);
	this->texture = model.substr(0, model.find_last_of("/")+1)+texture.C_Str();

	vertices.resize(scene->mMeshes[0]->mNumVertices);
	for(int i{}; i < vertices.size(); ++i)
	{
		aiVector3D& vertex{scene->mMeshes[0]->mVertices[i]};
		aiVector3D& uv{scene->mMeshes[0]->mTextureCoords[0][i]};
		vertices[i] = {vertex.x, vertex.y, vertex.z, uv.x, uv.y};
	}

	indices.resize(scene->mMeshes[0]->mNumFaces*3);
	for(unsigned i{}, j{}; i < scene->mMeshes[0]->mNumFaces; ++i, j += 3)
	{
		aiFace& face{scene->mMeshes[0]->mFaces[i]};
		if(face.mNumIndices != 3)
			Oreginum::Core::error("The mesh \""+model+"\" is not properly triangulated.");
		indices[j] = face.mIndices[0];
		indices[j+1] = face.mIndices[1];
		indices[j+2] = face.mIndices[2];
	}
}