#include "material.h"

Material::Material(char *DiffPath, char*SpectPath, float Shini){

	this->Shininess = Shini; //Pasar el valor de brillo

	//TEXTURA DIFFUSA:
	//Generar la textura y pasarla a OpenGL:
	//1. Reserva de memoria (La estamos haciendo como propiedad de esta clase)
	//2. Apuntar el puntero de textura a la memoria reservada:
	glGenTextures(1, &TextDiff);
	glBindTexture(GL_TEXTURE_2D, TextDiff);
	//3.A, Opciones de Wrapping (Si no se hace nada queda negro) Serían para ESTA imagen:
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT); //Con esto haremos que la textura se repita si sobra espacio vacío.

	//3.B, Opciones de Filtrado (ES OBLIGATORIO especificar esto, para ancho y alto) Son para ESTA imagen:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//4.Cargar la imagen:
	int diffTexWidth, diffTexHeight; //Tiene que ser para ESTA imagen, no puede ser global.
									 //Cargar una imagen con SOIL:
	unsigned char* diffImage = SOIL_load_image(DiffPath, &diffTexWidth, &diffTexHeight, 0, SOIL_LOAD_RGB); //Directorio, tamañoX, tamañoY, (ni idea de que es esto, pero en 0 va bien), modo de color.
																												   //Especificar una imagen bidimensional para OpenGL:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, diffTexWidth, diffTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, diffImage); //target, level of detail, color mode, tamañoX, tamañoY, borde, formato pixel data, tipo del pixel data, puntero a la imagen cargada.
																												 //Pasarle la imagen especificada a OpenGL, OGL lo guarda en un array, cada elemento es un GL_TEXTUREN, donde la N final es el nombre de la textura (para futuras referencias):
	glBindTexture(GL_TEXTURE_2D, 0); //Pasa las texturas creadas a OpenGL, y las llama 0.
	//5.Liberar el puntero de textura.
	SOIL_free_image_data(diffImage);

	//TEXTURA SPECULAR:
	glGenTextures(1, &TextSpec);
	glBindTexture(GL_TEXTURE_2D, TextSpec);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	int specTexWidth, specTexHeight;
	unsigned char* specImage = SOIL_load_image(SpectPath, &specTexWidth, &specTexHeight, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, specTexWidth, specTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, specImage);
	glBindTexture(GL_TEXTURE_2D, 1);
	SOIL_free_image_data(specImage);

}


Material::~Material(){
}

void Material::SetMaterial(Shader *shad) {
	shad->USE();
	glUniform1i(glGetUniformLocation(shad->Program, "elMaterial.diffuse"), 0);
	glUniform1i(glGetUniformLocation(shad->Program, "elMaterial.specular"), 1);
}

void Material::SetShininess(Shader *shad) {
	shad->USE();
	glUniform1f(glGetUniformLocation(shad->Program, "elMaterial.shininess"), Shininess);
}

void Material::ActivateTextures() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->TextDiff);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->TextSpec);

}
