#ifndef INVL_NEIGHBORSOLVER_H
#define INVL_NEIGHBORSOLVER_H

#include "tracker.h"

class neighborsolver {
	tracker::handle &h0, &h1;

	cube wrap(cube c, int m, int s) {
		return c.premove(move::inv(m)).move(m).sym(s);
	}

	void sanity_check_move_and_symmetry(int m, int s) {
		cube c0 = h0.ec(), c1 = h1.ec();
		if (wrap(c0, m, s).setCornerPerm(0) != c1) {
			abort();
		}
	}

    public:
	static void init();

	neighborsolver(tracker::handle &h0, tracker::handle &h1) : h0(h0), h1(h1) {
	}

	void solve(int m, int s);
};

#endif
