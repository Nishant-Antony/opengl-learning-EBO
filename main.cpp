#include <iostream>
#include <glad/glad.h> // FUNCTION LOADER. ALLOWS YOU TO USE FUNCTIONS PROVIDED BY GPU DRIVER.
#include <GLFW/glfw3.h> // TO CREATE WINDOWS AND HANDLE EVENTS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>

void framebuffer_size_callback(GLFWwindow* window, int width, int height); // RESIZE WINDOW 
void processInput(GLFWwindow* window);

std::string loadShaderSrc(const char* filename);

int main() {

	int success;
	char infoLog[512];

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // 3.x
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // 3.3

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // MODERN OPENGL ONLY.

# ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE)
#endif

		GLFWwindow* window = glfwCreateWindow(800, 600, "playing around with EBO!", NULL, NULL); // POINTER TO WINDOW

	if (window == NULL) { // WINDOW NOT CREATED
		std::cout << "COULD NOT CREATE WINDOW" << std::endl;
		glfwTerminate(); return -1;
	}

	glfwMakeContextCurrent(window); // MAKES window THE OFFICIAL WORKSPACE TO WORK IN.

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "FAILED TO INITIALIZE GLAD" << std::endl;
		glfwTerminate(); return -1;
	}


	glViewport(0, 0, 800, 600); // 0,0 -> START DRAWING FROM THE BOTTOM LEFT CORNER
	// 800, 600 -> 800 WIDTH, 600 HEIGHT

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // RESIZEABILITY

	/*
		SHADERS
	*/

	// COMPILE VERTEX SHADER

	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER); // ASKS GPU TO CREATE A VERTEX SHADER AND RETURNS A INT [ NUMBER ID ]
	std::string vertShaderSrc = loadShaderSrc("assets/vertex_core.glsl"); // CALL THE FN. ON THE vertex_core.glsl
	const GLchar* vertShader = vertShaderSrc.c_str(); // OPENGL UNDERSTANDS ONLY C
	// TAKE THE vertShaderSrc STRING AND CONVERT
	// INTO c style STRING AND vertShader is pointer 
	// TO THAT STRING.

	glShaderSource(vertexShader, 1, &vertShader, NULL);
	// vertexShader -> THE UNQIUE ID CREATED
	// 1 -> NUMBER OF STRINGS
	// &vertShader -> the actual code
	// NULL -> ignore length, figure it out automatically

	glCompileShader(vertexShader);
	// text -> GPU instructions

	// CATCH ERROR
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success) { // if not success
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "Error with vertex shader comp.: " << std::endl << infoLog << std::endl;
	}

	// COMPILE FRAGMENT SHADER -> ONE

	unsigned int fragmentShaders[2];

	fragmentShaders[0] = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragShaderSrc = loadShaderSrc("assets/fragment_core.glsl");
	const GLchar* fragShader = fragShaderSrc.c_str();
	glShaderSource(fragmentShaders[0], 1, &fragShader, NULL);
	glCompileShader(fragmentShaders[0]);

	// CATCH ERROR -> FRAG ONE

	glGetShaderiv(fragmentShaders[0], GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(fragmentShaders[0], 512, NULL, infoLog);
		std::cout << "Error with frag shader comp.: " << std::endl << infoLog << std::endl;
	}

	// FRAGMENT SHADER -> TWO

	fragmentShaders[1] = glCreateShader(GL_FRAGMENT_SHADER);
	fragShaderSrc = loadShaderSrc("assets/fragment_shader2.glsl");
	fragShader = fragShaderSrc.c_str();
	glShaderSource(fragmentShaders[1], 1, &fragShader, NULL);
	glCompileShader(fragmentShaders[1]);

	// CATCH ERROR -> FRAG TWO

	glGetShaderiv(fragmentShaders[1], GL_COMPILE_STATUS, &success);

	if (!success) {
		glGetShaderInfoLog(fragmentShaders[1], 512, NULL, infoLog);
		std::cout << "Error with frag shader comp.: " << std::endl << infoLog << std::endl;
	}

	// SHADER PROGRAM -> ONE
	unsigned int shaderPrograms[2];

	shaderPrograms[0] = glCreateProgram(); // GPU, CREATE A PROGRAM

	glAttachShader(shaderPrograms[0], vertexShader); // POSIITION LOGIC ATTATCHED
	glAttachShader(shaderPrograms[0], fragmentShaders[0]); // FRAGMENT LOGIC ATTATCHED

	glLinkProgram(shaderPrograms[0]); // COMBINE BOTH INTO A WORKING PROGRAM

	glGetProgramiv(shaderPrograms[0], GL_LINK_STATUS, &success);

	if (!success) {
		glGetProgramInfoLog(shaderPrograms[0], 512, NULL, infoLog);
		std::cout << "Linking error : " << std::endl << infoLog << std::endl;
	}

	// shader program -> TWO
	shaderPrograms[1] = glCreateProgram(); // GPU, CREATE A PROGRAM

	glAttachShader(shaderPrograms[1], vertexShader); // POSIITION LOGIC ATTATCHED
	glAttachShader(shaderPrograms[1], fragmentShaders[1]); // FRAGMENT LOGIC ATTATCHED

	glLinkProgram(shaderPrograms[1]); // COMBINE BOTH INTO A WORKING PROGRAM

	glGetProgramiv(shaderPrograms[1], GL_LINK_STATUS, &success);

	if (!success) {
		glGetProgramInfoLog(shaderPrograms[1], 512, NULL, infoLog);
		std::cout << "Linking error : " << std::endl << infoLog << std::endl;
	}

	// CATCH ERRORS -> SHADER ONE

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShaders[0]);
	glDeleteShader(fragmentShaders[1]);
	
	// 6 indices and 4 vertices are used in this program

	float vertices[] = { // vertices
		0.7f,  0.5f, 0.0f,
	   -0.3f,  0.5f, 0.0f,
	   -0.7f, -0.5f, 0.0f,
		0.3f, -0.5f, 0.0f
	};

	// EBO
	unsigned int indices[] = { // indices refering to the rectangle constuction

		0, 1, 2, // first triangle
		2, 3, 0, // second triangle

	};

	// VAO, VBO

	unsigned int VAO, VBO, EBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glGenBuffers(1, &EBO); // setting up EBO

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// SET ATTRIBUTE POINTER

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// SET UP EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	while (!glfwWindowShouldClose(window)) { // glfwWindowShouldClose -> should the window be closed? 
		// 1 -> YES || 0 -> NO
		processInput(window);

		glClearColor(0.79, 0.79, 0.79, 1.0);// SETS THIS COLOR IN BUFFER
		glClear(GL_COLOR_BUFFER_BIT);

		// DRAWING SHAPES
		glBindVertexArray(VAO);

		glUseProgram(shaderPrograms[0]);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)0); // render first triangle via first shader

		glUseProgram(shaderPrograms[1]);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * sizeof(unsigned int))); // render second triangle via second shader

		glfwSwapBuffers(window);
		glfwPollEvents(); // CHECK FOR EVENTS

	}

	glDeleteVertexArrays(1, &VBO);
	glDeleteBuffers(1, &VAO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();

	return 0;

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

}

std::string loadShaderSrc(const char* filename) {

	std::ifstream file; // CREATE FILE READER
	std::stringstream buf; // TEMPORARY STORAGE

	std::string ret = ""; // HOLDS THE FINAL TEXT

	file.open(filename);

	if (file.is_open()) {
		buf << file.rdbuf(); // COPY EVERYTING FROM FILE TO BUFFER
		ret = buf.str(); // CONVERT BUF TO STRING AND STORE IN ret
	}

	else {
		std::cout << "COULD NOT OPEN " << filename << std::endl;
	}

	return ret; // RETURN THE FINAL TEXT

}