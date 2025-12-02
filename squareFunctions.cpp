#include "globalConstants.h"
#include "squareFunction.h"
#include <cmath>
#include <vector>

void squaresInMotion(std::vector<Square>& squares, float gravity, float deltaTime, float e, float mu) {

	for (auto& square : squares) {
		computeEdges(square);
		float groundY = GLOBAL_FLOOR + square.pointSize / WINDOW_HEIGHT;
		float wallRight = 1.0f + square.pointSize / WINDOW_WIDTH;
		float wallLeft = -1.0f + square.pointSize / WINDOW_WIDTH;
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
	}
}

void computeEdges(Square& square) {
	// for now we will only use bottom to check if its on the floor,
	// after will most likey change the function to allow objects to slide off its its past a boundry
	// like if objects are stacked to high and the top object is "passing" the boundry we will be slided off
	square.squaresGround = GLOBAL_FLOOR + square.pointSize / WINDOW_HEIGHT;
	square.halfWidth = square.pointSize / WINDOW_WIDTH;
	square.halfHeight = square.pointSize / WINDOW_HEIGHT;
	square.top = square.pos.y + square.halfHeight;
	square.bottom = square.pos.y - square.halfHeight;
	square.left = square.pos.x - square.halfWidth;
	square.right = square.pos.x + square.halfWidth;
}

// bool function to check if square is on the floor
bool checkFloorSupport(Square& square) { // pass in reference of a square object
	computeEdges(square); // computes its edges by calling the computeEdges fuction

	// check if squares bottom is <= its own normalized ground coordinate AND the abs of its y velocity is < than 0.000f; 
	if (square.bottom <= square.squaresGround && std::abs(square.vel.vy) < 0.005f) {
		// if conditions are true
		//sets squares touching ground value to true
		square.touchingGround = true;
	}
	// initally set to false so it will return false or true;
	return square.touchingGround;
}

// bool function to check if square can be considerd a platfrom
// this function only considers if the squares y velocity is 0 meaning its not going up or down
bool isPlatform(Square& square) {
	computeEdges(square); // computes squares edges
	bool platform = false;
	if (std::abs(square.vel.vy) < 0.05f) { // if squares velocity is close to 0
		platform = true; // say yes it can be used as a platform
	}

	return platform; // return true or flase if it can be used as a platform
}

// TODO: Square collides with another square IF its NOT WORLD WALL or WORLD BOTTOM/TOP
// This logically will mean that if it hits anything that isnt a WORLD boundry it has to be another square
bool squareCollides(Square& a, Square& b) {

	// gets the center of both squares. Since both have pointSize of 15 units this is fine for now
	// will not work if one or the other has a different point size
	float halfWidthA = a.pointSize / WINDOW_WIDTH;
	float halfHeightA = a.pointSize / WINDOW_HEIGHT;
	float halfWidthB = b.pointSize / WINDOW_WIDTH;
	float halfHeightB = b.pointSize / WINDOW_HEIGHT;
	float halfWidthSum = halfWidthA + halfWidthB;
	float halfHeightSum = halfHeightA+ halfHeightB;

	// difference in x and y values between x and y positions
	float dx = a.pos.x - b.pos.x;
	float dy = a.pos.y - b.pos.y;

	// get the absolute value of both differences
	dx = std::abs(dx);
	dy = std::abs(dy);

	// return true if both dx and dy are smaller that half (width/height)*2
	return dx < halfWidthSum && dy < halfHeightSum;
}

void swapVelocities(Square& a, Square& b) {
	float tempVelX = a.vel.vx;
	float tempVelY = a.vel.vy;
	a.vel.vx = b.vel.vx;
	a.vel.vy = b.vel.vy;
	b.vel.vx = tempVelX;
	b.vel.vy = tempVelY;
}

// function to use one square as a platform
// here we are assuming that square b has alreay stopped moving in the y direction
// and that a is still falling on top of it
void practicePlatform(Square& a, Square& b) { // pass in two square objects
	computeEdges(b); // compute the edge of square we want as platform (hard coding it for now)

	float halfHeightFalling = a.pointSize / WINDOW_HEIGHT; // calculate the half height of the fall square a
	float centerY = b.top + halfHeightFalling; // centerY is essentially the new calculated floor for that square

	a.pos.y = centerY; //set a.pos.y to the newly calculated bottom
	a.vel.vy = 0.0f; // stop moving the objects y velocity
	a.yMotion = false; // sets a.yMotion to false to indicated we arent moving inthe y direction anymore

	if (std::abs(a.vel.vx) < 0.01f) {
		a.xMotion = false;
	}
}

bool horizontalOverlap(Square& a, Square& b) {
	computeEdges(a);
	computeEdges(b);
	bool overLapping = false;
	if (a.left < b.right && b.left < a.right) {
		overLapping = true;
	}

	return overLapping;
}

float getFloorSupportHeight(Square& square) {
	float halfHeight = square.pointSize / WINDOW_HEIGHT;
	return halfHeight + GLOBAL_FLOOR;
}

float getPlatformSupportHeight(Square& falling, Square& platform) {
	computeEdges(platform);

	float halfHeightFalling = falling.pointSize / WINDOW_HEIGHT;

	return platform.top + halfHeightFalling;
}

float findSupportHeightForSquare(Square& falling, std::vector<Square>& squares) {
	float bestSupport = getFloorSupportHeight(falling);

	for (auto& squareP : squares) {

		if (&squareP == &falling) { // still falling ignore this one since it can be a platform
			continue;
		}

		if (!isPlatform(squareP)) {
			continue;
		}

		if (!horizontalOverlap(falling, squareP)) {
			continue;
		}

		computeEdges(falling);
		computeEdges(squareP);

		if (squareP.top > falling.pos.y) {
			continue;
		}

		Square& platform = squareP;
		computeEdges(platform);
		if (platform.top > falling.pos.y) continue;

		float cannidate = getPlatformSupportHeight(falling, platform);

		if (cannidate <= falling.pos.y && cannidate > bestSupport) {
			bestSupport = cannidate;
		}
	}

	return bestSupport;
}

void updateSqureWithPlatforms(Square& square, std::vector<Square>& squares) {
	square.vel.vy = square.vel.vy - Gravity * deltaTime;
	square.pos.y = square.pos.y + square.vel.vy * deltaTime;
	square.pos.x = square.pos.x + square.vel.vx * deltaTime;

	float supportHeight = findSupportHeightForSquare(square, squares);

	if (square.pos.y < supportHeight && square.vel.vy < 0) {
		square.pos.y = supportHeight;
		square.vel.vy = 0.0f;
		square.yMotion = false;
	}

	computeEdges(square);
}

void updateAllSquares(std::vector<Square>& squares) {
	for (auto& square : squares) {
		updateSqureWithPlatforms(square, squares);
	}
}

bool isOnTop(Square& squareA, Square& squareB) {
	// recompute the edges of the squares we are comparing
	computeEdges(squareA);
	computeEdges(squareB);

	// test to see if the bottom square, square a
	bool verticalTouch = std::abs(squareA.bottom - squareB.top) < 0.05f;
	bool horizontalOverlap =
		(squareA.right > squareB.left) && (squareA.left < squareB.right);
	bool fallingOrResting = squareA.vel.vy <= 0.0f;

	return verticalTouch && horizontalOverlap && fallingOrResting;
}

void mergeHelper(std::vector<Square> &leftArray, std:: vector<Square> &rightArray, std:: vector <Square> &sorted) {

	size_t leftSide = leftArray.size();
	size_t rightSide = rightArray.size();
	size_t i = 0;
	size_t left = 0;
	size_t right = 0;

	while (left < leftSide && right < rightSide) {
		if (leftArray[left].left < rightArray[right].left) {
			sorted[i] = leftArray[left];
			left++;
		}
		else {
			sorted[i] = rightArray[right];
			right++;
		}
		i++;
	}

	while (left < leftSide) {
		sorted[i] = leftArray[left];
		left++;
		i++;
	}

	while (right < rightSide) {
		sorted[i] = rightArray[right];
		right++;
		i++;
	}
}

void mergeSort(std::vector<Square> &squares) {
	size_t length = squares.size();
	if (length <= 1) {
		return;
	}
	size_t middle = length / 2;
	std::vector<Square> left(squares.begin(), squares.begin() + middle);
	std::vector<Square> right(squares.begin() + middle, squares.end());
	mergeSort(left);
	mergeSort(right);
	mergeHelper(left, right, squares);
}


void collisionDetectionNestedLoop(std::vector<Square>& squares) {
	for (int i = 0; i < squares.size(); i++) {
		for (int j = i + 1; j < squares.size(); j++) {
			if (squareCollides(squares[i], squares[j])) {
				if (isPlatform(squares[j]) && isOnTop(squares[i], squares[j])) {
					squares[i].pos.y = squares[j].top + squares[i].halfHeight;
					squares[i].vel.vy = 0.0f;
					squares[i].yMotion = false;
					squares[i].inMotion = false;
				}
				else if (isPlatform(squares[i]) && isOnTop(squares[j], squares[i])) {
					squares[j].pos.y = squares[i].top + squares[j].halfHeight;
					squares[j].vel.vy = 0.0f;
					squares[j].yMotion = false;
					squares[j].inMotion = false;
				}
				else {
					swapVelocities(squares[i], squares[j]);
				}
			}
		}
	}
}

void collisionDetectionSweepAndPrune(std::vector<Square> &squares) {
	mergeSort(squares);
	std::vector <int> active;
	std::vector<std::pair<Square, Square>> squarePairs;

	for (int i = 0; i < squares.size(); i++) {

		Square &current = squares[i];

		int k = 0;

		while (k < active.size()) {
			Square& other = squares[active[k]];
			if (other.right < current.left) {
				active.erase(active.begin() + k);
			}
			else {
				k++;
			}
		}


		for (int &index : active) {
			Square& A = squares[index];
			Square& B = current;

			if (squareCollides(A, B)) {
				if (isPlatform(B) && isOnTop(A, B)) {
					A.pos.y = B.top + A.halfHeight;
					A.vel.vy = 0.0f;
					A.yMotion = false;
					A.inMotion = false;
				}
				else if (isPlatform(A) && isOnTop(B, A)) {
					B.pos.y = A.top + B.halfHeight;
					B.vel.vy = 0.0f;
					B.yMotion = false;
					B.inMotion = false;
				}
				else {
					swapVelocities(A, B);
				}
			}
		}
		active.push_back(i);
	}
}