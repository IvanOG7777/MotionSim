#include "globalConstants.h"
#include "squareFunction.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

bool runningSquares(std::vector<Square>& squares) {
	bool isRunning = false;

	for (auto& square : squares) {
		if (square.motionValues.inMotion == true) {
			isRunning = true;
			break;
		}
	}
	return isRunning;
}

void squaresInMotion(std::vector<Square>& squares) {

	for (auto& square : squares) {
		computeEdges(square);
		float groundY = GLOBAL_FLOOR + square.pointSize / WINDOW_HEIGHT;
		float wallRight = 1.0f + square.pointSize / WINDOW_WIDTH;
		float wallLeft = -1.0f + square.pointSize / WINDOW_WIDTH;
		float wallTop = 1.0f + square.pointSize / WINDOW_HEIGHT;
		float wallBottom = -1.0f + square.pointSize / WINDOW_HEIGHT;
		float accelerationFriction = mu * Gravity;
		/*std::cout << square.name << " gound value: " << square.groundY << "\n";
		std::cout << square.name << " y value: " << square.pos.y << "\n";
		std::cout << "\n";*/

		if (square.motionValues.inMotion) {
			square.vel.vy = square.vel.vy - Gravity * deltaTime;
			square.pos.x = square.pos.x + square.vel.vx * deltaTime;
			square.pos.y += square.vel.vy * deltaTime;

			if (square.pos.y <= groundY || square.pos.y <= GLOBAL_FLOOR) {
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
						square.motionValues.yMotion = false;
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
					square.motionValues.xMotion = false;
				}
			}

			//two if block to keep ball within the boundries

			/*keepInBounds(square);*/

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

			if (!square.motionValues.xMotion && !square.motionValues.yMotion) square.motionValues.inMotion = false;
		}
	}
}

void computeEdges(Square& square) {
	// for now we will only use bottom to check if its on the floor,
	// after will most likey change the function to allow objects to slide off its its past a boundry
	// like if objects are stacked to high and the top object is "passing" the boundry we will be slided off
	square.squaresGround = GLOBAL_FLOOR + square.pointSize / WINDOW_HEIGHT;
	square.halfs.halfWidth = square.pointSize / WINDOW_WIDTH;
	square.halfs.halfHeight = square.pointSize / WINDOW_HEIGHT;
	square.edges.top = square.pos.y + square.halfs.halfHeight;
	square.edges.bottom = square.pos.y - square.halfs.halfHeight;
	square.edges.left = square.pos.x - square.halfs.halfWidth;
	square.edges.right = square.pos.x + square.halfs.halfWidth;
}

void computeEdgesVector(std::vector<Square>& squares) {
	for (auto& square : squares) {
		square.squaresGround = GLOBAL_FLOOR + square.pointSize / WINDOW_HEIGHT;
		square.halfs.halfWidth = square.pointSize / WINDOW_WIDTH;
		square.halfs.halfHeight = square.pointSize / WINDOW_HEIGHT;
		square.edges.top = square.pos.y + square.halfs.halfHeight;
		square.edges.bottom = square.pos.y - square.halfs.halfHeight;
		square.edges.left = square.pos.x - square.halfs.halfWidth;
		square.edges.right = square.pos.x + square.halfs.halfWidth;
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
	float halfWidthA = a.halfs.halfWidth;
	float halfHeightA = a.halfs.halfHeight;
	float halfWidthB = b.halfs.halfWidth;
	float halfHeightB = b.halfs.halfHeight;
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

void swapVelocities(Square& a, Square& b, bool xAxisChange, bool yAxisChange) {
	computeEdges(a);
	computeEdges(b);
	if (xAxisChange == true && yAxisChange == false) {
		float tempVelX = a.vel.vx;

		a.vel.vx = b.vel.vx;
		b.vel.vx = tempVelX;
	}
	else if (xAxisChange == false && yAxisChange == true) {
		float tempVelY = a.vel.vy;
		a.vel.vy = b.vel.vy;
		b.vel.vy = tempVelY;
	}
	else if (xAxisChange == true && yAxisChange == true) {
		float tempVelX = a.vel.vx;
		float tempVelY = a.vel.vy;
		a.vel.vx = b.vel.vx;
		a.vel.vy = b.vel.vy;
		b.vel.vx = tempVelX;
		b.vel.vy = tempVelY;
	}
	else {
		return;
	}
}

bool isOnTop(Square& squareA, Square& squareB) {
	// recompute the edges of the squares we are comparing
	computeEdges(squareA);
	computeEdges(squareB);

	// test to see if the bottom square, square a
	bool verticalTouch = std::abs(squareA.edges.bottom - squareB.edges.top) < 0.005f;
	bool horizontalOverlap =
		(squareA.edges.right > squareB.edges.left) && (squareA.edges.left < squareB.edges.right);
	bool fallingOrResting = squareA.vel.vy <= 0.0f;

	return verticalTouch && horizontalOverlap && fallingOrResting;
}

void collisionDetectionNestedLoop(std::vector<Square>& squares) {
	for (int i = 0; i < squares.size(); i++) {
		for (int j = i + 1; j < squares.size(); j++) {
			if (squareCollides(squares[i], squares[j])) {
				if (isPlatform(squares[j]) && isOnTop(squares[i], squares[j])) {
					squares[i].pos.y = squares[j].edges.top + squares[i].halfs.halfHeight;
					squares[i].vel.vy = 0.0f;
					squares[i].motionValues.yMotion = false;
					squares[i].motionValues.inMotion = false;
				}
				else if (isPlatform(squares[i]) && isOnTop(squares[j], squares[i])) {
					squares[j].pos.y = squares[i].edges.top + squares[j].halfs.halfHeight;
					squares[j].vel.vy = 0.0f;
					squares[j].motionValues.yMotion = false;
					squares[j].motionValues.inMotion = false;
				}
				else {
					swapVelocities(squares[i], squares[j], true, true);
				}
			}
		}
	}
}

void keepInBounds(Square& square) {
	computeEdges(square);
	float wallLeft = -1.0f + square.halfs.halfWidth;
	float wallRight = 1.0f - square.halfs.halfWidth;
	float wallBottom = -1.0f + square.halfs.halfHeight;
	float wallTop = 1.0f - square.halfs.halfHeight;

	if (square.pos.x >= wallRight && square.vel.vx > 0) {
		square.pos.x = wallRight;
		square.vel.vx = -square.vel.vx * e;
	}
	if (square.pos.x <= wallLeft && square.vel.vx < 0) {
		square.pos.x = wallLeft;
		square.vel.vx = -square.vel.vx * e;
	}

	// keep inside vertical bounds (top)
	if (square.pos.y >= wallTop && square.vel.vy > 0) {
		square.pos.y = wallTop;
		square.vel.vy = -square.vel.vy * e;
	}

	if (square.pos.y <= wallBottom && square.vel.vy > 0) {
		square.pos.y = wallBottom;
		square.vel.vy = -square.vel.vy * e;
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
			return squares[a].edges.left < squares[b].edges.left; // checks square withn squares at index a and b and compare .left return either true or false
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
			if (other.edges.right < current.edges.left) { // if other.right value is less than current.left
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
			Square& a = squares[i];
			Square& b = squares[j];
			if (squareCollides(squares[i], squares[j])) {
				/*std::cout << "(" << i << "," << j << ")" << "\n";*/
				if (isPlatform(squares[j]) && isOnTop(squares[i], squares[j])) {
					squares[i].pos.y = squares[j].edges.top + squares[i].halfs.halfHeight;
					squares[i].pos.y = squares[j].edges.top;
					squares[i].vel.vy = 0.0f;
					squares[i].motionValues.yMotion = false;
					squares[i].motionValues.inMotion = false;
				}
				else if (isPlatform(squares[i]) && isOnTop(squares[j], squares[i])) {
					squares[j].pos.y = squares[i].edges.top + squares[j].halfs.halfHeight;
					squares[j].pos.y = squares[i].edges.top;
					squares[j].vel.vy = 0.0f;
					squares[j].motionValues.yMotion = false;
					squares[j].motionValues.inMotion = false;
				}
				else {
					// will calculate the amount of distance the squares are overlapping in both the x and y axis
					float overlapX = std::min(a.edges.right, b.edges.right) - std::max(a.edges.left, b.edges.left);
					float overlapY = std::min(a.edges.top, b.edges.top) - std::max(a.edges.bottom, b.edges.bottom);

					// check if overlapX is less than overlapY
					if (overlapX < overlapY) {

						// this is for HORIZONTAL collison
						float correction = overlapX / 2; // calculate the correction of the shift

						// if a is to the left of b
						if (a.pos.x < b.pos.x) {
							// adjust thier x positions wiht correction
							a.pos.x -= correction;
							b.pos.x += correction;
							
							float wallRightA = 1.0f + a.pointSize / WINDOW_WIDTH;
							float wallLeftA = -1.0f + a.pointSize / WINDOW_WIDTH;
							float wallRightB = 1.0f + b.pointSize / WINDOW_WIDTH;
							float wallLeftB = -1.0f + b.pointSize / WINDOW_WIDTH;

							if (a.pos.x >= wallRightA && a.vel.vx > 0) {
								a.pos.x = wallRightA; // snap the position of x to the right wall
								a.vel.vx = -a.vel.vx * e; // invert the velocity backwards and reduce velocity a bit by e
							}

							//if pos.x is less than the left wall NDC -1.0f AND vel.vs is less than 0
							if (a.pos.x <= wallLeftA && a.vel.vx < 0) {
								a.pos.x = wallLeftA; // snap the position of x to the left wall
								a.vel.vx = -a.vel.vx * e; // invert the velocity backwards and reduce velocity a bit by e
							}

							if (b.pos.x >= wallRightB && b.vel.vx > 0) {
								b.pos.x = wallRightB; // snap the position of x to the right wall
								b.vel.vx = -b.vel.vx * e; // invert the velocity backwards and reduce velocity a bit by e
							}

							//if pos.x is less than the left wall NDC -1.0f AND vel.vs is less than 0
							if (b.pos.x <= wallLeftB && b.vel.vx < 0) {
								b.pos.x = wallLeftB; // snap the position of x to the left wall
								b.vel.vx = -b.vel.vx * e; // invert the velocity backwards and reduce velocity a bit by e
							}

						}
						else { // if a is to the right of b
							// adjust thier x positions wiht correction
							a.pos.x += correction;
							b.pos.x -= correction;

							float wallRightA = 1.0f + a.pointSize / WINDOW_WIDTH;
							float wallLeftA = -1.0f + a.pointSize / WINDOW_WIDTH;
							float wallRightB = 1.0f + b.pointSize / WINDOW_WIDTH;
							float wallLeftB = -1.0f + b.pointSize / WINDOW_WIDTH;

							if (a.pos.x >= wallRightA && a.vel.vx > 0) {
								a.pos.x = wallRightA; // snap the position of x to the right wall
								a.vel.vx = -a.vel.vx * e; // invert the velocity backwards and reduce velocity a bit by e
							}

							//if pos.x is less than the left wall NDC -1.0f AND vel.vs is less than 0
							if (a.pos.x <= wallLeftA && a.vel.vx < 0) {
								a.pos.x = wallLeftA; // snap the position of x to the left wall
								a.vel.vx = -a.vel.vx * e; // invert the velocity backwards and reduce velocity a bit by e
							}

							if (b.pos.x >= wallRightB && b.vel.vx > 0) {
								b.pos.x = wallRightB; // snap the position of x to the right wall
								b.vel.vx = -b.vel.vx * e; // invert the velocity backwards and reduce velocity a bit by e
							}

							//if pos.x is less than the left wall NDC -1.0f AND vel.vs is less than 0
							if (b.pos.x <= wallLeftB && b.vel.vx < 0) {
								b.pos.x = wallLeftB; // snap the position of x to the left wall
								b.vel.vx = -b.vel.vx * e; // invert the velocity backwards and reduce velocity a bit by e
							}
						}
						// finally swap thier velocites
						/*swapVelocities(a, b);*/
						swapVelocities(a, b, true, false);
					}
					else {
						// this is for VERTICAL collison
						float correction = overlapY / 2;
						if (a.pos.y < b.pos.y) {
							a.pos.y -= correction;
							b.pos.y += correction;

							float wallTopA = 1.0f + a.pointSize / WINDOW_HEIGHT;
							float wallBottomA = -1.0f + a.pointSize / WINDOW_HEIGHT;
							float wallTopB = 1.0f + b.pointSize / WINDOW_HEIGHT;
							float wallBottomB = -1.0f + b.pointSize / WINDOW_HEIGHT;

							if (a.pos.y >= wallTopA && a.vel.vy > 0) {
								a.pos.y = wallTopA; // snap the position of x to the right wall
								a.vel.vy = -a.vel.vy * e; // invert the velocity backwards and reduce velocity a bit by e
							}

							//if pos.x is less than the left wall NDC -1.0f AND vel.vs is less than 0
							if (a.pos.y <= wallBottomA && a.vel.vy < 0) {
								a.pos.y = wallBottomA; // snap the position of x to the left wall
								a.vel.vy = -a.vel.vy * e; // invert the velocity backwards and reduce velocity a bit by e
							}

							if (b.pos.y >= wallTopB && b.vel.vy > 0) {
								b.pos.y = wallTopB; // snap the position of x to the right wall
								b.vel.vy = -b.vel.vy * e; // invert the velocity backwards and reduce velocity a bit by e
							}

							//if pos.x is less than the left wall NDC -1.0f AND vel.vs is less than 0
							if (b.pos.y <= wallBottomB && b.vel.vy < 0) {
								b.pos.y = wallBottomB; // snap the position of x to the left wall
								b.vel.vy = -b.vel.vy * e; // invert the velocity backwards and reduce velocity a bit by e
							}

						}
						else {
							a.pos.y += correction;
							b.pos.y -= correction;

							float wallTopA = 1.0f + a.pointSize / WINDOW_HEIGHT;
							float wallBottomA = -1.0f + a.pointSize / WINDOW_HEIGHT;
							float wallTopB = 1.0f + b.pointSize / WINDOW_HEIGHT;
							float wallBottomB = -1.0f + b.pointSize / WINDOW_HEIGHT;

							if (a.pos.y >= wallTopA && a.vel.vy > 0) {
								a.pos.y = wallTopA; // snap the position of x to the right wall
								a.vel.vy = -a.vel.vy * e; // invert the velocity backwards and reduce velocity a bit by e
							}

							//if pos.x is less than the left wall NDC -1.0f AND vel.vs is less than 0
							if (a.pos.y <= wallBottomA && a.vel.vy < 0) {
								a.pos.y = wallBottomA; // snap the position of x to the left wall
								a.vel.vy = -a.vel.vy * e; // invert the velocity backwards and reduce velocity a bit by e
							}

							if (b.pos.y >= wallTopB && b.vel.vy > 0) {
								b.pos.y = wallTopB; // snap the position of x to the right wall
								b.vel.vy = -b.vel.vy * e; // invert the velocity backwards and reduce velocity a bit by e
							}

							//if pos.x is less than the left wall NDC -1.0f AND vel.vs is less than 0
							if (b.pos.y <= wallBottomB && b.vel.vy < 0) {
								b.pos.y = wallBottomB; // snap the position of x to the left wall
								b.vel.vy = -b.vel.vy * e; // invert the velocity backwards and reduce velocity a bit by e
							}
						}

						/*swapVelocities(a, b);*/
						swapVelocities(a, b, false, true);
					}
				}
			}
		}
		active.push_back(indexCurrent); // push indexCurrent into active
		candidatePairs.clear(); // clear the canidate pairs to have fresh pair on next itteration.
	}
}