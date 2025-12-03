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

class Square {
public:
	Position pos;
	Velocity vel;
	float pointSize;
	bool yMotion = true;
	bool xMotion = true;
	bool inMotion = true;
	bool touchingGround = false;
	float groundY;
	float wallRight;
	float wallLeft;
	float mass;
	float squaresGround;
	float halfWidth;
	float halfHeight;
	float top;
	float bottom;
	float left;
	float right;
	std::string name;
};

void squaresInMotion(std::vector<Square>& squares);
void computeEdges(Square& square);
bool checkFloorSupport(Square& square);
bool isPlatform(Square& square);
bool squareCollides(Square& a, Square& b);
void swapVelocities(Square& a, Square& b);
void practicePlatform(Square& a, Square& b);
bool horizontalOverlap(Square& a, Square& b);
float getFloorSupportHeight(Square& square);
float getPlatformSupportHeight(Square& falling, Square& platform);
float findSupportHeightForSquare(Square& falling, std::vector<Square>& squares);
void updateSqureWithPlatforms(Square& square, std::vector<Square>& squares);
void updateAllSquares(std::vector<Square>& squares);
bool isOnTop(Square& squareA, Square& squareB);
void collisionDetectionNestedLoop(std::vector<Square>& squares);
void collisionDetectionSweepAndPrune(std::vector<Square> &squares);
void SAP(std::vector<Square>& squares);