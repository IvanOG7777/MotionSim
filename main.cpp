#include <iostream>
#include <cmath>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct Position {
	float x;
	float y;
};

struct Velocity {
	float vx;
	float vy;
};

class Square {
public:
	Position pos;
	Velocity vel;
	float pointSize;
	bool yMotion = true;
	bool xMotion = true;
	bool inMotion = true;
	float groundY;
	float wallRight;
	float wallLeft;
	float mass;
	std::string name;
};

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


bool squareInMotion(Square& square, float gravity, float deltaTime, float e, float mu, float pointSize, float windowWidth, float windowHeight) {
	float floorY = -1.0f;
	float groundY = floorY + pointSize / windowHeight;
	float wallRight = 1.0f + pointSize / windowWidth;
	float wallLeft = -1.0f + pointSize / windowWidth;
	float accelerationFriction = mu * gravity;

	if (square.inMotion) {
		square.vel.vy = square.vel.vy - gravity * deltaTime;
		square.pos.x = square.pos.x + square.vel.vx * deltaTime;
		square.pos.y += square.vel.vy * deltaTime;

		if (square.pos.y <= groundY) {
			square.pos.y = groundY; // snap the pos.y to be the groundY

			//check if vel.vy is negative meaning its falling
			// if it is we enter the block
			if (square.vel.vy < 0.0f) {
				// invert velocity upward and reduce velocity a bit by e
				square.vel.vy = -square.vel.vy * e;

				// check the ABS of current velocity is very small
				// this prevents very small bounces
				if (std::abs(square.vel.vy) < 0.05f) {
					square.vel.vy = 0.0f; // make velocity 0
					square.pos.y = groundY;
					square.yMotion = false;
				}
			}
		}

		// checks if both pos.y and the velocity are at 0. No more y movement
		if (square.pos.y == groundY && square.vel.vy == 0.0f) {
			// we check if 
			if (square.vel.vx > 0.0f) {
				square.vel.vx -= accelerationFriction * deltaTime;
				if (square.vel.vx < 0.0f) {
					square.vel.vx = 0.0f;
				}
			}
			else if (square.vel.vx < 0.0f) {
				square.vel.vx += accelerationFriction * deltaTime;
				if (square.vel.vx > 0.0f) {
					square.vel.vx = 0.0f;
				}
			}

			if (std::abs(square.vel.vx) < 0.5f) {
				square.vel.vx = 0.0f;
				square.xMotion = false;
			}
		}

		//two if block to keep ball within the boundries

		//if pos.x is greater than the right wall NDC 1.0f AND vel.vs is greater than 0
		if (square.pos.x >= wallRight && square.vel.vx > 0) {
			square.pos.x = wallRight; // snap the position of x to the right wall
			square.vel.vx = -square.vel.vx * e; // invert the velocity backwards and reduce velocity a bit by e
		}

		//if pos.x is less than the left wall NDC -1.0f AND vel.vs is less than 0
		if (square.pos.x <= wallLeft && square.vel.vx < 0) {
			square.pos.x = wallLeft; // snap the position of x to the left wall
			square.vel.vx = -square.vel.vx * e; // invert the velocity backwards and reduce velocity a bit by e
		}

		if (!square.xMotion && !square.yMotion) square.inMotion = false;
	}

	return square.inMotion;
}

bool squareCollides(Square& a, Square& b, float windowWidth, float windowHeight) {

	// gets the center of both squares. Since both have pointSize of 15 units this is fine for now
	// will not work if one or the other has a different point size
	float halfWidthA = a.pointSize / windowWidth;
	float halfHeightA = a.pointSize / windowHeight;
	float halfWidthB = b.pointSize / windowWidth;
	float halfHeightB = b.pointSize / windowHeight;

	// difference in x and y values between x and y positions
	float dx = a.pos.x - b.pos.x;
	float dy = a.pos.y - b.pos.y;

	// get the absolute value of both differences
	dx = std::abs(dx);
	dy = std::abs(dy);

	// return true if both dx and dy are smaller that half (width/height)*2
	return dx < halfWidthA + halfWidthB && dy < halfHeightA + halfHeightB;
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

void swapVelocities(Square& a, Square& b) {
	float tempVelX = a.vel.vx;
	float tempVelY = a.vel.vy;
	a.vel.vx = b.vel.vx;
	a.vel.vy = b.vel.vy;
	b.vel.vx = tempVelX;
	b.vel.vy = tempVelY;
}

int main() {

	Square squareA;
	Square squareB;
	Square squareC;
	std::vector<Square> squaresVector;


	float Gravity = 2.0f; //make sure to keep gravity in NDC. if not it will be too fast
	// calculation is done by 9.8 / wanted real life height
	// ex: 9.8 / 5 meters = 1.96 in NDC
	float deltaTime = 0.016f;
	int currentStep = 0;
	float e = 0.8;
	bool isInMotion = true;
	int bounceCount = 0;
	int hitWallCount = 0;
	float mu = 1.0;
	float accelerationFriction = mu * Gravity;
	int index = 0;


	// start position at middle of screen. Will Change to left bottom to simulate the motion of the object
	squareA.pos.x = -0.8f;
	squareA.pos.y = 0.8f;

	squareA.vel.vx = 0.0f;
	squareA.vel.vy = -0.6f;

	squareA.pointSize = 15.0f;
	squareA.name = "SquareA";

	squareB.pos.x = -0.8f;
	squareB.pos.y = 0.6f;

	squareB.vel.vx = 0.0f;
	squareB.vel.vy = -0.3f;

	squareB.pointSize = 30.0f;
	squareB.name = "SquareB";

	squaresVector.push_back(squareA);
	squaresVector.push_back(squareB);


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



	float windowWidth = 1280.0f;
	float windowHeight = 720.0f;

	GLuint programA = createProgram(vertexShaderSrc, fragmentShaderSrcA);
	GLuint programB = createProgram(vertexShaderSrc, fragmentShaderSrcB);
	if (!programA || !programB) {
		glfwTerminate();
		return -1;
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

	while (!glfwWindowShouldClose(window)) {
		bool anyMovement = false;

		for (auto& square : squaresVector) {
			bool moving = squareInMotion(square, Gravity, deltaTime, e, mu, square.pointSize, windowWidth, windowHeight);
			std::cout << "Position of " << square.name;
			std::cout << "(x,y): " << square.pos.x << "," << square.pos.y;
			std::cout << std::endl;

			anyMovement = anyMovement || moving;
		}

		if (squareCollides(squaresVector[0], squaresVector[1], windowWidth, windowHeight)) {
			swapVelocities(squaresVector[0], squaresVector[1]);
		}

		if (!anyMovement) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}


		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(VAO);

		for (size_t i = 0; i < squaresVector.size(); ++i) {
			Square& square = squaresVector[i];

			if (i == 0) {
				// squareA
				glUseProgram(programA);
				glUniform2f(uPositionLocA, square.pos.x, square.pos.y);
				glUniform1f(uPointSizeLocA, square.pointSize);
			}
			else {
				// squareB
				glUseProgram(programB);
				glUniform2f(uPositionLocB, square.pos.x, square.pos.y);
				glUniform1f(uPointSizeLocB, square.pointSize);
			}

			glDrawArrays(GL_POINTS, 0, 1);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteProgram(programA);
	glDeleteProgram(programB);

	glfwTerminate();

	return 0;
}