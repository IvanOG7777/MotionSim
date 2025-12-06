#pragma once
#include <vector>
#include "globalConstants.h"
#include <string>

struct Position {
	float x;
	float y;
};

struct Velocity {
	float vx;
	float vy;
};

struct Wall {
	float rightWall;
	float leftWall;
	float topWall;
	float bottomWall;
};

struct Edges {
	float top;
	float bottom;
	float left;
	float right;
};

struct MotionValues {
	bool yMotion = true;
	bool xMotion = true;
	bool inMotion = true;
};

struct Halfs {
	float halfWidth;
	float halfHeight;
};

class Square {
public:
	Position pos;
	Position prevPos; 
	Velocity vel;
	Wall wall;
	Edges edges;
	MotionValues motionValues;
	Halfs halfs;
	float pointSize;
	bool touchingGround = false;
	float groundY;
	float squaresGround;
	std::string name;
};
bool runningSquares(std::vector<Square>& squares);
void squaresInMotion(std::vector<Square>& squares, float deltaTime);
void computeEdges(Square& square);
void computeEdgesVector(std::vector<Square>& squares);
bool isPlatform(Square& square);
bool squareCollides(Square& a, Square& b);
void swapVelocities(Square& a, Square& b, bool xAxisChange, bool yAxisChange);
bool isOnTop(Square& squareA, Square& squareB);
void collisionDetectionNestedLoop(std::vector<Square>& squares);
void keepInBounds(Square& square, bool isHorizontal);
void collisionDetectionSweepAndPrune(std::vector<Square> &squares);
time_t getCurrentTime();
void updateGame(std::vector<Square>& squares, float dt);