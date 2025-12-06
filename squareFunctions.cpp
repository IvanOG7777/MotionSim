#include "globalConstants.h"
#include "squareFunction.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
#include <ctime>

// function to check is all squares are still in motion
bool runningSquares(std::vector<Square>& squares) { // pass in vector of squares
	bool isRunning = false; // isRunning intial state is false

	// loop over all squares
	for (auto& square : squares) {
		if (square.motionValues.inMotion == true) { // check if current square is still in motion
			isRunning = true; // make running true
			break; // break out and move to next square
		}
	}
	return isRunning; // return isRunning true or false
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
	square.wall.rightWall = 1.0f + square.pointSize / WINDOW_WIDTH;
	square.wall.leftWall = -1.0f + square.pointSize / WINDOW_WIDTH;
	square.wall.topWall = 1.0f + square.pointSize / WINDOW_HEIGHT;
	square.wall.bottomWall = -1.0f + square.pointSize / WINDOW_HEIGHT;
}

// Essentially the same function as above for for a vector of squares
void computeEdgesVector(std::vector<Square>& squares) {
	for (auto& square : squares) {
		square.squaresGround = GLOBAL_FLOOR + square.pointSize / WINDOW_HEIGHT;
		square.halfs.halfWidth = square.pointSize / WINDOW_WIDTH;
		square.halfs.halfHeight = square.pointSize / WINDOW_HEIGHT;
		square.edges.top = square.pos.y + square.halfs.halfHeight;
		square.edges.bottom = square.pos.y - square.halfs.halfHeight;
		square.edges.left = square.pos.x - square.halfs.halfWidth;
		square.edges.right = square.pos.x + square.halfs.halfWidth;
		square.wall.rightWall = 1.0f + square.pointSize / WINDOW_WIDTH;
		square.wall.leftWall = -1.0f + square.pointSize / WINDOW_WIDTH;
		square.wall.topWall = 1.0f + square.pointSize / WINDOW_HEIGHT;
		square.wall.bottomWall = -1.0f + square.pointSize / WINDOW_HEIGHT;
	}
}

// function to check if square is in bounds of the map
void keepInBounds(Square& square, bool isHorizontal) { // pass in the square we are checking and if we are checking horizontal or vertical (bool)
	computeEdges(square); // compute its egdes to get up to date values

	// if isHorizontal is true, we are checking for left and right walls
	if (isHorizontal) {
		// compute squares left and right wall values.
		// each square has it own boundies becasue of its point size, square A might not have the same wall values as square B
		float wallLeft = -1.0f + square.halfs.halfWidth;
		float wallRight = 1.0f - square.halfs.halfWidth;
		// check if s.pos.x is greater than its wallRight value and if its x velocity is greater than 0, meaning we are still moving toawrds the wall
		if (square.pos.x >= wallRight && square.vel.vx > 0) {
			square.pos.x = wallRight; // snap its x position to its wallRitght value
			square.vel.vx = -square.vel.vx * e; // and reverse its velocity
		}

		// same thing but for left
		if (square.pos.x <= wallLeft && square.vel.vx < 0) {
			square.pos.x = wallLeft;
			square.vel.vx = -square.vel.vx * e;
		}
	}
	else if (!isHorizontal) {
		float wallBottom = -1.0f + square.halfs.halfHeight;
		float wallTop = 1.0f - square.halfs.halfHeight;


		// check if s.pos.y is greater than its wallTop value and if its y velocity is greater than 0, meaning we are still moving toawrds the wall
		if (square.pos.y >= wallTop && square.vel.vy > 0) {
			square.pos.y = wallTop; // snap its y position to its wallTop value
			square.vel.vy = -square.vel.vy * e; // and reverse its velocity
		}

		// same thing but for bottom
		if (square.pos.y <= wallBottom && square.vel.vy < 0) {
			square.pos.y = wallBottom;
			square.vel.vy = -square.vel.vy * e;
		}
	}
}


//POTENTIAL FIX:
//	Tuneling happens becasue the velocity in y direction is faster than the distance y from a and b

// function used to move each square individually with physics
void squaresInMotion(std::vector<Square>& squares, float deltaTime) { // pass in the vector of squares

	// loop over each square within the vector
	for (auto& square : squares) {
		computeEdges(square); // compute its edges and wall values first to have up to date values
		float groundY = GLOBAL_FLOOR + square.pointSize / WINDOW_HEIGHT;

		//check if square is currently in motion
		if (square.motionValues.inMotion) {
			/*std::cout << "Is " << square.name << " moving?: " << (square.motionValues.inMotion ? "TRUE" : "FALSE") << "\n";*/
			// if it is recompute its velocity and x/y position
			// I think this is where the issue persists of the red square phasing through the blue on initial start.
			// secifically this line is causing the issue: square.vel.vy = square.vel.vy - Gravity * deltaTime; when delta time is too big
			/*std::cout << "Calculating physics for: " << square.name << "\n";*/
			square.vel.vy = square.vel.vy - Gravity * deltaTime;
			square.pos.x = square.pos.x + (square.vel.vx * deltaTime);
			square.pos.y += square.vel.vy * deltaTime;

			// check squares y position is less than its ground ground value or the GLOBAL_FLOOR value (-1.0f)
			if (square.pos.y < groundY) {
				/*std::cout << "Snapping" << square.name << " to its ground value" << "\n";*/
				square.pos.y = groundY; // snap the pos.y to be the groundY

				//check if vel.vy is negative meaning its falling
				// if it is we enter the block
				if (square.vel.vy <= 0.0f) {
					// invert velocity upward and reduce velocity a bit by e
					square.vel.vy = -square.vel.vy * e;


					// check the ABS of current velocity is very small
					// this prevents very small bounces
					if (std::abs(square.vel.vy) <= 0.05f) {
						square.vel.vy = 0.0f; // make velocity 0
						square.pos.y = groundY;
						square.motionValues.yMotion = false;
					}
				}
			}
			// checks if both pos.y and the velocity are at 0. No more y movement
			if (square.pos.y == groundY && square.vel.vy == 0.0f) {
				// Check if the velocity in the x axis is positive, meaning we are moving to the right
				if (square.vel.vx > 0.0f) {
					square.vel.vx -= accelerationFriction * deltaTime; // subtract a bit from the velocity using friction * time
					if (std::abs(square.vel.vx) <= 0.15f) { // check if the velocity 
						square.vel.vx = 0.0f;
						square.motionValues.xMotion = false;
					}
				}
				else if (square.vel.vx < 0.0f) {
					square.vel.vx += accelerationFriction * deltaTime;
					if (std::abs(square.vel.vx) <= 0.15f) {
						square.vel.vx = 0.0f;
						square.motionValues.xMotion = false;
					}
				}
			}

			keepInBounds(square, true);
			keepInBounds(square, false);

			if (!square.motionValues.xMotion && !square.motionValues.yMotion) square.motionValues.inMotion = false;
		}
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

// bool function to detect if squares have collided
bool squareCollides(Square& a, Square& b) { // parameters are 2 square objects
	computeEdges(a);
	computeEdges(b);
	// recompute thier edges are current state to have up to date values

	// calculate the half height and width of both squares
	float halfWidthSum = a.halfs.halfWidth + a.halfs.halfWidth;
	float halfHeightSum = a.halfs.halfHeight+ b.halfs.halfHeight;

	// calcualte the distance between the squares x and y current positions
	float dx = std:: abs(a.pos.x - b.pos.x);
	float dy = std:: abs(a.pos.y - b.pos.y);

	// check if the distance betwen square in X axis is greater than thier halfWidthSum
	// if it is we are to far away on the x axis they we know they dont collide
	if (dx >= halfWidthSum) {
		return false;
	}

	// check if the distance between squares in the Y axis is less than thier halfHeightSum
	// if distance is less return true meaning the squares are either touching or are overlapping with eachother
	if (dy < halfHeightSum) {
		return true;
	}

	// calculate the relative velocity between a and b y velocites
	// tess us how fast a is approaching or leaving b along the y axis
	float relativeVy = a.vel.vy - b.vel.vy;

	// if that realative velocity is 0 squares cant get any closer vertically
	// if both have the same downward y velocity even if falling together, they stay the same distance apart
	// no new collison can happen in current frame
	if (relativeVy == 0.0f) {
		return false;
	}

	// this block checks if both squares are moving towards eachother, checks if a is either above or below b in the y axis
	if ((a.pos.y > b.pos.y && relativeVy >= 0.0f) || (a.pos.y < b.pos.y && relativeVy <= 0.0f)) {
		return false;
	}

	// calculate the gap, which is the distance between the two squares minus their half Height sum
	float gap = dy - halfHeightSum;

	// maxAproach calculates the minimum distance the squares will vertically close within a frame.
	float maxApproach = std::abs(relativeVy) * deltaTime;

	// if squares cna move  far enough in current fram to eleminate the gap we have collided in this frame
	return maxApproach >= gap;
}

void swapVelocities(Square& a, Square& b, bool xAxisChange, bool yAxisChange) {
	computeEdges(a);
	computeEdges(b);
	if (xAxisChange == true && yAxisChange == false) {
		float tempVelX = a.vel.vx;

		a.vel.vx = b.vel.vx;
		b.vel.vx = tempVelX;

		/*std::cout << a.name << " and " << b.name << " have swapped velocities on x axis" << "\n";*/
	}
	else if (xAxisChange == false && yAxisChange == true) {
		float tempVelY = a.vel.vy;
		a.vel.vy = b.vel.vy;
		b.vel.vy = tempVelY;

		/*std::cout << a.name << " and " << b.name << " have swapped velocities on y axis" << "\n";*/
	}
	else if (xAxisChange == true && yAxisChange == true) {
		float tempVelX = a.vel.vx;
		float tempVelY = a.vel.vy;
		a.vel.vx = b.vel.vx;
		a.vel.vy = b.vel.vy;
		b.vel.vx = tempVelX;
		b.vel.vy = tempVelY;

		/*std::cout << a.name << " and " << b.name << " have swapped velocities on both axis" << "\n";*/
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
	bool verticalTouch = std::abs(squareA.edges.bottom - squareB.edges.top) < 0.01f;
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

							keepInBounds(a, true);
							keepInBounds(b, true);

						}
						else { // if a is to the right of b
							// adjust thier x positions wiht correction
							a.pos.x += correction;
							b.pos.x -= correction;

							keepInBounds(a, true);
							keepInBounds(b, true);
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

							keepInBounds(a, false);
							keepInBounds(b, false);

						}
						else {
							a.pos.y += correction;
							b.pos.y -= correction;

							keepInBounds(a, false);
							keepInBounds(b, false);
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

void updateGame(std::vector<Square>& squares, float frameDt) {
	
		squaresInMotion(squares, frameDt);
		collisionDetectionSweepAndPrune(squares);

}