//GLEW
#define GLEW_STATIC
#include <GL\glew.h>
//GLFW
#include <GLFW\glfw3.h>
#include <iostream>

using namespace std;

//Librería de cargado de imagenes:
#include <SOIL.h>

//Librerías de matematicas de OpenGL:
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

//Clases:
#include "../Shader.h"
#include "../Camara.h"
#include "../Object.h"

const GLint WIDTH = 800, HEIGHT = 600; //Dimensiones de la ventana que creamos mas adelante


//Invocación de la clase camara, para todas las funcionalidades de camara necesarias:
//Los 2 primeros valores son puntos con los que la clase forma los vectores que necesitamos para la camara:
Camara* camara = new Camara(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), 0.1f, 60.0f); //Posicion, Direccion, Sensibilidad de camara, fov de camara.

bool wireframe = false; //Si la geometria deberia mostrarse en modo wireframe

//Shaders para clipping 3D:
const GLchar* vertexPath = "./src/Vertex.vertexshader";
const GLchar* fragmentPath = "./src/Fragment.fragmentshader";

//Definicion inicial de funciones manager de inputs:
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode); //Funcion rasteadora de inputs por teclado en la ventana 
void mouse_move_input(GLFWwindow* window, double xpos, double ypos) 
{
	camara->MouseMove(xpos, ypos);
}
void mouse_scroll_input(GLFWwindow* window, double xOffset, double yOffset)
{
	camara->MouseScroll(xOffset, yOffset);
}

//Variables calculo de tiempo:
GLfloat deltaTime = 0.0f;
GLfloat prevFrame = 0.0f;

//Variables para el offset oscilatorio:
bool incrementando = true;
float elOffset = 0.0f;
float offsetMaximo = 0.5f;

//Variable para la función mix:
float merge = 0.0;

//Variables rotación cubo:
float cubeAngleX = 1.0f;
float cubeAngleY = 1.0f;

//Variables posición ratón (para camara)
GLfloat mPrevX = WIDTH/2, mPrevY = HEIGHT/2; //Componentes posicion previa del raton

//Variables camara:

	//Angulos de camara iniciales (Se influencian con el raton)
float pitch = 0.0f; //(Arriba-Abajo  ejes: Y-X/Z)
float yaw = -90.0f; //(Izquierda-Derecha  ejes: Z/X)
    //Variable que contiene el angulo de field of vision (la modificamos en el multi input):
float fov = 60.0f;
float fovSens = 2.0f;
float minFov = 20.0f;
float maxFov = 80.0f;

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

bool keys[1024]; //Guarda que teclas se estan pulsando, ya que el sistema de input de glfw no puede procesar más de 1 tecla a la vez.

void multiInputChecker() { //Función para revisar diversos inputs simultaneos:
	
	//Incrementar o decrementar combinación de texturas:
	if (keys[GLFW_KEY_1] && merge < 0.950f || keys[GLFW_KEY_KP_1] && merge < 0.950f) { //tecla 1
		merge += 0.005f;
		cout << "Valor de mix: " << merge << endl;
	}
	if (keys[GLFW_KEY_2] && merge > 0.005f || keys[GLFW_KEY_KP_2] && merge > 0.005f) { //tecla 2
		merge -= 0.005f;
		cout << "Valor de mix: " << merge << endl;
	}

	//Inputs de movimiento de cubo:

	if (keys[GLFW_KEY_LEFT]) { // Izquierda
		cubeAngleY -= 1.0f;
		cout << "Grados de rotacion cubo 1 (X): " << cubeAngleY << endl;
	}
	if (keys[GLFW_KEY_RIGHT]) { // Derecha
		cubeAngleY += 1.0f;
		cout << "Grados de rotacion cubo 1 (X): " << cubeAngleY << endl;
	}
	if (keys[GLFW_KEY_UP]) { // Arriba
		cubeAngleX -= 1.0f;
		cout << "Grados de rotacion cubo 1 (Y): " << cubeAngleX << endl;
	}
	if (keys[GLFW_KEY_DOWN]) { // Abajo
		cubeAngleX += 1.0f;
		cout << "Grados de rotacion cubo 1 (Y): " << cubeAngleX << endl;
	}
}

int main() {

	//initGLFW
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);

	//comprobar que GLFW estaactivo
	if (!glfwInit())
		exit(EXIT_FAILURE);

	//set GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	//create a window
	window = glfwCreateWindow(WIDTH, HEIGHT, "Practica 1A", nullptr, nullptr);
	if (!window) {
		cout << "Error al crear la ventana" << endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);

	//set GLEW and inicializate
	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit()) {
		cout << "Error al iniciar glew" << endl;
		glfwTerminate();
		return NULL;
	}

	//Definicion de funciones de rastreo de input (Determinar que funcion va a esperar inputs de que tipo): Parametros: En que ventana se usará esta función, que función.
	glfwSetKeyCallback(window, key_callback); //Definicion de función de input por teclado
	glfwSetCursorPosCallback(window, mouse_move_input); //Definicion de función de input por posicion de raton
	glfwSetScrollCallback(window, mouse_scroll_input); //Definicion de funcion de input por scroll de raton

	//set windows and viewport
	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Encapsula el ratón para que no pueda salir de las coordenadas internas a la ventana, y lo esconde.

	//Activar Z-Buffer (Buffer de profundidad de fragmentos)
	glEnable(GL_DEPTH_TEST); //Esto comprobará que las caras de los poligonos no se fuckeen una encima de la otra, asegurando mostrar con prioridad las que están más cerca de la camara.

	//Shaders loading object:
	Shader myShader = Shader::Shader(vertexPath, fragmentPath);

	//Generating cube object with transformations:
	vec3 cubeScale = vec3(0.5, 0.5, 0.5);
	vec3 cubeRotate = vec3(1, 1, 1);
	vec3 cubeTranslate = vec3(-1.f, 0, 0);
	Object cube(cubeScale, cubeRotate, cubeTranslate);

	//Generating light cube object with transformations:
	vec3 lightCubeScale = vec3(1, 1, 1);
	vec3 lightCubeRotate = vec3(2, 2, 2);
	vec3 lightCubeTranslate = vec3(3, 0, 0);
	Object lightCube(lightCubeScale, lightCubeRotate, lightCubeTranslate);

	//BUCLE DE DIBUJO:
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		multiInputChecker(); //Llamando comprobación de inputs simultaneos

		//Actualizando valores de tiempo:
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - prevFrame;
		prevFrame = currentFrame;
		camara->SetDT(deltaTime); //Pasamos el delta time a la propiedad de la clase

		//Mirar si se ha pulsado alguna tecla que mueva la posicion de la camara:
		camara->DoMovement();

		//Mirar si el raton se ha movido para consecuentemente mover la camara:
		void mouse_move_input(GLFWwindow* window, double xpos, double ypos);

		//Mirar si se ha usado el scroll para consecuentemente cambiar el fov de camara:
		void mouse_scroll_input(GLFWwindow* window, double xOffset, double yOffset);

		myShader.USE();

		//----------
		//VISTA 3D:
		//----------
		//Convertir las coordenadas de nuestros vertices pasando por local > world > vew > clip, utilizando las matrizes modelo, vista y proyeccion en cada paso intermedio:

		//-> Matriz VISTA, pasa coordenadas de: MUNDO -> VISTA:
		//Trasladamos la escena en la dirección contraria hacia donde queremos mover la camara, causando el efecto de que la camara se ha movido:
		glm::mat4 view;
		view = camara->LookAt();
		GLint viewLoc = glGetUniformLocation(myShader.Program, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); //<Pasar matriz al shader

		//-> Matrz PROYECCION, define que tipo de proyeccion queremos:
		//En este caso una perspectiva, que nos permitirá ver objetos en 3D con profundidad.
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camara->GetFOV()), (float)(screenWidth / screenHeight), 0.1f, 100.0f); //angulo de fov, tamaño pantalla (tiene que ser un float), plano near, plano far.
		GLint projLoc = glGetUniformLocation(myShader.Program, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//-> Matriz MODELO, pasa coordenadas de: LOCAL -> MUNDO:
		//Aqui solo la estamos pasando al shader.
		glm::mat4 model;
		GLint modelLoc = glGetUniformLocation(myShader.Program, "model"); //Localizar matriz modelo en el shader
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection)); //Pasar la matriz

		//Clearing color buffer array from OpenGL in order to ensure there is no colors being applied from last iteration:
		//Also clearing Z-Buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//cube.drawCube();
		//lightCube.drawCube();

		glBindVertexArray(0);

		//Cambia framebuffer por buffer de ventana:
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW. No need for even returning 0.
	exit(EXIT_SUCCESS);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {

	//-------------------------------
	//INPUTS INDIVIDUALES:
	//-------------------------------

	//Cierre de ventana:
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	//Cambio de modo de dibujo:
	if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		wireframe = !wireframe;
		cout << wireframe;
	}

	//------------------------------------------------
	//INPUTS SIMULTANEOS - Estados de teclas pulsadas:
	//------------------------------------------------

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}
