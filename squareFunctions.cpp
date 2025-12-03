#include "globalConstants.h"
#include "squareFunction.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

void squaresInMotion(std::vector<Square>& squares) {

	for (auto& square : squares) {
		computeEdges(square);
		float groundY = GLOBAL_FLOOR + square.pointSize / WINDOW_HEIGHT;
		float wallRight = 1.0f + square.pointSize / WINDOW_WIDTH;
		float wallLeft = -1.0f + square.pointSize / WINDOW_WIDTH;
		float accelerationFriction = mu * Gravity;

		if (square.inMotion) {
			square.vel.vy = square.vel.vy - Gravity * deltaTime;
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

void computeEdgesVector(std::vector<Square>& squares) {
	for (auto& square : squares) {
		square.squaresGround = GLOBAL_FLOOR + square.pointSize / WINDOW_HEIGHT;
		square.halfWidth = square.pointSize / WINDOW_WIDTH;
		square.halfHeight = square.pointSize / WINDOW_HEIGHT;
		square.top = square.pos.y + square.halfHeight;
		square.bottom = square.pos.y - square.halfHeight;
		square.left = square.pos.x - square.halfWidth;
		square.right = square.pos.x + square.halfWidth;
	}
}

// bool function to check if square can be considerd a platfrom
// this function only considers if the squares y velocity is 0 meaning its not going up or down
bool isPlatform(Square& square) {
	computeEdges(square); // computes squares edges
	bool platform = false;
	if (std::abs(square.vel.vy) < 0.0005f) { // if squares velocity is close to 0
		platform = true; // say yes it can be used as a platform
	}

	return platform; // return true or flase if it can be used as a platform
}

// TODO: Square collides with another square IF its NOT WORLD WALL or WORLD BOTTOM/TOP
// This logically will mean that if it hits anything that isnt a WORLD boundry it has to be another square
bool squareCollides(Square& a, Square& b) {
	computeEdges(a);
	computeEdges(b);

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
	computeEdges(a);
	computeEdges(b);
	float tempVelX = a.vel.vx;
	float tempVelY = a.vel.vy;
	a.vel.vx = b.vel.vx;
	a.vel.vy = b.vel.vy;
	b.vel.vx = tempVelX;
	b.vel.vy = tempVelY;
}

bool isOnTop(Square& squareA, Square& squareB) {
	// recompute the edges of the squares we are comparing
	computeEdges(squareA);
	computeEdges(squareB);

	// test to see if the bottom square, square a
	bool verticalTouch = std::abs(squareA.bottom - squareB.top) < 0.005f;
	bool horizontalOverlap =
		(squareA.right > squareB.left) && (squareA.left < squareB.right);
	bool fallingOrResting = squareA.vel.vy <= 0.0f;

	return verticalTouch && horizontalOverlap && fallingOrResting;
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
	std::vector<int> sorted;
	sorted.reserve(squares.size());

	for (int i = 0; i < squares.size(); i++) {
		sorted.push_back(i);
	}
	computeEdgesVector(squares);
	// call sort once on sorted
	// std:: sort is in charge of all sorting. It will traverse the vector swapping values if lambda says so
	std::sort(sorted.begin(), sorted.end(),
		[&](int a, int b) { // pass 2 indices from sorted into lamda fucntion. Lamda function is the judge that says yes or no to swapping values
			return squares[a].left < squares[b].left; // checks square withn squares at index a and b and compare .left return either true or false
			// if condition is true say yes or no to swapping
			//continue this until full sorted
		}
	);
	// NOTE: Sorting with std:: sort isnt dont in any particular order, 

	std::vector<int> active; // holds active indices that may collide with others
	std::vector<std::pair<int, int>> candidatePairs; // will hold vector of pairs of canidates close to eachother

	// loop through sorted indices
	for (auto indexCurrent : sorted) {
		Square& current = squares[indexCurrent]; // create current square at current index from sorted
		computeEdges(current);

		int k = 0; // k itterator
		// on inital call we wont enter this becasue there isnt anything in active since active.size() = 0
		// while k < active.size()
		while (k < active.size()) {
			Square& other = squares[active[k]]; // create another currentSquare but this time at original index from squares vector
			computeEdges(other);
			// moving from lef to right
			if (other.right < current.left) { // if other.right value is less than current.left
				// essentially saying if the other square is all the way to the left of current square
				active.erase(active.begin() + k); // delete other from active
			}
			else { // else move to next other
				++k;
			}
		} // NOTE: Will only remove from active if other is completly to the left of current

		// now we loop through the active indices
		for (auto indexOther : active) {
			int i = indexOther; // i is equal to current index in active
			int j = indexCurrent; // j is equal to curernt index within sorted
			if (i > j) std::swap(i, j); // prevent same comparision of objects, essentailly checking if i isnt j
			candidatePairs.emplace_back(i, j); // pass those indices into a pair then into the vector of pairs
			
		}

		std::sort(candidatePairs.begin(), candidatePairs.end()); // sort the pairs

		// At this point we have pairs that are colliding,
		for (auto pair : candidatePairs) {
			// grab the indices within current pair and preform collision test or swapping velocites on them
			int i = pair.first;
			int j = pair.second;
			if (squareCollides(squares[i], squares[j])) {
				std::cout << "(" << i << "," << j << ")" << std::endl;
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
		active.push_back(indexCurrent); // push indexCurrent into active
		candidatePairs.clear();
	}
}