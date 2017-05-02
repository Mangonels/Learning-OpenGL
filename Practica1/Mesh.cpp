#include "Mesh.h"

Mesh::Mesh(vector<Vertex> vert, vector<GLuint> ind, vector<Texture> text) {

	vertices = vert;
	indices = ind;
	textures = text;

	setupMesh();
}

void Mesh::setupMesh() { //Prepara la mesh para su renderizacion mediante la mandatoria inicializacion de vertices.
	//Creacion de buffsers de OGL:
	glGenVertexArrays(1, &VAO); //definiendo cual es el VAO
	glGenBuffers(1, &VBO); //definiendo cual es el VBO
	glGenBuffers(1, &EBO); //definiendo cual es el EBO

	//Rellenar y pasar los buffers a OGL:
	glBindVertexArray(VAO); {
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

		// Definicion de Vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		// Definicion de Vertex Normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
		// Definicion de Vertex Texture Coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

	}glBindVertexArray(0);
}

void Mesh::Draw(Shader MeshShader, GLint DrawMode) {

	GLuint diffuseNr = 1;
	GLuint specularNr = 1;
	for (GLuint i = 0; i < textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		stringstream ss; //comprueba si hay más de una textura por componente
		string number;
		string name = textures[i].type;
		if (name == "texture_diffuse")
			ss << diffuseNr++; // concatena en ss el numero de textura difusa que toca EJ: texture_diffise2
		else if (name == "texture_specular")
			ss << specularNr++; // concatena en ss el numero de textura especular que toca EJ:texture_specular2
		number = ss.str();
		glUniform1f(glGetUniformLocation(MeshShader.Program, ("material." + name + number).c_str()), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	// Draw mesh
	glBindVertexArray(VAO);
	glPolygonMode(GL_FRONT_AND_BACK, DrawMode);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	for (GLuint i = 0; i < this->textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}