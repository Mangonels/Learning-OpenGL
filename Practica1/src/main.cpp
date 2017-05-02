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

//Nuestra clase shader:
#include "../Shader.h"

//Nuestra clase camara:
#include "../Camara.h"

//Nuestra clase modelo:
#include "../Model.h"
//Modelos obj disponibles:

int modelShown = 0; //Que modelo deberia mostrarse (0 a 2)

const GLint WIDTH = 800, HEIGHT = 600; //Dimensiones de la ventana que creamos mas adelante


//Invocación de la clase camara, para todas las funcionalidades de camara necesarias:
//Los 2 primeros valores son puntos con los que la clase forma los vectores que necesitamos para la camara:
Camara* camara = new Camara(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), 0.1f, 60.0f); //Posicion, Direccion, Sensibilidad de camara, fov de camara.

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

//Guardador de teclas pulsadas:
bool keys[1024];

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

	//Generando shaders:
	Shader myShader = Shader::Shader(vertexPath, fragmentPath);

	const GLchar *deer = "./3DModels/Deer/deer_obj/deer-obj.obj";
	const GLchar *wuson = "./3DModels/spider/WusonOBJ.obj";
	const GLchar *sofa = "./3DModels/Sofa/sofa_obj/sofa.obj";
	//Cargando modelos:
	Model myModel0 = Model::Model(deer);
	Model myModel1 = Model::Model(wuson);
	Model myModel2 = Model::Model(sofa);

	//BUCLE DE DIBUJO:
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		glClearColor(1.0f,1.0f,1.0f,1.0f);

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

		//--------------------------------------------
		//ACTUALIZANDO ALGUNOS VALORES PARA EL SHADER:
		//--------------------------------------------

		myShader.USE();

		//--------------------
		//MATRICES DE ESPACIO:
		//--------------------
		//Convertir las coordenadas de nuestros vertices pasando por local > world > vew > clip, utilizando las matrizes modelo, vista y proyeccion en cada paso intermedio:

		//-> Matriz VISTA <-, pasa coordenadas de: MUNDO -> VISTA:
		//Trasladamos la escena en la dirección contraria hacia donde queremos mover la camara, causando el efecto de que la camara se ha movido:
		//GLfloat radius = 10.0f; //Para rotación de camara automatica.
		//GLfloat camX = sin(glfwGetTime()) * radius; //Rotación de camara automatica.
		//GLfloat camZ = cos(glfwGetTime()) * radius; //Rotación de camara automatica.
		glm::mat4 view;
		//view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)); //Activar esto hace rotar la camara automaticamente.
		view = camara->LookAt(); //La matriz view sera el output del metodo lookat de la clase camara.
		GLint viewLoc = glGetUniformLocation(myShader.Program, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); //<Pasar matriz al shader

		//-> Matrz PROYECCION <-, define que tipo de proyeccion queremos:
		//En este caso una perspectiva, que nos permitirá ver objetos en 3D con profundidad.
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camara->GetFOV()), (float)(screenWidth / screenHeight), 0.1f, 100.0f); //angulo de fov, tamaño pantalla (tiene que ser un float), plano near, plano far.
		GLint projLoc = glGetUniformLocation(myShader.Program, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//-> Matriz MODELO <-, pasa coordenadas de: LOCAL -> MUNDO:
		glm::mat4 model;
		//Lo siguiente esta comentado porque lo repartimos entre los casos a la hora de dibujar.
		//GLint modelLoc = glGetUniformLocation(myShader.Program, "model"); //Localizar matriz modelo en el shader
		//model = glm::translate(model, glm::vec3(0.0f, -1.00f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

		//Clearing color buffer array from OpenGL in order to ensure there is no colors being applied from last iteration:
		//Also clearing Z-Buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//---------------
		//DIBUJAR MODELO:
		//---------------
		//Adaptacion de la matriz modelo:
		switch (modelShown) 
		{
		case 0: //Operaciones sobre el ciervo:
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); //Trasladar
			model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f)); //Escalar
			break;
		case 1: //Operaciones sobre la vaca:
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
			break;
		case 2: //Operaciones sobre el sofa:
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));
			break;
		default:
			break;
		}

		//Pasar la matriz modelo adaptada segun el modelo previamente:
		glUniformMatrix4fv(glGetUniformLocation(myShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		//Dibujar modelos:
		switch (modelShown)
		{
		case 0:
			myModel0.Draw(myShader, GL_TRIANGLES); //Modelo de un ciervo
			break;
		case 1:
			myModel1.Draw(myShader, GL_TRIANGLES); //Modelo de una especie de vaca/toro raro
			break;
		case 2:
			myModel2.Draw(myShader, GL_TRIANGLES); //Modelo de un sofa
			break;
		default:
			break;
		}

		// Intercambia el buffer con el de la ventana para mostrar el resultado en ventana:
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

	//Cambio de modelo:
	if (key == GLFW_KEY_1 && action == GLFW_PRESS || key == GLFW_KEY_KP_1 && action == GLFW_PRESS)
		modelShown = 0;
	if (key == GLFW_KEY_2 && action == GLFW_PRESS || key == GLFW_KEY_KP_1 && action == GLFW_PRESS)
		modelShown = 1;
	if (key == GLFW_KEY_3 && action == GLFW_PRESS || key == GLFW_KEY_KP_1 && action == GLFW_PRESS)
		modelShown = 2;

	//------------------------------------------------
	//INPUTS SIMULTANEOS - Estados de teclas pulsadas:
	//------------------------------------------------

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}
