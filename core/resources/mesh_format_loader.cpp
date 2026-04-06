#include "mesh_format_loader.h"

#include "build/development-fbuild/_deps/assimp-src/code/AssetLib/3MF/3MFXmlTags.h"
#include "mesh.h"
#include "resource_loader.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <iostream>

namespace feather {

void MeshFormatLoader::_bind_members() {
	ClassDB::bind_method(&Type::recognize_extension, "recognize_extension");
}

MeshFormatLoader::MeshFormatLoader() : _importer { std::make_unique<Assimp::Importer>() } {
}

bool MeshFormatLoader::recognize_extension(const std::string& extension) const {
	return _importer->IsExtensionSupported(extension);
}

MeshFormatLoader::~MeshFormatLoader() = default;

std::shared_ptr<Resource> MeshFormatLoader::load(const Path& path) {
	const aiScene* scene = _importer->ReadFile(
			path.string(),
			aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "Assimp error: " << _importer->GetErrorString() << std::endl;
		return nullptr;
	}

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		uint32_t index_offset = vertices.size();

		vertices.reserve(mesh->mNumVertices);
		indices.reserve(mesh->mNumFaces * 3);

		for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
			Vertex vertex;
			vertex.position = Vector3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
			if (mesh->HasNormals()) {
				vertex.normal = Vector3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
			}
			if (mesh->mTextureCoords[0]) {
				vertex.uv = Vector2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y);
			}
			vertices.push_back(vertex);
		}

		for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
			aiFace face = mesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; k++) {
				indices.push_back(face.mIndices[k] + index_offset);
			}
		}
	}

	std::shared_ptr<ComplexMesh> complex_mesh = std::make_shared<ComplexMesh>();
	complex_mesh->set_mesh_data(vertices, indices);

	return complex_mesh;
}

INPLACE_REGISTER_BEGIN(MeshFormatLoader);
ClassDB::register_class<MeshFormatLoader>();
INPLACE_REGISTER_END(MeshFormatLoader);

} // namespace feather