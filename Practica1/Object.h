#pragma once
#include <GL\glew.h>
#include<glm.hpp>
#include <gtc\matrix_transform.hpp>
using namespace glm;
#include "Shader.h"

enum FigureType {
	cube = 0,
};
class Object {

public:
	Object(vec3 theScale, vec3 theRotation, vec3 thePosition); //cube object generator with transformations

	void Scale(vec3 theScale);
	void Rotate(vec3 theRotation);
	void Translate(vec3 theTranslation);

	mat4 generateModelMatrix();
	vec3 getPosition();
	void drawCube();
	void cleanupCube();

private:
	GLuint VBO, VAO, EBO;
	vec3 position;
	vec3 rotation;
	vec3 scaling;
};