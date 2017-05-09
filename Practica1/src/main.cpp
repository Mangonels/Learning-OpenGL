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
#include "../material.h"

const GLint WIDTH = 800, HEIGHT = 600; //Dimensiones de la ventana que creamos mas adelante


//Invocación de la clase camara, para todas las funcionalidades de camara necesarias:
//Los 2 primeros valores son puntos con los que la clase forma los vectores que necesitamos para la camara:
Camara* camara = new Camara(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), 0.1f, 60.0f); //Posicion, Direccion, Sensibilidad de camara, fov de camara.

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

//Variables posición ratón (para camara)
GLfloat mPrevX = WIDTH/2, mPrevY = HEIGHT/2; //Componentes posicion previa del raton

//Variables de la camara:
//Angulos de camara iniciales (Se influencian con el raton)
float pitch = 0.0f; //(Arriba-Abajo  ejes: Y-X/Z)
float yaw = -90.0f; //(Izquierda-Derecha  ejes: Z/X)
//Variable que contiene el angulo de field of vision (la modificamos en el multi input):
float fov = 60.0f;
float fovSens = 2.0f;
float minFov = 20.0f;
float maxFov = 80.0f;
//Obtenemos posicion de la camara mediante metodo (Necesario para pasar a shaders mas abajo):
glm::vec3 camaraPos = camara->getPosition();

//Variables cubos:

//CUBO:
//Rotacion:
GLfloat cubeRotateX = 0.0f;
GLfloat cubeRotateY = 0.0f;
GLfloat cubeRotateZ = 0.0f;
//Posicion:
GLfloat cubePosOffsetX = 0.0f;
GLfloat cubePosOffsetY = 0.0f;
GLfloat cubePosOffsetZ = 0.0f;

//CUBO LUZ Y GENERACION DE LUZ:
//Posicion:
GLfloat lightPosOffsetX = 0.0f;
GLfloat lightPosOffsetY = 0.0f;
GLfloat lightPosOffsetZ = 0.0f;

glm::vec3 lightPos(0.0f, 2.0f, 0.0f); //La posicion de la luz que usaremos tambien como posicion de traslado del cubo emisor de luz
glm::vec3 lightDir(0.0f, -1.0f, 0.0f); //La direccion de la luz que usamos para todas las luces

GLfloat spotLightInnerRadius = glm::cos(glm::radians(13.0f));
GLfloat spotLightOuterRadius = glm::cos(glm::radians(20.0f));

//Variables calculo de atenuacion de luz puntual:
float constant = 1.0f; //Componente constante que asegura que el valor de atenuacion que genere la formula no sea menor a 1.
float linear = 0.09f; //Se multiplica por la distancia, reduciendo la intensidad en estilo linear.
float quadratic = 0.032f; //Se multiplica por el quadrante de la distancia y establece un descenso de intensidad para el origen de luz.

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

bool keys[1024]; //Guarda que teclas se estan pulsando, ya que el sistema de input de glfw no puede procesar más de 1 tecla a la vez.

void multiInputChecker() { //Función para revisar diversos inputs simultaneos:

	//Inputs de rotacion de cubo:

	if (keys[GLFW_KEY_KP_8]) { // Rotacion frontal
		cubeRotateX = glm::mod(cubeRotateX, 360.0f) + 0.3f;
		cout << "Rotacion cubo A (X): " << cubeRotateX << endl;
	}
	if (keys[GLFW_KEY_KP_2]) { // Rotacion trasera
		cubeRotateX = glm::mod(cubeRotateX, 360.0f) - 0.3f;
		cout << "Rotacion cubo A (X): " << cubeRotateX << endl;
	}
	if (keys[GLFW_KEY_KP_4]) { // Rotacion lateral izquierda
		cubeRotateZ = glm::mod(cubeRotateZ, 360.0f) - 0.3f;
		cout << "Rotacion cubo A (Z): " << cubeRotateZ << endl;
	}
	if (keys[GLFW_KEY_KP_6]) { // Rotacion lateral derecha
		cubeRotateZ = glm::mod(cubeRotateZ, 360.0f) + 0.3f;
		cout << "Rotacion cubo A (Z): " << cubeRotateZ << endl;
	}

	//Inputs de movimiento de cubo:

	if (keys[GLFW_KEY_LEFT]) { // Izquierda
		cubePosOffsetX -= 0.01f;
		cout << "Posicion cubo A (X): " << cubePosOffsetX << endl;
	}
	if (keys[GLFW_KEY_RIGHT]) { // Derecha
		cubePosOffsetX += 0.01f;
		cout << "Posicion cubo A (X): " << cubePosOffsetX << endl;
	}
	if (keys[GLFW_KEY_UP]) { // Arriba
		cubePosOffsetY += 0.01f;
		cout << "Posicion cubo A (Y): " << cubePosOffsetY << endl;
	}
	if (keys[GLFW_KEY_DOWN]) { // Abajo
		cubePosOffsetY -= 0.01f;
		cout << "Posicion cubo A (Y): " << cubePosOffsetY << endl;
	}

	//Inputs de movimiento de cubo luz:

	if (keys[GLFW_KEY_F]) { // Izquierda
		lightPosOffsetX -= 0.01f;
		//cout << "Posicion cubo B (X): " << lightPosOffsetX << endl;
	}
	if (keys[GLFW_KEY_H]) { // Derecha
		lightPosOffsetX += 0.01f;
		//cout << "Posicion cubo B (X): " << lightPosOffsetX << endl;
	}
	if (keys[GLFW_KEY_T]) { // Arriba
		lightPosOffsetY += 0.01f;
		//cout << "Posicion cubo B (Y): " << lightPosOffsetY << endl;
	}
	if (keys[GLFW_KEY_G]) { // Abajo
		lightPosOffsetY -= 0.01f;
		//cout << "Posicion cubo B (Y): " << lightPosOffsetY << endl;
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

	//Crear shaders:
	//Posibles directorios segun el tipo de iluminacion que queramos:
	const GLchar* simplePath = "./src/FragmentLight.fragmentshader"; //Luces ambiental, Difusa y Especular combinadas (Phong)
	const GLchar* directionalPath = "./src/FragmentDirectionalLight.fragmentshader"; //Luz direccional
	const GLchar* pointPath = "./src/FragmentPointLight.fragmentshader"; //Luz puntual
	const GLchar* spotPath = "./src/FragmentSpotLight.fragmentshader"; //Luz focal														  
	//Shader para objetos:
	Shader *lightShader = new Shader("./src/VertexLight.vertexshader", simplePath); //<- Cambiar el tipo de fragshader segun disponibles arriba: Se trata del shader de reflejos de luz sobre objetos
	//Shader especifico para el cubo emisor de luz:
	Shader emitterShader = Shader::Shader("./src/VertexEmitter.vertexshader", "./src/FragmentEmitter.fragmentshader"); //Color base, solo para el cubo de la luz

	//Generating cube object with first transformations:
	vec3 cubeScale = vec3(1.f, 1.f, 1.f);
	vec3 cubeRotate = vec3(1.f, 1.f, 1.f);
	vec3 cubeTranslate = vec3(0.f, 0.f, 0.f);
	Object cube(cubeScale, cubeRotate, cubeTranslate); //CUBE A

	//Generating light cube object with first transformations:
	vec3 lightCubeScale = vec3(0.1f, 0.1f, 0.1f);
	vec3 lightCubeRotate = vec3(1.f, 1.f, 1.f);
	Object lightCube(lightCubeScale, lightCubeRotate, lightPos); //CUBE B, la posicion es la posicion de la luz definida globalmente.

	//Generacion de material:
	Material material("./Materials/difuso.png", "./Materials/especular.png", 16);

	material.SetMaterial(lightShader); //Pasar el shader por referencia a la clase material que le asigna una textura difusa y especular.

	//BUCLE DE DIBUJO:
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents(); //Comprobar si se ha activado algun evento y ejecutar la funcion correspondiente (Las hemos definido antes fuera del main)
		multiInputChecker(); //Llamando comprobación de inputs simultaneos

		//Actualizando valores de tiempo:
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - prevFrame;
		prevFrame = currentFrame;
		camara->SetDT(deltaTime); //Pasamos el delta time a la propiedad de la clase

		//Mirar si se ha pulsado alguna tecla que mueva la posicion de la camara:
		camara->DoMovement();

		//Clearing color buffer array from OpenGL in order to ensure there is no colors being applied from last iteration:
		//Also clearing Z-Buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		//Aplicar lightShader (Shader especifico cubo):
		lightShader->USE(); //Shaders para el CUBO
		material.SetShininess(lightShader); //Pasar el shader por referencia a la clase material que le asigna el valor de brillo
		material.ActivateTextures();
			//Localizando las variables uniform de color del shader de luces, e inicializandolas mediante transferencia:
			//-------------------------
			GLint objectColorLoc = glGetUniformLocation(lightShader->Program, "objectColor");
			GLint lightColorLoc = glGetUniformLocation(lightShader->Program, "lightColor");
			GLint lightPosLoc = glGetUniformLocation(lightShader->Program, "lightPosition");
			GLint lightDirPos = glGetUniformLocation(lightShader->Program, "lightDirection");
			GLint lightSpotInnerCone = glGetUniformLocation(lightShader->Program, "innerConeRadius"); 
			GLint lightSpotOuterCone = glGetUniformLocation(lightShader->Program, "outerConeRadius");
			GLint viewPosLoc = glGetUniformLocation(lightShader->Program, "viewPos");
			glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f); //Enviar Color del objeto
			glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); //Enviar Color de la luz
			glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z); //Enviar Posicion de la luz
			glUniform3f(lightDirPos, lightDir.x, lightDir.y, lightDir.z); //Enviar Direccion para cualquier luz
			glUniform1f(lightSpotInnerCone, spotLightInnerRadius); //Enviar angulo que determina el radio de apertura de la luz focal
			glUniform1f(lightSpotOuterCone, spotLightOuterRadius); //Enviar angulo que determina el radio de apertura de la luz focal
			glUniform3f(viewPosLoc, camaraPos.x, camaraPos.y, camaraPos.z); //Enviar Posicion de la camara

			//Variables de calculo de atenuacion en point light:
			glUniform1f(glGetUniformLocation(lightShader->Program, "constant"), constant);
			glUniform1f(glGetUniformLocation(lightShader->Program, "linear"), linear);
			glUniform1f(glGetUniformLocation(lightShader->Program, "quadratic"), quadratic);
			//-------------------------

			//Ahora hay que adaptar las matrices para este shader:
			//Ajustes sobre las posiciones del cubo para generar adecuadamente la matriz modelo:
			 cube.Rotate(glm::vec3(cubeRotateX, cubeRotateY, cubeRotateZ));
			 cube.Translate(glm::vec3(cubeTranslate.x + cubePosOffsetX, cubeTranslate.y + cubePosOffsetY, cubeTranslate.z + cubePosOffsetZ)); //Cambiar posicion del cubo

			//Localizar donde van las matrices:
			GLint viewLoc = glGetUniformLocation(lightShader->Program, "view");
			GLint projLoc = glGetUniformLocation(lightShader->Program, "projection");
			GLint modelLoc = glGetUniformLocation(lightShader->Program, "model");

			//Matriz Vista:
			glm::mat4 view = camara->LookAt(); //Trasladamos la escena en la dirección contraria hacia donde queremos mover la camara, causando el efecto de que la camara se ha movido:
			//Matriz Proyeccion:
			glm::mat4 projection = glm::perspective(glm::radians(camara->GetFOV()), (float)(screenWidth / screenHeight), 0.1f, 100.0f); //angulo de fov, tamaño pantalla (tiene que ser un float), plano near, plano far.
			//Matriz Modelo:
			glm::mat4 model = cube.generateModelMatrix(); //La matriz modelo la define el generador de matriz del cubo.

			//Enviar matrices a sus localizaciones:
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		
		//DIBUJAR CUBO:
		cube.drawCube();

		//Aplicar emitterShader (Shader especifico cubo emisor de luz)
		emitterShader.USE();
			//Ajustar uniforms en las localizaciones pertinentes:
			//Localizar donde van las matrices:
			modelLoc = glGetUniformLocation(emitterShader.Program, "model");
			viewLoc = glGetUniformLocation(emitterShader.Program, "view");
			projLoc = glGetUniformLocation(emitterShader.Program, "projection");

			//Preparaciones en el cubo para la matriz modelo:
			 lightCube.Translate(glm::vec3(lightPos.x /*+ lightPosOffsetX*/, lightPos.y /*+ lightPosOffsetY*/, lightPos.z /*+ lightPosOffsetZ*/)); //Descomentando los offsets podemos cambiar posicion del cubo luz																													  //Light cube model matrix adjustments:
			 
			//Cambiamos la matriz modelo, (las demas matrices pueden quedarse igual)
			model = glm::mat4(lightCube.generateModelMatrix()); //Obtener una nueva matriz

			//Enviar matrices a sus localizaciones:
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		//DIBUJAR CUBO DE LUZ:
		lightCube.drawCube();

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

	//------------------------------------------------
	//INPUTS SIMULTANEOS - Estados de teclas pulsadas:
	//------------------------------------------------

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}
