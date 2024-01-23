
//***************************************************************************//
//*** Functions about Krylov labels                                       ***//
//***************************************************************************//

// Turn a vertical or horizontal slice into a Krylov label
void decimate(std::list<int>& dots) {
	bool updated = true;
	std::list<int>::iterator myitr;

	// remove redundant dots
	while(!dots.empty() and updated) {
		// std::cout << "size: " << dots.size() << " ";
		updated = false;
		for (myitr = dots.begin(); myitr != std::prev(dots.end(), 1)
							   and myitr != dots.end(); ++myitr) {
			// std::cout << "working on " << *myitr << " ";
			if (*myitr == *(std::next(myitr, 1))) {
				dots.erase(myitr++); // Remove this one
				dots.erase(myitr++); // And then the next one
				updated = true; 
			}
		}
		if (!dots.empty() and (*(dots.begin()) == *(--dots.end()))) {
			// std::cout << "size: " << dots.size() << " ";
			dots.erase(dots.begin());
			*(--dots.end()) = *(dots.begin());
			dots.erase(dots.begin());
			updated = true;
		}
	}
}

ulong dots_to_int(std::list<int>& dots) {
	std::list<int>::iterator myitr;
	ulong krylov_label = 0;

	for (myitr = dots.begin(); myitr != dots.end(); ++myitr) {
		krylov_label *= m;
		krylov_label += *myitr;
	}
	return krylov_label;
}

ulong spin_dots(std::list<int>& dots) {
	ulong krylov_label = 0;
	ulong min_krylov_label = 0;

	min_krylov_label = dots_to_int(dots);

	for (int i = 0; i < dots.size()/2-1; i++) {
		dots.push_back(dots.front());
		dots.pop_front();
		dots.push_back(dots.front());
		dots.pop_front();
		krylov_label = dots_to_int(dots);

		if (krylov_label < min_krylov_label) min_krylov_label = krylov_label;
	}
	return min_krylov_label;
}

ulong label_to_int(std::list<int>& dots, int length) {
	if (dots.size() == 0) return 0;

	if (dots.size() < length) return spin_dots(dots);

	// store the remaining dots in an ulong
	return dots_to_int(dots);
}

int horz_label_length() {
	std::list<int> dots(underlying_spins.begin(), underlying_spins.begin() + width);
	decimate(dots);
	return dots.size();
} 

int vert_label_length() {
	std::list<int> dots;
	for (int row = 0; row < height; row++) dots.push_back(get_spin(2*row+1, 0));
	decimate(dots);
	return dots.size();
}
