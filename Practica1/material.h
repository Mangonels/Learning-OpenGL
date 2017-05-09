#pragma once
#define GLEW_STATIC
#include <GL\glew.h>
#include <SOIL.h>
#include "Shader.h"

class Material{

public:
	Material(char *DiffPath, char*SpectPath, float Shini);
	~Material();
	void SetMaterial(Shader *shad);
	void ActivateTextures();
	void SetShininess(Shader *shad);
private:
	GLuint TextDiff, TextSpec;
	float Shininess;

};

