#include "MeshImport.h"
#include "MeshDocument.h"

#include <core/ObjectProperties.h>
#include <core/PropertiesUtils.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

#include <fstream>

namespace
{

inline glm::mat4 convert(const aiMatrix4x4& in)
{
	glm::mat4 out(glm::uninitialize);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			out[i][j] = in[i][j];
	return out;
}

inline glm::vec3 convert(aiVector3D v)
{
	return glm::vec3{ v.x, v.y, v.z };
}

simplerender::Mesh::SPtr createMesh(const aiMesh* input)
{
	auto mesh = std::make_shared<simplerender::Mesh>();
	mesh->m_vertices.reserve(input->mNumVertices);
	for (unsigned int j = 0; j < input->mNumVertices; ++j)
		mesh->m_vertices.push_back(convert(input->mVertices[j]));

	if (input->mPrimitiveTypes == aiPrimitiveType_TRIANGLE)
	{
		mesh->m_triangles.reserve(input->mNumFaces);
		for (unsigned int j = 0; j < input->mNumFaces; ++j)
			mesh->m_triangles.push_back({ input->mFaces[j].mIndices[0], input->mFaces[j].mIndices[1], input->mFaces[j].mIndices[2] });

		mesh->m_normals.reserve(input->mNumVertices);
		for (unsigned int j = 0; j < input->mNumVertices; ++j)
			mesh->m_normals.push_back(convert(input->mNormals[j]));
	}
	else if (input->mPrimitiveTypes == aiPrimitiveType_LINE)
	{
		mesh->m_edges.reserve(input->mNumFaces);
		for (unsigned int j = 0; j < input->mNumFaces; ++j)
			mesh->m_edges.push_back({ input->mFaces[j].mIndices[0], input->mFaces[j].mIndices[1] });
	}

	if (input->mNumUVComponents[0] == 2)
	{
		mesh->m_texCoords.reserve(input->mNumVertices);
		for (unsigned int j = 0; j < input->mNumVertices; ++j)
		{
			auto t = input->mTextureCoords[0][j];
			mesh->m_texCoords.push_back(glm::vec2(t.x, t.y));
		}
	}

	return mesh;
}

inline simplerender::Material::Color convert(const aiColor3D& c)
{
	return simplerender::Material::Color(c.r, c.g, c.b, 1.0f);
}

simplerender::Material::SPtr createMaterial(const aiMaterial* input)
{
	auto material = std::make_shared<simplerender::Material>();

	aiColor3D color (0.f,0.f,0.f);
	float floatValue;
	aiString path;

	if(aiReturn_SUCCESS == input->Get(AI_MATKEY_COLOR_DIFFUSE, color))
		material->diffuse = convert(color);

	if(aiReturn_SUCCESS == input->Get(AI_MATKEY_COLOR_AMBIENT, color))
		material->ambient = convert(color);

	if(aiReturn_SUCCESS == input->Get(AI_MATKEY_COLOR_SPECULAR, color))
		material->specular = convert(color);

	if(aiReturn_SUCCESS == input->Get(AI_MATKEY_COLOR_EMISSIVE, color))
		material->emissive = convert(color);

	if(aiReturn_SUCCESS == input->Get(AI_MATKEY_SHININESS, floatValue))
		material->shininess = floatValue;

	if(aiReturn_SUCCESS == input->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path))
		material->textureFilename = path.C_Str();

	return material;
}

bool doesFileExist(const std::string& path)
{
	return std::ifstream(path).good();
}

}

MeshImport::MeshImport(MeshDocument* doc, simplerender::Scene& scene, Graph& graph)
	: m_document(doc)
	, m_scene(scene)
	, m_graph(graph)
{
}

std::pair<MeshImport::Meshes, MeshImport::Materials> MeshImport::importMeshes(const std::string& filePath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath,
	//	aiProcess_JoinIdenticalVertices |
	//	aiProcess_SortByPType |
		aiProcess_RemoveRedundantMaterials);

	if (scene)
	{
	//	importer.ApplyPostProcessing(aiProcess_Triangulate);
		parseScene(scene);
		findTextures(filePath);
	}

	return std::make_pair(m_newMeshes, m_newMaterials);
}

void MeshImport::parseNode(const aiScene* scene, const aiNode* aNode, const glm::mat4& transformation, MeshNode* parent)
{
	auto n = m_document->createNode(aNode->mName.C_Str(), MeshNode::Type::Node, parent);
	auto nodeTransformation = convert(aNode->mTransformation);
	n->transformationComponents = toTransformationComponents(nodeTransformation);
	auto accTrans = nodeTransformation * transformation;

	for (unsigned int i = 0; i < aNode->mNumMeshes; ++i)
		parseMeshInstance(scene, aNode->mMeshes[i], accTrans, n.get());

	for (unsigned int i = 0; i < aNode->mNumChildren; ++i)
		parseNode(scene, aNode->mChildren[i], accTrans, n.get());
}

void MeshImport::parseMeshInstance(const aiScene* scene, unsigned int id, const glm::mat4& transformation, MeshNode* parent)
{
	const auto inMesh = scene->mMeshes[id];
	const auto meshId = meshIndex(id);
	const auto materialId = materialIndex(inMesh->mMaterialIndex);

	auto n = m_document->createNode(inMesh->mName.C_Str(), MeshNode::Type::Instance, parent);
	n->meshId = meshId;
	n->materialId = materialId;
	n->mesh = meshId != -1 ? m_scene.meshes()[meshId] : nullptr;
	n->material = materialId != -1 ? m_scene.materials()[materialId] : nullptr;
	n->transformationMatrix = transformation;
	n->instance = std::make_shared<simplerender::ModelInstance>();
	n->instance->transformation = glm::transpose(transformation);
	n->instance->mesh = n->mesh;
	n->instance->material = n->material;
	m_scene.addInstance(n->instance);
}

void MeshImport::parseScene(const aiScene* scene)
{
	addMeshes(scene);
	addMaterials(scene);

	// Adding graph
	glm::mat4 transformation;
	auto root = dynamic_cast<MeshNode*>(m_graph.root());
	parseNode(scene, scene->mRootNode, transformation, root);
}

void MeshImport::addMeshes(const aiScene* scene)
{
	auto meshesGroup = m_document->meshesGroup();
	if (!meshesGroup)
		meshesGroup = m_graph.root();

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		const auto inMesh = scene->mMeshes[i];
		if (!inMesh->HasPositions() || !inMesh->HasFaces()) // Need vertices and faces
			continue;
		if (inMesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE && !inMesh->HasNormals()) // If triangles, need normals
			continue;
		if (inMesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE && inMesh->mPrimitiveTypes != aiPrimitiveType_LINE) // Accept either triangles, or lines
			continue;

		auto node = m_document->createNode(inMesh->mName.length ? inMesh->mName.C_Str() : "mesh " + std::to_string(i),
										   MeshNode::Type::Mesh,
										   meshesGroup);

		auto mesh = createMesh(inMesh);
		node->mesh = mesh;
		int index = m_scene.meshes().size();
		m_scene.addMesh(mesh);
		m_meshesIndices.emplace_back(i, index);
		m_newMeshes.push_back(mesh.get());
	}
}

void MeshImport::addMaterials(const aiScene* scene)
{
	auto materialsGroup = m_document->materialsGroup();
	if (!materialsGroup)
		materialsGroup = m_graph.root();

	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		const auto inMaterial = scene->mMaterials[i];

		aiString name;
		inMaterial->Get(AI_MATKEY_NAME,name);

		auto node = m_document->createNode(name.length ? name.C_Str() : "material " + std::to_string(i),
										   MeshNode::Type::Material,
										   materialsGroup);
		auto material = createMaterial(inMaterial);
		node->material = material;
		int index = m_scene.materials().size();
		m_scene.addMaterial(material);
		m_materialIndices.emplace_back(i, index);
		m_newMaterials.push_back(material.get());
	}
}

int MeshImport::meshIndex(int meshId)
{
	for (const auto& mesh : m_meshesIndices)
	{
		if (mesh.first == meshId)
			return mesh.second;
	}

	return -1;
}

int MeshImport::materialIndex(int materialId)
{
	for (const auto& material : m_materialIndices)
	{
		if (material.first == materialId)
			return material.second;
	}

	return -1;
}

void MeshImport::findTextures(const std::string& modelPath)
{
	// Find the repertory of the model file
	auto pos = modelPath.find_last_of("/\\");
	if (pos == std::string::npos)
		return;
	std::string dir = modelPath.substr(0, pos);

	// Modify the relative texture paths
	for (auto& material : m_newMaterials)
	{
		auto& fileName = material->textureFilename;
		if (!fileName.empty())
		{
			auto path = dir + "/" + fileName;
			if (doesFileExist(path))
				fileName = path;
		}
	}
}
