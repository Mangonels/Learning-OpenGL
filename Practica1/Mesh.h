#pragma once

#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <GL\glew.h>
#include <gtc\matrix_transform.hpp>
#include <glm.hpp>

#include <..\include\assimp\types.h>

using namespace glm;
using namespace std;

struct Vertex
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
};

struct Texture 
{
	GLuint id;
	string type;
	aiString path; //aiString requiere assimp types.h
};

class Mesh
{
public:
	//Datos de la mesh:
	vector<Vertex> vertices;
	vector<GLuint> indices;
	vector<Texture> textures;
	//Constructor y destructor de la clase:
	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures);
	//Funciones:
	void Draw(Shader shader, GLint DrawMode);
private:
	//Definicion de buffers de OGL:
	GLuint VAO, VBO, EBO;
	//Metodos:
	void setupMesh();
};

