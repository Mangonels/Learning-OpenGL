#pragma once

#include "Mesh.h"

#include <SOIL.h>

#include <..\include\assimp\importer.hpp>
#include <..\include\assimp\scene.h>
#include <..\include\assimp\postprocess.h>

class Model
{
public:
	Model(const GLchar* path);
	//Metodos:
	void Draw(Shader shader, GLint DrawMode);
private:
	//Datos del modelo:
	vector<Mesh> meshes; //Aqui es donde se guardan todas las meshes
	string directory;
	//Metodos:
	void loadModel(string path); //Carga el archivo, se llama en el constructor.
	void processNode(aiNode* node, const aiScene*scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
	GLint Model::TextureFromFile(const char* path, string directory);
};

