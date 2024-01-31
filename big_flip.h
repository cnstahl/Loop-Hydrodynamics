std::array<int, 2*height*width> already_visited;
std::vector<std::pair<int, int>> edges_to_flip;

int next_dir (int row, int col, int dir, int a_color, int b_color) {
	std::vector<int> next_dirs = {};
	std::pair<int, int> next_edge;
	// look at the three edges other than the previous one
	// at this step, we want b_color on vertical or a_color on hoizontal
	for (int next_dir = 0; next_dir < 4; next_dir++) {
		if ((4+next_dir-dir) % 4 == 2) continue; // don't double back

		next_edge = get_edge_from_dir(row, col, next_dir);
		if (already_visited[index_from_coords(next_edge.first, next_edge.second)] == 1) continue;
		if (next_dir%2) { // this means next_dir = 1 or 3, so we're on a horizontal edge
			if (get_spin_from_dir(row, col, next_dir) == a_color) next_dirs.push_back(next_dir); 
		}
		else {
			if (get_spin_from_dir(row, col, next_dir) == b_color) next_dirs.push_back(next_dir); 
		}
	}
	// std::cout << "next_dirs has " << std::to_string(next_dirs.size()) << ": ";
	// for (int i = 0; i < next_dirs.size(); i++) {
	// 	std::cout << std::to_string(next_dirs[i]) << ", ";
	// }
	// std::cout << "\n";

	// choose an item from the array randomly
	switch(next_dirs.size()) {
		case 1: return next_dirs[0];
		case 2: return next_dirs[rand_int2(gen)];
		case 3: return next_dirs[rand_int3(gen)];
		case 4: return next_dirs[rand_int4(gen)];
	}
	exit(EXIT_FAILURE);
}

void find_big_path(int start_row, int start_col, int dir, int a_color, int b_color) {
	std::pair<int, int> next_edge;
	int row = start_row;
	int col = start_col;

	while (true) {
		// look for b_color on vertical or a_color on horizontal
		next_edge = get_edge_from_dir(row, col, dir);
		edges_to_flip.push_back(next_edge);
		already_visited[index_from_coords(next_edge.first, next_edge.second)] = 1;
		std::tie(row, col) = get_face_from_dir(row, col, dir);
		// std::cout << ": dir = " << std::to_string(dir) << "\n";
		// std::cout << std::to_string(row) << ", " << std::to_string(col) << "\n";	
		dir = next_dir(row, col, dir, a_color, b_color);

		// look for a_color on vertical or b_color on horizontal
		next_edge = get_edge_from_dir(row, col, dir);
		edges_to_flip.push_back(next_edge);
		already_visited[index_from_coords(next_edge.first, next_edge.second)] = 1;
		std::tie(row, col) = get_face_from_dir(row, col, dir);
		// std::cout << ": dir = " << std::to_string(dir) << "\n";
		// std::cout << std::to_string(row) << ", " << std::to_string(col) << "\n";

		if ((row == start_row) and (col == start_col)) return; // we did it!
		dir = next_dir(row, col, dir, b_color, a_color);

	}
}

void big_update() {
	int start_row, start_col, dir, a_color, b_color;
	std::pair<int, int> next_edge;
	already_visited = {};
	edges_to_flip.clear();

	// choose a starting face and direction
	start_row = rand_row(gen);
	start_col = rand_col(gen);
	dir = rand_dir(gen);
	// std::cout << std::to_string(start_row) << ", " << std::to_string(start_col) << "\n";

	// get the colors a_color (alpha) and b_color (beta)
	a_color = get_spin_from_dir(start_row, start_col, dir);
	if (dir%2 == 1) { // this means dir = 1 or 3, so we're on a horizontal edge and need to switch
		b_color = a_color;
		a_color = (b_color + rand_diff(gen)) % m;
	}
	else b_color = (a_color + rand_diff(gen)) % m;

	// construct the big loop, flipping spins later
	find_big_path(start_row, start_col, dir, a_color, b_color);

	// flip all the marked spins 
	int num_edges_to_flip = edges_to_flip.size();
	for (int i=0; i < num_edges_to_flip; i++) {
		auto edge = edges_to_flip[i];
		set_spin(edge.first, edge.second, ((edge.first + i) % 2) ? b_color : a_color); // takes care of double odd/even effect
	}
}

void symmetry_flip() {
	// choose a first spin and get a_color


	// choose a second spin on the same sublattice and get b_color

	// big flip for a

	// big flip for b

	// check that an even number of spins have been flipped in the label (accept or reject)

	// if accept, do the flip
}


