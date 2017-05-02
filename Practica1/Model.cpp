#include "Model.h"

Model::Model(const GLchar* path)
{
	this->loadModel(path);
}

void Model::Draw(Shader shader, GLint drawMode) {
	for (GLuint i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader, drawMode);
}

void Model::loadModel(string path) {
	Assimp::Importer importer;

	//aiProcess_GenNormals : actually creates normals for each vertex if the model didn't contain normal vectors.
	//aiProcess_SplitLargeMeshes : splits large meshes into smaller sub-meshes which is useful if your rendering has a maximum number of vertices allowed and can only process smaller meshes.
	//aiProcess_OptimizeMeshes : actually does the reverse by trying to join several meshes into one larger mesh, reducing drawing calls for optimization.

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
	// Process all the node's meshes (if any)
	for (GLuint i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	// Then do the same for each of its children
	for (GLuint i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene);
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;

	for (GLuint i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		// Process vertex positions, normals and texture coordinates
		vec3 vectAux;
		vectAux.x = mesh->mVertices[i].x;
		vectAux.y = mesh->mVertices[i].y;
		vectAux.z = mesh->mVertices[i].z;
		vertex.Position = vectAux;

		if (mesh->HasNormals()) {
			vectAux.x = mesh->mNormals[i].x;
			vectAux.y = mesh->mNormals[i].y;
			vectAux.z = mesh->mNormals[i].z;

			vertex.Normal = vectAux;
		}
		vec2 vec = vec2(0.0f, 0.0f);
		if (mesh->mTextureCoords[0]) {// Comprobamos que tenga textura
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
		}
		vertex.TexCoords = vec;
		vertices.push_back(vertex);
	}
	// Process indices
	for (GLuint i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// Process material
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
	vector<Texture> textures;
	for (GLuint i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		Texture texture;
		texture.id = TextureFromFile(str.C_Str(), directory);
		texture.type = typeName;
		texture.path = str;
		textures.push_back(texture);
	}
	return textures;
}

GLint Model::TextureFromFile(const char* path, string directory) {
	//Generate texture ID and load texture data 
	string filename = string(path);
	filename = directory + '/' + filename;
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}