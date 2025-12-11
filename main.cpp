#include <iostream>
#include <thread>
#include <chrono>
#include "squareFunction.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLuint compileShader(GLenum type, const char* src) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	GLint success;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cerr << "Shader compile error " << std::endl;
		std::cerr << infoLog << std::endl;
	}
	return shader;
}

GLuint createProgram(const char* vertSrc, const char* fragSrc) {
	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertSrc);
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragSrc);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cerr << "Program link error " << std::endl;
		std::cerr << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}

const char* vertexShaderSrc = R"(
		#version 330 core
		layout (location = 0) in vec2 aPos;

		uniform vec2  uPosition;
		uniform float uPointSize;

		void main() {
			gl_Position = vec4(uPosition, 0.0, 1.0);
			gl_PointSize = uPointSize;
		}
	)";

const char* fragmentShaderSrcA = R"(
		#version 330 core
		out vec4 FragColor;

		void main() {
			FragColor = vec4(1.0, 0.2, 0.2, 1.0); // solid red
		}
	)";

const char* fragmentShaderSrcB = R"(
		#version 330 core
		out vec4 FragColor;

		void main() {
			FragColor = vec4(0.2, 0.8, 1.0, 1.0); // bright cyan
		}
	)";

const char* fragmentShaderSrcC = R"(
		#version 330 core
		out vec4 FragColor;

		void main() {
			FragColor = vec4(1.0, 0.2, 0.8, 0.4); // solid pink
		}
	)";

const char* fragmentShaderSrcD = R"(
		#version 330 core
		out vec4 FragColor;

		void main() {
			FragColor = vec4(0.2, 0.8, 0.0, 1.0); // bright cyan
		}
	)";

int main() {

	Square squareA;
	Square squareB;
	Square squareC;
	Square squareD;
	//char pressedKey;
	std::vector<Square> squaresVector;
	std::vector<GLuint> programVector;
	std::vector<GLint> positionLocVector;
	std::vector<GLint> pointSizeLocVector;

	int currentStep = 0;

	int bounceCount = 0;
	int hitWallCount = 0;
	float accelerationFriction = mu * Gravity;
	int index = 0;


	// start position at middle of screen. Will Change to left bottom to simulate the motion of the object
	squareA.pos.x = -0.8f;
	squareA.pos.y = 0.8f;

	squareA.vel.vx = 0.0f;
	squareA.vel.vy = -0.6f;

	squareA.pointSize = 15.0f;
	squareA.name = "red";

	squareB.pos.x = -0.8f;
	squareB.pos.y = 0.6f;

	squareB.vel.vx = 0.0f;
	squareB.vel.vy = -0.3f;

	squareB.pointSize = 30.0f;
	squareB.name = "blue";

	squareC.pos.x = -0.2f;
	squareC.pos.y = 1.0f;

	squareC.vel.vx = -0.10f;
	squareC.vel.vy = -0.3f;

	squareC.pointSize = 60.0f;
	squareC.name = "pink";

	squareD.pos.x = 0.5f;
	squareD.pos.y = -0.6f;

	squareD.vel.vx = -0.3f;
	squareD.vel.vy = 0.5f;

	squareD.pointSize = 50.0f;
	squareD.name = "green";

	squaresVector.push_back(squareA);
	squaresVector.push_back(squareB);
	squaresVector.push_back(squareC);
	squaresVector.push_back(squareD);


	/* Initialize the library */
	if (!glfwInit())
		return -1;

	GLFWwindow* window;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1280, 720, "Motion Simulator", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
		glfwTerminate();
		return -1;
	}

	glEnable(GL_PROGRAM_POINT_SIZE);


	glViewport(0, 0, 1280, 720);
	glfwSwapInterval(1);

	// background color
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

	GLuint programA = createProgram(vertexShaderSrc, fragmentShaderSrcA);
	GLuint programB = createProgram(vertexShaderSrc, fragmentShaderSrcB);
	GLuint programC = createProgram(vertexShaderSrc, fragmentShaderSrcC);
	GLuint programD = createProgram(vertexShaderSrc, fragmentShaderSrcD);
	programVector.push_back(programA);
	programVector.push_back(programB);
	programVector.push_back(programC);
	programVector.push_back(programD);

	for (auto program : programVector) {
		if (!program) {
			glfwTerminate();
			return -1;
		}
	}

	float point[] = { 0.0f, 0.0f };

	GLuint VAO;
	GLuint VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	glBindVertexArray(0);

	GLint uPositionLocA = glGetUniformLocation(programA, "uPosition");
	GLint uPointSizeLocA = glGetUniformLocation(programA, "uPointSize");

	GLint uPositionLocB = glGetUniformLocation(programB, "uPosition");
	GLint uPointSizeLocB = glGetUniformLocation(programB, "uPointSize");

	GLint uPositionLocC = glGetUniformLocation(programC, "uPosition");
	GLint uPointSizeLocC = glGetUniformLocation(programC, "uPointSize");

	GLint uPositionLocD = glGetUniformLocation(programD, "uPosition");
	GLint uPointSizeLocD = glGetUniformLocation(programD, "uPointSize");

	positionLocVector.push_back(uPositionLocA);
	positionLocVector.push_back(uPositionLocB);
	positionLocVector.push_back(uPositionLocC);
	positionLocVector.push_back(uPositionLocD);

	pointSizeLocVector.push_back(uPointSizeLocA);
	pointSizeLocVector.push_back(uPointSizeLocB);
	pointSizeLocVector.push_back(uPointSizeLocC);
	pointSizeLocVector.push_back(uPointSizeLocD);

	srand(static_cast<unsigned>(time(nullptr)));

	/*Square newSquare = createSquare();

	std::cout << "New squares values" << std::endl;

	std::cout << "Velocity: " << std::endl;
	std::cout << newSquare.vel.vx << "," << newSquare.vel.vy << std::endl;

	std::cout << "Position: " << std::endl;
	std::cout << newSquare.pos.x << "," << newSquare.pos.y << std::endl;

	std::cout << "Point Size: " << std::endl;
	std::cout << newSquare.pointSize << std::endl;*/
	while (!glfwWindowShouldClose(window)) {

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			Square newSquare = createSquare();

			squaresVector.push_back(newSquare);
		}

		updateGame(squaresVector, deltaTime);
		/*bool running = runningSquares(squaresVector);

		if (running == false) {
			glfwTerminate();
			return 0;
		}*/
		glClear(GL_COLOR_BUFFER_BIT);
		glBindVertexArray(VAO);

		for (int i = 0; i < squaresVector.size(); i++) {
			int colorIndex = static_cast<int>(i % programVector.size());
			glUseProgram(programVector[colorIndex]);
			glUniform2f(positionLocVector[colorIndex], squaresVector[i].pos.x, squaresVector[i].pos.y);
			glUniform1f(pointSizeLocVector[colorIndex], squaresVector[i].pointSize);

			glDrawArrays(GL_POINTS, 0, 1);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
		/*std::this_thread::sleep_for(std::chrono::milliseconds(200));*/
	}

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteProgram(programA);
	glDeleteProgram(programB);
	glDeleteProgram(programC);
	glDeleteProgram(programD);

	glfwTerminate();

	return 0;
}