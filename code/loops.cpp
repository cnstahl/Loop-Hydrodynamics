
#include <cassert>
#include <fstream>
#include <cmath>
#include <random>
#include <array>
#include <cmath>
#include <algorithm>
#include <deque>
#include <map>
#include <iostream>

// #include <chrono>
// #include <cstddef>
// #include <iomanip>
// #include <numeric>
// #include <vector>

typedef unsigned long ulong;
typedef unsigned int  uint;

const int height  = 4;    // System height
const int width   = 5;    // System width THIS SHOULD BE THE BIGGER NUMBER
const int m = 3;           // number of colors
const int background = 0;  // color of background (left/right boundary) spins
int step  = 0;             // used for filenames
int steps = 1000000;            // simulation time

// std::deque<ulong> deque_of_states = {0};

// declare random number generator outside of main
std::random_device rd;  // Used to obtain a seed for the random number engine
std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
std::uniform_int_distribution<int> rand_color(0,m-1);
std::uniform_int_distribution<int> rand_diff(1 , m-1);
std::uniform_int_distribution<int> rand_row(0,height-1);
std::uniform_int_distribution<int> rand_col(0,width-1);

//***************************************************************************//
//*** Timing info                                                         ***//
//***************************************************************************//

auto start = std::chrono::steady_clock::now();
auto end = std::chrono::steady_clock::now();
std::chrono::duration<double> diff = end - start;
		
//***************************************************************************//
//*** Object for checking if two rows are compatible                      ***//
//***************************************************************************//

class Row {
public:
	std::array<int, width> underlying_row;

public:
	Row(uint row_as_int) {
		set_row_from_int(row_as_int);
	}

	int get_spin(int column) {
		return underlying_row[column];
	}

	void set_spin(int column, int color) {
		underlying_row[column] = color;
	}

	uint row_to_int() {
		uint row_as_int = 0;

		for (int column = 0; column < width; column++) {
			row_as_int *=3;
			row_as_int += get_spin(column);
		}
		return row_as_int;
	}

	void set_row_from_int(uint row_as_int) {
		for (int column = width-1; column >=0; column--) {
			set_spin(column, row_as_int % 3); 
			row_as_int /= 3;
		}
	}

};
		
//***************************************************************************//
//*** Object for entire state                                             ***//
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
	bool row_is_valid(int row);
	bool state_is_valid();
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

bool State::row_is_valid(int row) {
	for (int column = 0; column < width; column++) {
		if (!is_valid(row, column)) {return false;}
	}
	return true;
}

bool State::state_is_valid() {
	for (int row = 0; row < height; row++) {
		for (int column = 0; column < width; column++) {
			if (!is_valid(row, column)) {return false;}
		}
	}
	return true;
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
	// std::assert(true); //fix this

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
	std::cout << ")";

}

//***************************************************************************//
//*** Enumeration class                                                   ***//
//***************************************************************************//

class State_Enum {
private:

public:
	std::vector<ulong> vector_of_states;          // List of all the states
	std::vector<uint>  vector_of_sym_sectors;     // sym sector of every state      
	std::vector<uint>  vector_of_Krylovs;         // Krylov sector of every state
	std::vector<uint>  short_vector_of_syms;      // dense vector of sym labels
	std::vector<uint>  short_vector_of_Krylovs;   // dense vector of Krylov labels
	std::vector<uint>  symmetry_sector_sizes;     // size of each symmetry sector
	std::vector<uint>  Krylov_sector_sizes;       // size of each Krylov sector
	std::vector<uint>  Krylov_to_sym;

	// given a row, which next rows are valid?
	std::vector<std::deque<ulong>> valid_next_rows;
	std::vector<std::vector<bool>> transfer_matrix;

	std::map<ulong, ulong> map_state_to_index;
	std::map<uint, uint>   map_Krylov_to_index;
	std::map<uint, uint>   map_sym_to_index;
	std::map<uint, std::vector<ulong>> map_sym_to_states;
	std::map<uint, std::vector<ulong>> map_Krylov_to_states;
	std::map<uint, std::vector<uint>>  map_sym_to_Krylovs;

	int num_states;
	int num_sym_sectors;
	int num_Krylov_sectors;

	void populate_next_rows() {
		ulong max_row_value = 1;
		ulong this_row_value;
		ulong big_row_value;
		ulong next_row_value;
		State state;
		std::deque<ulong> valid_next_rows_here;

		std::cout << "Starting Next Rows: ";
		end = std::chrono::steady_clock::now();
		diff = end - start;
		std::cout << diff.count() << "\n";

		for (int column = 0; column < width; column++) {
			max_row_value *= m;
		}

		valid_next_rows.reserve(max_row_value);

		for (this_row_value = 0; this_row_value < max_row_value; this_row_value++) {
			valid_next_rows_here = {};
			big_row_value = this_row_value*max_row_value;
			for (next_row_value = 0; next_row_value < max_row_value; next_row_value++) {
				state.set_state_from_int(big_row_value + next_row_value);
				if (state.row_is_valid(height-1)) {
					valid_next_rows_here.push_back(next_row_value);
				}
			}
			valid_next_rows.push_back(valid_next_rows_here);
		}

		std::cout << "Finished Next Rows: ";
		end = std::chrono::steady_clock::now();
		diff = end - start;
		std::cout << diff.count() << "\n";

	}

	void transfer_populate() {
		std::deque<ulong> old_deque_of_states = {};
		std::deque<ulong> new_deque_of_states = {};
		ulong state_as_int;
		ulong prev_state_value;
		ulong prev_row_value;
		std::vector<ulong> valid_next_rows_here;
		ulong max_row_value = 1;
		State state;

		populate_next_rows();

		std::cout << "Starting Populate: ";
		end = std::chrono::steady_clock::now();
		diff = end - start;
		std::cout << diff.count() << "\n";

		for (int column = 0; column < width; column++) {
			max_row_value *= m;
		}

		// Set the first row with impunity
		for (ulong this_row_value = 0; this_row_value < max_row_value; this_row_value++) {
			old_deque_of_states.push_front(this_row_value);
		}

		// std::cout << old_deque_of_states.size() << "\n";

		for (int row = 1; row < height; row++) {
			while (!old_deque_of_states.empty()) {
				prev_state_value = old_deque_of_states.back();
				prev_row_value = prev_state_value % max_row_value;
				prev_state_value *= max_row_value;

				for (ulong this_row_value : valid_next_rows[prev_row_value]) {
					new_deque_of_states.push_front(prev_state_value + this_row_value);
				}
				old_deque_of_states.pop_back();
			}
			old_deque_of_states = new_deque_of_states;
			new_deque_of_states = {};
			// std::cout << old_deque_of_states.size() << "\n";
		}

		// Still need to check the last row of plaquettes is valid
		int row = 0;
		while (!old_deque_of_states.empty()) {
			state_as_int = old_deque_of_states.back();
			state.set_state_from_int(state_as_int);
			if (state.row_is_valid(row)) {
				new_deque_of_states.push_front(state_as_int);
			}
			old_deque_of_states.pop_back();
		}
		old_deque_of_states = new_deque_of_states;
		new_deque_of_states = {};

		num_states = old_deque_of_states.size();
		vector_of_states = std::vector<ulong> (num_states);

		std::cout << "Starting Last Check: ";
		end = std::chrono::steady_clock::now();
		diff = end - start;
		std::cout << diff.count() << "\n";


		for (int i = 0; i < num_states; i++) {
			state_as_int = old_deque_of_states.front();
			vector_of_states[i] = state_as_int;
			map_state_to_index[state_as_int] = i;
			old_deque_of_states.pop_front();
			// std::cout << "\n" << state_as_int << "   " << map_state_to_index[state_as_int] << "\n";
		}

		std::cout << "Finished Populate: ";
		end = std::chrono::steady_clock::now();
		diff = end - start;
		std::cout << diff.count() << "\n";

	}

	// void better_populate() {
	// 	std::deque<ulong> old_deque_of_states = {};
	// 	std::deque<ulong> new_deque_of_states = {};
	// 	State state;
	// 	int row;
	// 	int column;
	// 	ulong row_value;
	// 	ulong max_row_value = 1;
	// 	ulong state_as_int;
	// 	bool state_is_good;

	// 	for (column = 0; column < width; column++) {
	// 		max_row_value *= m;
	// 	}

	// 	// Set the first row with impunity
	// 	for (row_value = 0; row_value < max_row_value; row_value++) {
	// 		old_deque_of_states.push_front(row_value);
	// 	}

	// 	// std::cout << old_deque_of_states.size() << "\n";

	// 	// For each remaining row
	// 	for (row = 1; row < height; row++) {
	// 		while (!old_deque_of_states.empty()) {
	// 			state_as_int = old_deque_of_states.back() * max_row_value;
	// 			for (row_value = 0; row_value < max_row_value; row_value++) {
	// 				state.set_state_from_int(state_as_int + row_value);
	// 				if (state.row_is_valid(height-1)) {
	// 					new_deque_of_states.push_front(state_as_int + row_value);	
	// 				}
	// 			}
	// 			old_deque_of_states.pop_back();
	// 		}
	// 		old_deque_of_states = new_deque_of_states;
	// 		new_deque_of_states = {};
	// 		// std::cout << old_deque_of_states.size() << "\n";
	// 	}

	// 	// Still need to check the last row of plaquettes is valid
	// 	row = 0;
	// 	while (!old_deque_of_states.empty()) {
	// 		state_as_int = old_deque_of_states.back();
	// 		state.set_state_from_int(state_as_int);
	// 		if (state.row_is_valid(row)) {
	// 			new_deque_of_states.push_front(state_as_int);
	// 		}
	// 		old_deque_of_states.pop_back();
	// 	}
	// 	old_deque_of_states = new_deque_of_states;
	// 	new_deque_of_states = {};

	// 	num_states = old_deque_of_states.size();
	// 	vector_of_states = std::vector<ulong> (num_states);


	// 	for (int i = 0; i < num_states; i++) {
	// 		state_as_int = old_deque_of_states.front();
	// 		vector_of_states[i] = state_as_int;
	// 		map_state_to_index[state_as_int] = i;
	// 		old_deque_of_states.pop_front();
	// 		// std::cout << "\n" << state_as_int << "   " << map_state_to_index[state_as_int] << "\n";
	// 	}
	// }

	// void populate_vector_of_states() {
	// 	std::deque<ulong> old_deque_of_states = {0};
	// 	std::deque<ulong> new_deque_of_states = {};
	// 	State state;
	// 	int row = 0;
	// 	int column = 0;

	// 	// Set the first row with impunity
	// 	for (column = 0; column < width; column++) {
	// 		while (!old_deque_of_states.empty()) {
	// 			state.set_state_from_int(old_deque_of_states.back());

	// 			// Set the spin to each color
	// 			for (int color = 0; color < m; color ++) {
	// 				state.set_spin(row, column, color);
	// 				new_deque_of_states.push_front(state.state_to_int());
	// 			}
	// 			// Remove a state after looking at it
	// 			old_deque_of_states.pop_back();
	// 		}
	// 		old_deque_of_states = new_deque_of_states;
	// 		new_deque_of_states = {};
	// 	}

	// 	// std::cout << old_deque_of_states.size() << "\n";

	// 	// For each remaining row
	// 	for (row = 1; row < height; row++) {
	// 		// Set the first column with impunity
	// 		column = 0;
	// 		while (!old_deque_of_states.empty()) {
	// 			state.set_state_from_int(old_deque_of_states.back());

	// 			// Set the spin to each color
	// 			for (int color = 0; color < m; color ++) {
	// 				state.set_spin(row, column, color);
	// 				new_deque_of_states.push_front(state.state_to_int());
	// 			}
	// 			// Remove a state after looking at it
	// 			old_deque_of_states.pop_back();
	// 		}
	// 		old_deque_of_states = new_deque_of_states;
	// 		new_deque_of_states = {};

	// 		// For each remaining site, we have to actually check if the spin is valid
	// 		for (int column = 1; column < width; column++) {

	// 			// Pull states from the queue one at a time
	// 			while (!old_deque_of_states.empty()) {
	// 				state.set_state_from_int(old_deque_of_states.back());

	// 				// Try setting the spin to each color
	// 				for (int color = 0; color < m; color ++) {
	// 					state.set_spin(row, column, color);
	// 					if (state.is_valid(row, column)) {

	// 						// Add valid states to a new queue
	// 						new_deque_of_states.push_front(state.state_to_int());
	// 					}
	// 				}
	// 				// Remove a state after looking at it
	// 				old_deque_of_states.pop_back();
	// 			}
	// 			old_deque_of_states = new_deque_of_states;
	// 			new_deque_of_states = {};
	// 		}
	// 		// Still need to check that the last plaquette is valid
	// 		while (!old_deque_of_states.empty()) {
	// 			state.set_state_from_int(old_deque_of_states.back());
	// 			if (state.is_valid(row, 0)) {
	// 				new_deque_of_states.push_front(state.state_to_int());
	// 			}
	// 			old_deque_of_states.pop_back();
	// 		}
	// 		old_deque_of_states = new_deque_of_states;
	// 		new_deque_of_states = {};
	// 		// std::cout << old_deque_of_states.size() << "\n";
	// 	}

	// 	// Still need to check the last row of plaquettes is valid
	// 	row = 0;
	// 	for (column = 0; column < width; column++) {
	// 		while (!old_deque_of_states.empty()) {
	// 			state.set_state_from_int(old_deque_of_states.back());

	// 			// Set the spin to each color
	// 			if (state.is_valid(row, column)) {
	// 				new_deque_of_states.push_front(state.state_to_int());
	// 			}
	// 			// Remove a state after looking at it
	// 			old_deque_of_states.pop_back();
	// 		}
	// 		old_deque_of_states = new_deque_of_states;
	// 		new_deque_of_states = {};
	// 	}

	// 	num_states = old_deque_of_states.size();
	// 	vector_of_states = std::vector<ulong> (num_states);


	// 	ulong state_as_int;
	// 	for (int i = 0; i < num_states; i++) {
	// 		state_as_int = old_deque_of_states.front();
	// 		vector_of_states[i] = state_as_int;
	// 		map_state_to_index[state_as_int] = i;
	// 		old_deque_of_states.pop_front();
	// 		// std::cout << "\n" << state_as_int << "   " << map_state_to_index[state_as_int] << "\n";
	// 	}
	// }

	// populate vector of symmetry sectors corresponding to each state
	void populate_vector_of_sym_sectors() {
		State state;
		vector_of_sym_sectors = std::vector<uint> (num_states);

		int i = 0;
		for (ulong state_as_int : vector_of_states) {
			state.set_state_from_int(state_as_int);
			vector_of_sym_sectors[i] = state.get_sym_sector();
			i++;
		}

		// deque_of_sym_sectors;
	}

	// Populate the vector of (meaningless) Krylov sector labels
	void populate_vector_of_Krylovs() {
		std::vector<bool> is_visited(num_states, false);
		std::deque<ulong> deque_of_states = {};
		uint Krylov_label = 0;
		State state;
		int old_color;
		int index;

		vector_of_Krylovs = std::vector<uint> (num_states);

		// Iterate through all valid states
		for (ulong state_as_int : vector_of_states) {
			index = map_state_to_index[state_as_int];
			// std::cout << "\n" << state_as_int << "   " << index << "\n";

			// If we've already seen this state, ignore it
			if (is_visited[index]) {continue;}

			// Otherwise, mark it as seen and apply a Krylov label
			is_visited[index] = true;
			vector_of_Krylovs[index] = Krylov_label;

			// Build a new deque for this Krylov sector
			deque_of_states = {state_as_int};

			// Iterate through the deque
			while (!deque_of_states.empty()) {
				state.set_state_from_int(deque_of_states.front());
				for (int column = 0; column < width; column++) {
					for (int row = 0; row < height; row ++) {
						if (state.is_flippable(row, column)) {

							// Don't forget what color this was originally!
							old_color = state.get_spin(row, column);

							// Flip to other colors
							for (int color = 1; color < m; color++) {
								state.set_spin(row, column, (old_color + color) % m);
								state_as_int = state.state_to_int();
								index = map_state_to_index[state_as_int];
								if (!is_visited[index]) {
									// std::cout << state_as_int << "   " << index << "\n";
									is_visited[index] = true;
									vector_of_Krylovs[index] = Krylov_label;
									deque_of_states.push_back(state_as_int);
								}
							}
							state.set_spin(row, column, old_color);
						}
					}
				}
				deque_of_states.pop_front();
			}

			// Time for a new Krylov label
			Krylov_label++;
		}
	}

	void finish_enumeration() {
		ulong state_as_int;
		int i;
		uint sym_sector;

		// Loop through list of states and populate sym and Krylov maps
		for (i = 0; i < num_states; i++) {
			state_as_int = vector_of_states[i];
			map_sym_to_states[vector_of_sym_sectors[i]].push_back(state_as_int);
			map_Krylov_to_states[vector_of_Krylovs[i]].push_back(state_as_int);
		}

		num_sym_sectors = map_sym_to_states.size();
		short_vector_of_syms  = std::vector<uint> (num_sym_sectors);
		symmetry_sector_sizes = std::vector<uint> (num_sym_sectors);
		num_Krylov_sectors = map_Krylov_to_states.size();
		short_vector_of_Krylovs = std::vector<uint> (num_Krylov_sectors);
		Krylov_sector_sizes     = std::vector<uint> (num_Krylov_sectors);

		i = 0;
		// For each symmetry sector
		for (auto sym_to_states : map_sym_to_states) {
			short_vector_of_syms[i] = sym_to_states.first;
			symmetry_sector_sizes[i] = sym_to_states.second.size();
			map_sym_to_index[sym_to_states.first] = i;
			i++;
		}

		i = 0;
		// For each Krylov sector
		for (auto Krylov_to_states : map_Krylov_to_states) {
			short_vector_of_Krylovs[i] = Krylov_to_states.first;
			Krylov_sector_sizes[i] = Krylov_to_states.second.size();
			map_Krylov_to_index[Krylov_to_states.first] = i;

			state_as_int = Krylov_to_states.second.front();
			sym_sector = vector_of_sym_sectors[map_state_to_index[state_as_int]];
			map_sym_to_Krylovs[sym_sector].push_back(Krylov_to_states.first);
			i++;
		}
	}

};

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
	State_Enum state_enum;

	// state_enum.populate_vector_of_states();
	state_enum.transfer_populate();

	std::cout << "Finished population\n";
	std::cout << height << " " << width << ", " << state_enum.num_states << "\n";
	// state_enum.populate_vector_of_sym_sectors();
	// std::cout << ", " << state_enum.vector_of_sym_sectors.size() << "\n\n";
	// state_enum.populate_vector_of_Krylovs();
	// std::cout << "index, symmetry, Krylov label\n";
	// for (int index = 0; index < state_enum.num_states; index++) {
			
	// 	if (state_enum.vector_of_sym_sectors[index] == 2119) {
	// 		std::cout << index << ", " << state_enum.vector_of_sym_sectors[index];
	// 		std::cout << ", " << state_enum.vector_of_Krylovs[index] << "\n";
	// 		state.set_state_from_int(state_enum.vector_of_states[index]);
	// 		state.print_state();
	// 		print_sym_values(state_enum.vector_of_sym_sectors[index]);
	// 		std::cout << "\n\n";
	// 	}
	// }
	// state_enum.finish_enumeration();

	// int max_Krylov;
	// int this_Krylov;
	// for (int i = 0; i < state_enum.num_sym_sectors; i++) {
	// 	std::cout << "Size of symmetry sector: ";
	// 	std::cout << state_enum.symmetry_sector_sizes[i] << "; ";
	// 	max_Krylov = 0;
	// 	for (auto Krylov : state_enum.map_sym_to_Krylovs[state_enum.short_vector_of_syms[i]]) {
	// 		this_Krylov = state_enum.map_Krylov_to_states[Krylov].size();
	// 		if (this_Krylov > max_Krylov) {
	// 			max_Krylov = this_Krylov;
	// 		}
	// 		// std::cout << this_Krylov;
	// 	} 
	// 	std::cout << "Size of largest Krylov sector: ";
	// 	std::cout << max_Krylov << "\n";
	// }


	// for (int i = 0; i < state_enum.num_states; i++) {
	// 	state.set_state_from_int(state_enum.vector_of_states[i]);
	// 	state.print_state();
	// 	// std::cout << deque_of_sym_sectors.back();
	// 	// print_sym_values(state_enum.vector_of_sym_sectors[i]);
	// 	std::cout << "\n";
	// }
}





