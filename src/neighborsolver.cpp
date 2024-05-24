#include "neighborsolver.h"
#include "thread.h"
#include "tracker.h"
#include "involution.h"

void neighborsolver::init() {
	static std::once_flag flag;
	std::call_once(flag, [&]() {
		tracker::init();
		involution::init();
	});
}

void neighborsolver::solve(int m, int s) {
	sanity_check_move_and_symmetry(m, s);

	int mi = move::inv(m);
	int goal = h1.proven_min();

	for (moveseq sol : tracker::get_solutions(h0.idx)) {
		cube c = wrap(cube::from_moveseq(sol), m, s);
		ccoord cc = c;
		if (!h1.is_unsolved(cc)) {
			continue;
		}

		moveseq sol_m;
		sol_m.push_back(sym::move(mi, s));
		for (auto m : sol) {
			sol_m.push_back(sym::move(m, s));
		}
		sol_m.push_back(sym::move(m, s));
		sol_m = sol_m.canonical();

		if (sol_m.size() == goal) {
			h1.solution(sol_m);
		}
	}
}
