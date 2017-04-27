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

	//-------------------------------------------------------------
	//CREAR Y CARGAR UNA TEXTURA PARA APLICARLA A NUESTRO POLIGONO:
	//-------------------------------------------------------------
	//Generar la textura y pasarla a OpenGL:
	//1. Reservar memoria en openGL:
	GLuint dirtTexture;
	//2. Apuntar el puntero de textura a la memoria reservada:
	glGenTextures(1, &dirtTexture);
	glBindTexture(GL_TEXTURE_2D, dirtTexture);
	//3.A, Opciones de Wrapping (Si no se hace nada queda negro) Serían para ESTA imagen:
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT); //Con esto haremos que la textura se repita si sobra espacio vacío.
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
	//3.B, Opciones de Filtrado (ES OBLIGATORIO especificar esto, para ancho y alto) Son para ESTA imagen:
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	//4.Cargar la imagen:
	int dirtTexWidth, dirtTexHeight; //Tiene que ser para ESTA imagen, no puede ser global.
		//Cargar una imagen con SOIL:
	unsigned char* dirtImage = SOIL_load_image("./src/dirt.png", &dirtTexWidth, &dirtTexHeight, 0, SOIL_LOAD_RGB); //Directorio, tamañoX, tamañoY, (ni idea de que es esto, pero en 0 va bien), modo de color.
		//Especificar una imagen bidimensional para OpenGL:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dirtTexWidth, dirtTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, dirtImage); //target, level of detail, color mode, tamañoX, tamañoY, borde, formato pixel data, tipo del pixel data, puntero a la imagen cargada.
		//Pasarle la imagen especificada a OpenGL, OGL lo guarda en un array, cada elemento es un GL_TEXTUREN, donde la N final es el nombre de la textura (para futuras referencias):
	glBindTexture(GL_TEXTURE_2D, 0); //Pasa las texturas creadas a OpenGL, y las llama 0.
	//5.Liberar el puntero de textura.
	SOIL_free_image_data(dirtImage);

	//Una segunda textura:
	GLuint ironTexture;
	glGenTextures(1, &ironTexture);
	glBindTexture(GL_TEXTURE_2D, ironTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	int ironTexWidth, ironTexHeight;
	unsigned char* ironImage = SOIL_load_image("./src/iron.png", &ironTexWidth, &ironTexHeight, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ironTexWidth, ironTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, ironImage);
	glBindTexture(GL_TEXTURE_2D, 1);
	SOIL_free_image_data(ironImage);

	//---------------------
	// Propiedades cubo 3D:
	//---------------------
	GLfloat VertexBufferCube[] = {
		//Vertices del Cubo	  //Vertices de textura
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		0.5f , -0.5f, -0.5f,  1.0f, 0.0f,
		0.5f ,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f ,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f , -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f ,  0.5f,  0.5f,  1.0f, 1.0f,
		0.5f ,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		0.5f ,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f ,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f , -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f , -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f , -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f ,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f , -0.5f, -0.5f,  1.0f, 1.0f,
		0.5f , -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f , -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		0.5f ,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f ,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f ,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	glm::vec3 CubesPositionBuffer[] = { //Son todas las posiciones del mundo donde queremos colocar un cubo.
		glm::vec3(0.0f ,  0.0f,  0.0f),
		glm::vec3(2.0f ,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f , -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f , -2.0f, -2.5f),
		glm::vec3(1.5f ,  2.0f, -2.5f),
		glm::vec3(1.5f ,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	//Activar Z-Buffer (Buffer de profundidad de fragmentos)
	glEnable(GL_DEPTH_TEST); //Esto comprobará que las caras de los poligonos no se fuckeen una encima de la otra, asegurando mostrar con prioridad las que están más cerca de la camara.

	//---------------
	//VBO y VAO (3D):
	//---------------
	// Crear los VBO, VAO y EBO
	GLuint VBO;
	GLuint VAO;

	//reservar memoria para el VAO, VBO y EBO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	//Establecer el objeto:
	glBindVertexArray(VAO);

	//Enlazar el buffer con openGL:
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //buffer name, pointer to buffer (This binds the previosuly generated buffer to OpenGL)
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexBufferCube), VertexBufferCube, GL_STATIC_DRAW); //buffer name, size for the buffer, pointer for data which will be copied into buffer, expected usage.

	//Establecer las propiedades de los vertices (Tenemos que indicar que parte del array corresponde a un componente del vertice. X componentes = 1 propiedad)
	//Coordenadas:
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid*)0); //index, cuantos componentes, tipo, si se va a normalizar, size conjunto de componentes, offset para empezar a mirar los componentes de la propiedad.
	glEnableVertexAttribArray(0); //Habilitar definición de coordenadas.
	//Coordenadas de textura: 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat))); //(Definen que parte de la textura utilizamos para dibujarla encima del poligono)
	glEnableVertexAttribArray(1);

	//liberar el buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//liberar el buffer de vertices
	glBindVertexArray(0);

	Shader myShader = Shader::Shader(vertexPath, fragmentPath);

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

		//--------------------------------------------
		//ACTUALIZANDO ALGUNOS VALORES PARA EL SHADER:
		//--------------------------------------------

		//Pasando el modo de dibujo a los shader para que sepan si aplicar o no textura:
		GLint wireLoc = glGetUniformLocation(myShader.Program, "wireframe");
		glUniform1f(wireLoc, wireframe);

		//Pasando el valor necesario para la función mixer del Vertex.vertexshader
		GLint mixValue = glGetUniformLocation(myShader.Program, "merge");
		//Especifica el valor de una variable uniform para el objeto programa actual:
		glUniform1f(mixValue, merge); //Localización, float

		//Pasandole la textura que se ha generado fuera del while, a OpenGL, pero tambien al shader de fragmentos:
		glActiveTexture(GL_TEXTURE0); //Activamos la textura llamada 0: GL_TEXTURE0
		glBindTexture(GL_TEXTURE_2D, dirtTexture); //Pasamos la textura "dirtTexture", que es una textura 2D, a OpenGL
		//Así es como accedemos a la variable sampler2D del fragmentshader, y le asignamos la textura en posición 0:
		glUniform1i(glGetUniformLocation(myShader.Program, "dirtTexture"), 0); 

		//Pasando la segunda textura:
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ironTexture);
		glUniform1i(glGetUniformLocation(myShader.Program, "ironTexture"), 1);

		myShader.USE();

		//----------
		//VISTA 3D:
		//----------
		//Convertir las coordenadas de nuestros vertices pasando por local > world > vew > clip, utilizando las matrizes modelo, vista y proyeccion en cada paso intermedio:

		//-> Matriz MODELO, pasa coordenadas de: LOCAL -> MUNDO:
		//Aqui solo la estamos pasando al shader.
		GLint modelLoc = glGetUniformLocation(myShader.Program, "model"); //Localizar matriz modelo en el shader

		//-> Matriz VISTA, pasa coordenadas de: MUNDO -> VISTA:
		//Trasladamos la escena en la dirección contraria hacia donde queremos mover la camara, causando el efecto de que la camara se ha movido:
		//GLfloat radius = 10.0f; //Para rotación de camara automatica.
		//GLfloat camX = sin(glfwGetTime()) * radius; //Rotación de camara automatica.
		//GLfloat camZ = cos(glfwGetTime()) * radius; //Rotación de camara automatica.
		glm::mat4 view;
		//view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)); //Activar esto hace rotar la camara automaticamente.
		view = camara->LookAt();
		GLint viewLoc = glGetUniformLocation(myShader.Program, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); //<Pasar matriz al shader

		//-> Matrz PROYECCION, define que tipo de proyeccion queremos:
		//En este caso una perspectiva, que nos permitirá ver objetos en 3D con profundidad.
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(camara->GetFOV()), (float)(screenWidth / screenHeight), 0.1f, 100.0f); //angulo de fov, tamaño pantalla (tiene que ser un float), plano near, plano far.
		GLint projLoc = glGetUniformLocation(myShader.Program, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//Clearing color buffer array from OpenGL in order to ensure there is no colors being applied from last iteration:
		//Also clearing Z-Buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Pintar el VAO
		glBindVertexArray(VAO);

		//Pintar Opaco:
		if (!wireframe) {
			glClearColor(0.5, 0.5, 0.5, 1.0);
			for (GLuint i = 0; i < 10; i++) //Este for lo usamos para ir iterando por las posiciones guardadas en CubesPositionBuffer
			{
				glm::mat4 model; //La matriz modelo
				if (i >= 1) { //Todos los cubos excepto el primero:
					model = glm::translate(model, CubesPositionBuffer[i]); //Asignamos valores a la matriz modelo, esta desplaza el eje a la posición que haya en CubesPositionBuffer[i]
					model = glm::rotate(model, (GLfloat)glfwGetTime() * 1.0f, glm::vec3(0.5f, 1.0f, 0.0f)); //Que roten solos estos cubos con el tiempo glfwGetTime()
					glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); //Localizacion de la matriz modelo en el shader de vertices

					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glDrawArrays(GL_TRIANGLES, 0, 36); //Dibujar un cubo (En la posición que estamos)
				}
				else { //El primer cubo entraría aqui, lo controlamos nosostros, rotandolo segun la variable firstCubeAngle:
					model = glm::translate(model, CubesPositionBuffer[i]);
					//La rotacion la controlamos nosotros:
					model = glm::rotate(model, glm::radians(cubeAngleX), glm::vec3(1.0f, 0.0f, 0.0f)); //Rotación en eje X, Los ultimos parametros significan cuanto afecta en x, y, z el angulo especificado. De 0 a 1.
					model = glm::rotate(model, glm::radians(cubeAngleY), glm::vec3(0.0f, 1.0f, 0.0f)); //Rotación en eje Y
					glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
			}
		} 
		//Pintar con Lineas:
		else {
			glClearColor(0.5, 0.5, 0.5, 1.0);
			for (GLuint i = 0; i < 10; i++)
			{
				glm::mat4 model;

				if (i >= 1) {
					model = glm::translate(model, CubesPositionBuffer[i]);
					model = glm::rotate(model, (GLfloat)glfwGetTime() * 1.0f, glm::vec3(0.5f, 1.0f, 0.0f));
					glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
				else {
					model = glm::translate(model, CubesPositionBuffer[i]);
					model = glm::rotate(model, glm::radians(cubeAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
					model = glm::rotate(model, glm::radians(cubeAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
					glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
			}
		}

		glBindVertexArray(0);

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}
	// liberar la memoria de los VAO, EBO y VBO
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	//glDeleteBuffers(1, &EBO); //Si usamos 2D

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
