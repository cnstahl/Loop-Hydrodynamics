#include <iostream>
#include <cassert>
#include <fstream>
#include <cmath>
#include <random>
#include <array>
#include <cmath>
#include <algorithm>
#include <list>

typedef unsigned long ulong;
typedef unsigned int  uint;

const int height  = 3;    // System height
const int width   = 3;    // System width
const int m = 3;           // number of colors
const int background = 0;  // color of background (left/right boundary) spins
int step  = 0;             // used for filenames
int steps = 1000000;            // simulation time

// std::list<ulong> list_of_states = {0};

// declare random number generator outside of main
std::random_device rd;  // Used to obtain a seed for the random number engine
std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
std::uniform_int_distribution<int> rand_color(0,m-1);
std::uniform_int_distribution<int> rand_diff(1 , m-1);
std::uniform_int_distribution<int> rand_row(0,height-1);
std::uniform_int_distribution<int> rand_col(0,width-1);

//***************************************************************************//
//*** Lookup table for number of states                                   ***//
//***************************************************************************//

// int lookup_num_states[]

//***************************************************************************//
//*** Object orientation!!!     Woohoo!!                                  ***//
//***************************************************************************//

class State {
private:
	std::array<int, height*width> underlying_state;

	int index_from_coords(int row, int column) {
		row = (row + height) % height;
		column = (column + width) % width;
		return row*width + column;
	}

public:
	State() {
		underlying_state = {{background}};
	}

	State(ulong state_as_int) {
		set_state_from_int(state_as_int);
	}

	void reset() {
		underlying_state = {{background}};
	}

	int get_spin(int row, int column) {
		// return the state of this spin
		return underlying_state[index_from_coords(row, column)];
	}

	void set_spin(int row, int column, int color) {
		// flip to a different color (not the same)
		// std::cout << "flipping to " + std::to_string(color) + "\n";
		underlying_state[index_from_coords(row, column)] = color;
		// std::cout << "flipped  to " + std::to_string(get_spin(row,column)) + "\n";
	}

	void change_spin(int row, int column) {
		underlying_state[index_from_coords(row, column)] = 
		(underlying_state[index_from_coords(row, column)] + rand_diff(gen)) % m;
	}

	ulong state_to_int() {
		ulong state_as_int = 0;

		for (int row = 0; row < height; row++) {
			for (int column = 0; column < width; column++) {
				state_as_int *=3;
				state_as_int += get_spin(row, column);
			}
		}
		return state_as_int;
	}

	void set_state_from_int(ulong state_as_int) {

		for (int row = height-1; row >=0; row--) {
			for (int column = width-1; column >=0; column--) {
				set_spin(row, column, state_as_int % 3); 
				state_as_int /= 3;
			}
		}
	}

	bool is_flippable(int row, int column);
	void update();
	void print_state();
	bool is_valid(int row, int column);
	unsigned int get_sym_sector();
};

//***************************************************************************//
//*** Functions to update and print the state                             ***//
//***************************************************************************//

bool State::is_valid(int row, int column) {
	// Check that the plaquette above and left of the given spin is valid
	return ((get_spin(row, column) == get_spin(row-1, column-1)) or 
			(get_spin(row-1, column) == get_spin(row, column-1)));
}

bool State::is_flippable(int row, int column) {
	int color_check = get_spin(row, column+1); // right spin
	return (get_spin(row+1, column) == color_check &&
			get_spin(row, column-1) == color_check && 
			get_spin(row-1, column) == color_check);
}

void State::update() {
	// choose a spin
	int row    = rand_row(gen);
	int column = rand_col(gen);

	if (is_flippable(row, column)) {
		change_spin(row, column);
	}
}

void State::print_state() {
	// std::ofstream myfile;
	// myfile.open("state");

	for (int row = 0; row <= height; row++) {
		for (int column = 0; column <= width; column++) {
			std::cout << std::to_string(get_spin(row, column));
		}
		std::cout << "\n";
	}

    // myfile.close();
}

//***************************************************************************//
//*** Functions about the state                                           ***//
//***************************************************************************//

// Return the 2m symmetry values packed into an unsigned int
// Symmetry numbers range [-(L/2), (L+1)/2] becomes [0, L] INCLUSIVE
// Fails if (L+1)^(2m) >= MaxInt (unsigned)
unsigned int State::get_sym_sector() {
	std::assert(true); //fix this

	unsigned int sym_sector = 0;
	int charge_density;    // edge degree of freedom
	int sym_value; 
	int sign;              // -1 for odd, +1 for even

	// Vertical labels
	for (int color = m-1; color >= 0; color--) {
		sym_sector *= (height+1);
		sym_value = height/2;

		for (int row = 0; row < height; row++) {
			sign = row%2 ? -1 : 1;
			charge_density = get_spin(row, 0) + get_spin(row-1, 0);
			charge_density = (m + charge_density) % m;
			if (charge_density == color) {
				sym_value += sign;
			}
		}
		sym_sector += sym_value;
	}

	// Horizontal labels
	for (int color = m-1; color >= 0; color--) {
		sym_sector *= (width+1);
		sym_value = width/2;

		for (int column = 0; column < width; column++) {
			sign = column%2 ? -1 : 1;
			charge_density = get_spin(0, column) + get_spin(0, column-1);
			charge_density = (m + charge_density) % m;
			if (charge_density == color) {
				sym_value += sign;
			}
		}
		sym_sector += sym_value;
	}
	return sym_sector;
}

void print_sym_values(unsigned int sym_sector) {
	int sym_value;

	std::cout << "Horizontal Labels: (";
	for (int color = 0; color < m; color++) {
		sym_value = (sym_sector % (width+1)) - width/2;
		std::cout << sym_value << ", ";
		sym_sector /= (width+1);
	}
	std::cout << ")\nVertical Labels:   (";
	for (int color = 0; color < m; color++) {
		sym_value = (sym_sector % (height+1)) - height/2;
		std::cout << sym_value << ", ";
		sym_sector /= (height+1);
	}
	std::cout << ")\n";

}

//***************************************************************************//
//*** Non-class functions                                                 ***//
//***************************************************************************//

// std::list<int> get_first_row() {
// 	std::list<int> l = {};
// 	int int_to_add = 0;

// 	for (int column = 0; column < width; column++) {
// 		l.push_front(int_to_add);
// 		int_to_add += 1;

// 		l.push_front(int_to_add);
// 		int_to_add += 1;

// 		l.push_front(int_to_add);
// 		int
// 	}

// 	return l;
// }

std::list<ulong> populate_list_of_states() {
	std::list<ulong> old_list_of_states = {0};
	std::list<ulong> new_list_of_states = {};
	State state;
	int row = 0;
	int column = 0;

	// Set the first row with impunity
	for (column = 0; column < width; column++) {
		while (!old_list_of_states.empty()) {
			state.set_state_from_int(old_list_of_states.back());

			// Set the spin to each color
			for (int color = 0; color < m; color ++) {
				state.set_spin(row, column, color);
				new_list_of_states.push_front(state.state_to_int());
			}
			// Remove a state after looking at it
			old_list_of_states.pop_back();
		}
		old_list_of_states = new_list_of_states;
		new_list_of_states = {};
	}

	// For each remaining row
	for (row = 1; row < height; row++) {
		// Set the first column with impunity
		column = 0;
		while (!old_list_of_states.empty()) {
			state.set_state_from_int(old_list_of_states.back());

			// Set the spin to each color
			for (int color = 0; color < m; color ++) {
				state.set_spin(row, column, color);
				new_list_of_states.push_front(state.state_to_int());
			}
			// Remove a state after looking at it
			old_list_of_states.pop_back();
		}
		old_list_of_states = new_list_of_states;
		new_list_of_states = {};

		// For each remaining site, we have to actually check if the spin is valid
		for (int column = 1; column < width; column++) {

			// Pull states from the queue one at a time
			while (!old_list_of_states.empty()) {
				state.set_state_from_int(old_list_of_states.back());

				// Try setting the spin to each color
				for (int color = 0; color < m; color ++) {
					state.set_spin(row, column, color);
					if (state.is_valid(row, column)) {

						// Add valid states to a new queue
						new_list_of_states.push_front(state.state_to_int());
					}
				}
				// Remove a state after looking at it
				old_list_of_states.pop_back();
			}
			old_list_of_states = new_list_of_states;
			new_list_of_states = {};
		}
		// Still need to check that the last plaquette is valid
		while (!old_list_of_states.empty()) {
			state.set_state_from_int(old_list_of_states.back());
			if (state.is_valid(row, 0)) {
				new_list_of_states.push_front(state.state_to_int());
			}
			old_list_of_states.pop_back();
		}
		old_list_of_states = new_list_of_states;
		new_list_of_states = {};
	}
	// Still need to check the last row of plaquettes is valid
	row = 0;
	for (column = 0; column < width; column++) {
		while (!old_list_of_states.empty()) {
			state.set_state_from_int(old_list_of_states.back());

			// Set the spin to each color
			if (state.is_valid(row, column)) {
				new_list_of_states.push_front(state.state_to_int());
			}
			// Remove a state after looking at it
			old_list_of_states.pop_back();
		}
		old_list_of_states = new_list_of_states;
		new_list_of_states = {};
	}

	return old_list_of_states;
}

// Return a list of symmetry sectors corresponding to each state
std::list<uint> populate_list_of_sym_sectors(std::list<ulong> list_of_states) {
	std::list<uint> list_of_sym_sectors;
	State state;

	for (ulong state_as_int : list_of_states) {
		state.set_state_from_int(state_as_int);
		list_of_sym_sectors.push_back(state.get_sym_sector());
	}

	return list_of_sym_sectors;
}

// Return a list of (meaningless) Krylov sector labels
std::list<uint> populate_list_of_Krylovs(std::list<ulong> list_of_states) {
	int num_states = list_of_states.size();
	bool visited[num_states] = [false];
}

//***************************************************************************//
//***    Main                                                             ***//
//***************************************************************************//

int main(int argc, char *argv[]) {
	// // Evolve state, print state, store as int, print again
	// State state;
	// for (step = 0; step < steps; step++) {
	// 	state.update();
	// }
	// state.print_state();
	// std::cout << "\n";
	// state.set_state_from_int(state.state_to_int());
	// state.print_state();

	State state;
	std::list<ulong> list_of_states = populate_list_of_states();
	std::list<uint> list_of_sym_sectors = populate_list_of_sym_sectors(list_of_states);
	std::cout << height << " " << width << " " << list_of_states.size();
	std::cout << ", " << list_of_sym_sectors.size() << "\n";

	while (!list_of_states.empty()) {
		state.set_state_from_int(list_of_states.back());
		state.print_state();
		// std::cout << list_of_sym_sectors.back();
		print_sym_values(list_of_sym_sectors.back());
		std::cout << "\n";
		list_of_states.pop_back();
		list_of_sym_sectors.pop_back();
	}
}





