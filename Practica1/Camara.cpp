#pragma once

#include "Camara.h"

#include <vector>

#include <iostream>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

//Variable externa que necesitamos para algunos metodos:
extern bool keys[1024];

glm::vec3 worldUpVector = glm::vec3(0.0f, 1.0f, 0.0f); //Un vector unitario que apunta hacia arriba, lo usamos para ayudar a calcular los vectores de la camara.

Camara::Camara(glm::vec3 position, glm::vec3 direction, GLfloat sensitivity, GLfloat fov)
{
	this->cameraPos = position;
	this->cameraFront = direction;
	this->cameraUp = worldUpVector;
	this->Sensitivity = sensitivity;
	this->FOV = fov;
	this->maxFov = fov + 50;
	this->minFov = fov - 50;
}

Camara::~Camara()
{
}

void Camara::DoMovement() //Inputs de movimiento de camara
{
	GLfloat cameraSpeed = 5.0f * Deltatime;

	//std::cout << "Eje 1: " << this->cameraPos.b << "Eje 2: " << this->cameraPos.g << std::endl;

	if (keys[GLFW_KEY_A]) { //Mirar si en el array de estados de key inputs esta activada esta key.
		this->cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; //Mover camara a izquierda = Restar a la posicion de la camara el cross product entre vector superior y delantero, dando así el vector perpendicular en el eje X. Todo esto se normaliza y se multiplica por la velocidad.
	}
	if (keys[GLFW_KEY_D]) {
		this->cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; //Mover camara a derecha = Lo mismo que izquierda pero restando.
	}
	if (keys[GLFW_KEY_W]) {
		this->cameraPos += cameraSpeed * cameraFront; //Mover camara adelante = Sumar a la posición de la camara el vector Z * velocidad
	}
	if (keys[GLFW_KEY_S]) {
		this->cameraPos -= cameraSpeed * cameraFront; //Mover camara atras = Restar a la posición de la camara el vector Z * velocidad
	}
}

void Camara::MouseMove(double xpos, double ypos)//Metodo que controla la rotacion de la camara segun movimiento del raton
{

	//Actualizando diferencia entre posicion anterior y actual del ratón, guardada en mouseXdiff

	if (firstMouse) { //Evita salto de camara al inicio
		this->LastMx = xpos;
		this->LastMy = ypos;
		firstMouse = false;
	}

	GLfloat mouseDiffX = xpos - this->LastMx;
	GLfloat mouseDiffY = this->LastMy - ypos;
	this->LastMx = xpos;
	this->LastMy = ypos;

	mouseDiffX *= Camara::Sensitivity;
	mouseDiffY *= Camara::Sensitivity;

	//Aumentar los angulos yaw y pitch según el offset entre posiciones del ratón:
	this->YAW = glm::mod(YAW + mouseDiffX, 360.0f); //Vamos a asegurarnos de que este valor no crece o decrementa demasiado con el mod de 360º
	this->PITCH += mouseDiffY;

	//Limitador de angulo vertical de camara:
	if (this->PITCH > 89.0f) this->PITCH = 89.0f;
	if (this->PITCH < -89.0f) this->PITCH = -89.0f;

	//Asignación de angulo de camara (Insercion de valores para el vector cameraFront que a su vez necesita la matriz lookAt):
	glm::vec3 front;
	//Calculo de angulo pitch(Y-X/Z):      Calculo de angulo yaw(Z-X):
	front.x = cos(glm::radians(this->YAW))       *       cos(glm::radians(this->PITCH));
	front.y = sin(glm::radians(this->PITCH));
	front.z = sin(glm::radians(this->YAW))       *       cos(glm::radians(this->PITCH));
	this->cameraFront = glm::normalize(front); //Normalizamos el vector y lo asignamos como eje frontal, que luego se aplica a la matriz lookAt. Aplicando así el angulo de camara!
}

void Camara::MouseScroll(double xScroll, double yScroll) 
{ //Funcion que incrementa o decrementa el fov segun si sube o baja el scroll del raton, y segun una sensibilidad, creando zoom

	//std::cout << "Fov: " << this->FOV << std::endl;
	//std::cout << "YScroll: " << yScroll << std::endl;
	std::cout << "FOV + 100" << this->FOV + 100 << std::endl;
	int offset = (int)yScroll;
	switch (offset) {
	case 0:
		break;
	case 1:
		this->FOV += fovSens;
		break;
	case -1:
		this->FOV -= fovSens;
		break;
	default:
		break;
	}
	//Limitadores:
	if (FOV > maxFov) FOV = maxFov;
	else if (FOV < minFov) FOV = minFov;
}

glm::mat4 Camara::LookAt()
{
	glm::mat4 lookAtMatrix = glm::lookAt(this->cameraPos, this->cameraPos + this->cameraFront, this->cameraUp);
	return lookAtMatrix;
}

GLfloat Camara::GetFOV()
{
	return this->FOV;
}

void Camara::SetDT(GLfloat deltatime)
{
	this->Deltatime = deltatime;
}
