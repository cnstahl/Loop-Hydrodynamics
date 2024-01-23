
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <array>
#include <list>
#include <functional>
#include <utility>

typedef unsigned long ulong;

const int length = 4;
const int width  = length;    // System width 
const int height = length;    // System height
const int m = 3;           // number of colors
int step  = 0;             // used for filenames
int steps = 2000;            // simulation time
int run = 0; 
int runs = 1;
int steps_per_print = 150;
std::string type = "data";

// declare random number generator outside of main
std::random_device rd;  // Used to obtain a seed for the random number engine
auto seed = rd();
std::mt19937 gen(seed); // Standard mersenne_twister_engine seeded with rd()
std::uniform_int_distribution<int> rand_color(0,m-1);
std::uniform_int_distribution<int> rand_diff(1 , m-1);
std::uniform_int_distribution<int> rand_row(0,height-1);
std::uniform_int_distribution<int> rand_row2(0,2*height-1);
std::uniform_int_distribution<int> rand_col(0,width-1);
std::uniform_int_distribution<int> rand_dir(0,3); // 0 is E, 1 is N...
std::uniform_int_distribution<int> rand_int2(0,1);
std::uniform_int_distribution<int> rand_int3(0,2);

//***************************************************************************//
//*** Timing info                                                         ***//
//***************************************************************************//

// auto start = std::chrono::steady_clock::now();
// auto end = std::chrono::steady_clock::now();
// std::chrono::duration<double> diff = end - start;

//***************************************************************************//
//*** Functions for entire state                                          ***//
//***************************************************************************//

std::array<int, 2*height*width> underlying_spins;

int index_from_coords(int row, int column) {
	row = (row + 2*height) % (2*height);
	column = (column + width) % width;
	return row*width + column;
}

void reset() {
	underlying_spins = {};
}

void set_all(int background) {
	for (auto &v : underlying_spins) v = background;
}

int get_spin(int row, int column) {
	// return the state of this spin
	return underlying_spins[index_from_coords(row, column)];
}

void set_spin(int row, int column, int color) {
	// flip to a different color 
	underlying_spins[index_from_coords(row, column)] = color;
}

int get_spin_from_dir(int row, int column, int dir) { // 0 is E, 1 is N, 2 is W, 3 is S
	switch(dir) {
		case 0: return get_spin(2*row+1, column+1);
		case 1: return get_spin(2*row  , column);
		case 2: return get_spin(2*row+1, column);
		case 3: return get_spin(2*row+2, column);
	}
	exit(EXIT_FAILURE);
}

void set_spin_from_dir(int row, int col, int dir, int color) {
	switch(dir) {
		case 0: set_spin(2*row+1, col+1, color); return;
		case 1: set_spin(2*row  , col  , color); return;
		case 2: set_spin(2*row+1, col  , color); return;
		case 3: set_spin(2*row+2, col  , color); return;
	}
}

std::pair<int, int> get_face_from_dir(int row, int col, int dir) {
	switch(dir) {
		case 0: return std::make_pair((row+height)%height, (col+width+1)%width);
		case 1: return std::make_pair((row+height-1)%height, (col+width)%width);
		case 2: return std::make_pair((row+height)%height, (col+width-1)%width);
		case 3: return std::make_pair((row+height+1)%height, (col+width)%width);
	}
	exit(EXIT_FAILURE);
}

std::pair<int, int> get_edge_from_dir(int row, int col, int dir) {
	switch(dir) {
		case 0: return std::make_pair(2*row+1, col+1);
		case 1: return std::make_pair(2*row  , col  );
		case 2: return std::make_pair(2*row+1, col  );
		case 3: return std::make_pair(2*row+2, col  );
	}
	exit(EXIT_FAILURE);
}

bool is_flippable(int row, int col) {
	int old_color = get_spin(2*row-1, col);
	return (get_spin(2*row, col-1) == old_color &&
			get_spin(2*row, col  ) == old_color &&
			get_spin(2*row+1, col) == old_color);
}

//***************************************************************************//
//*** Functions about Krylov labels                                       ***//
//***************************************************************************//

#include "krylovs.h"

//***************************************************************************//
//*** Local and Nonlocal updates                                          ***//
//***************************************************************************//

void update() {
	// choose a vertex
	int row = rand_row(gen);
	int col = rand_col(gen);

	int old_color = get_spin(2*row-1, col);

	if (get_spin(2*row, col-1) == old_color &&
		get_spin(2*row, col  ) == old_color &&
		get_spin(2*row+1, col) == old_color) {

		int new_color = (old_color + rand_diff(gen)) % m;
		set_spin(2*row-1, col, new_color);
		set_spin(2*row, col-1, new_color);
		set_spin(2*row, col  , new_color);
		set_spin(2*row+1, col, new_color);
	}
}

void time_step(int time) {
	for (int i = 0; i < height*width*time; i++) update();
}

#include "big_flip.h"

void big_time_step(int time) {
	for (int i = 0; i < time; i++) big_update();
}

void print_state() {
	for (int row = 0; row < 2*height; row++) {
		for (int col = 0; col < width; col++) {
			std::cout << " · " << get_spin(row, col);
		}
		std::cout << " · " << "\n ";
		row++;
		for (int col = 0; col < width; col++) {
			std::cout << get_spin(row, col) << "   ";
		}
		std::cout << get_spin(row, 0) << "\n";
	}
	for (int col = 0; col < width; col++) {
		std::cout << " · " << get_spin(0, col);
	}
	std::cout << " · " << "\n";
}

//***************************************************************************//
//*** Observables	                                                      ***//
//***************************************************************************//

int total_s() {
	int total = 0;
	for (int row = 0; row < 2*height; row++) {
		for (int col = 0; col < width; col++) {
			total += get_spin(row, col);
		}
	}
	return total;
}

int count_flippable() {
	int total = 0;
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			total += is_flippable(row, col);
		}
	}
	return total;
}

int is_trivial() {
	return ((vert_label_length() == 0) and (horz_label_length() == 0));
}

//***************************************************************************//
//*** Print and main                                                      ***//
//***************************************************************************//

// std::function<void(int time)> update_func = &time_step;
std::function<void(int time)> update_func = &big_time_step;

// std::function<int()> func = &total_s;
// std::string name = "total_s";

std::function<int()> func = &is_trivial;
std::string name = "is_trivial";

// std::function<int()> func = &count_flippable;
// std::string name = "flippable";

void print_data() {
	std::ofstream myfile;
	myfile.open (type + "/" + name + std::to_string(width) + "by" + 
				 std::to_string(height) + ".dat");

	myfile << "Sum of all the s values of a" + std::to_string(width) + "by"
				+ std::to_string(height) + "system. Rows are runs\n";

	for (run = 0; run < runs; run++) {
		reset();
		myfile << func();
		for (step = 0; step < steps; step++) {
			update_func(steps_per_print);
			myfile << ", " << func();
		}
		myfile << "\n";
	}
}

int main(int argc, char *argv[]) {
	print_state();
	// std::cout << std::to_string(seed);

	// print_data();
	// print_N_trivial();

	std::cout << "\n\n\n";
	big_time_step(1);
	std::cout << vert_label_length() << " and " << horz_label_length() << "\n";
	// std::cout << is_trivial() << "\n";
	print_state();
}


