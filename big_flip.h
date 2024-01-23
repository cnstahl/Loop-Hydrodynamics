
void big_update() {
	int row, col;
	std::pair<int, int> next_face;
	std::pair<int, int> next_edge;
	std::array<int, 2*height*width> already_visited = {};

	// choose a starting face and direction
	int start_row = rand_row(gen);
	int start_col = rand_col(gen);
	int dir = rand_dir(gen);
	// std::cout << std::to_string(start_row) << ", " << std::to_string(start_col) << "\n";

	// get the colors a_color (alpha) and b_color (beta)
	int a_color = get_spin_from_dir(start_row, start_col, dir);
	int b_color = (a_color + rand_diff(gen)) % m;
	if (dir%2 == 1) { // this means dir = 1 or 3, so we're on a horizontal edge and need to switch
		b_color = a_color;
		a_color = (b_color + rand_diff(gen)) % m;
	}

	// get to the next face
	set_spin_from_dir(start_row, start_col, dir, (dir%2 ? a_color : b_color));
	next_edge = get_edge_from_dir(start_row, start_col, dir);
	already_visited[index_from_coords(next_edge.first, next_edge.second)] = 1;
	next_face = get_face_from_dir(start_row, start_col, dir);
	row = next_face.first;
	col = next_face.second;
	// std::cout << ": dir = " << std::to_string(dir) << "\n";
	// std::cout << std::to_string(row) << ", " << std::to_string(col) << "\n";
	
	// construct the big loop, flipping spins along the way
	std::vector<int> next_dirs;
	while (true) {
		next_dirs = {};
		// look at the three edges other than the previous one
		// at this step, we want b_color on vertical or a_color on hoizontal
		for (int next_dir = 0; next_dir < 4; next_dir++) {
			if ((4+next_dir-dir) % 4 == 2) continue; // don't double back

			next_edge = get_edge_from_dir(row, col, next_dir);
			// if (next_dir == 2) std::cout << row << col << dir << "trying 2, color = " << get_spin_from_dir(row, col, dir) << "\n";
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
			case 1: 
				dir = next_dirs[0];
				break;
			case 2: 
				dir = next_dirs[rand_int2(gen)];
				break;
			case 3: 
				dir = next_dirs[rand_int3(gen)];
				break;
		}
		// if it was a horizontal edge, switch it to b_color
		set_spin_from_dir(row, col, dir, (dir%2 ? b_color : a_color));
		next_edge = get_edge_from_dir(row, col, dir);
		already_visited[index_from_coords(next_edge.first, next_edge.second)] = 1;
		next_face = get_face_from_dir(row, col, dir);
		row = next_face.first;
		col = next_face.second;
		// std::cout << ": dir = " << std::to_string(dir) << "\n";
		// std::cout << std::to_string(row) << ", " << std::to_string(col) << "\n";

		if ((row == start_row) and (col == start_col)) return; // we did it!

		// now do it again, but for the other color
		next_dirs = {};
		// look at the three edges other than the previous one
		// at this step, we want a_color on vertical or b_color on horizontal
		for (int next_dir = 0; next_dir < 4; next_dir++) {
			if ((4+next_dir-dir) % 4 == 2) continue; // don't double back

			next_edge = get_edge_from_dir(row, col, next_dir);
			if (already_visited[index_from_coords(next_edge.first, next_edge.second)] == 1) continue;

			if (next_dir%2) { // this means next_dir = 1 or 3, so we're on a vertical edge
				if (get_spin_from_dir(row, col, next_dir) == b_color) next_dirs.push_back(next_dir); 
			}
			else {
				if (get_spin_from_dir(row, col, next_dir) == a_color) next_dirs.push_back(next_dir); 
			}
		}
		// std::cout << "second option\n";
		// std::cout << "next_dirs has " << std::to_string(next_dirs.size()) << ": ";
		// for (int i = 0; i < next_dirs.size(); i++) {
		// 	std::cout << std::to_string(next_dirs[i]) << ", ";
		// }
		// std::cout << "\n";

		// choose an item from the array randomly
		switch(next_dirs.size()) {
			case 1: 
				dir = next_dirs[0];
				break;
			case 2: 
				dir = next_dirs[rand_int2(gen)];
				break;
			case 3: 
				dir = next_dirs[rand_int3(gen)];
				break;
		}
		// if it was a horizontal edge, switch it to a_color
		set_spin_from_dir(row, col, dir, (dir%2 ? a_color : b_color));
		next_edge = get_edge_from_dir(row, col, dir);
		already_visited[index_from_coords(next_edge.first, next_edge.second)] = 1;
		next_face = get_face_from_dir(row, col, dir);
		row = next_face.first;
		col = next_face.second;
		// std::cout << ": dir = " << std::to_string(dir) << "\n";
		// std::cout << std::to_string(row) << ", " << std::to_string(col) << "\n";

		if ((row == start_row) and (col == start_col)) exit (EXIT_FAILURE);
	}	
}
