#include <set>
#include <bitset>
#include "ecsolver.h"
#include "ccoord.h"
#include "corner_hash.h"
#include "cprune.h"
#include "moveseq.h"
#include "bits.h"
#include "involution.h"
#include "thread.h"
#include "interrupt.h"

void ecsolver::init() {
	static std::once_flag flag;
	std::call_once(flag, [&]() {
		eprune::init();
		cprune::init();
		tracker::init();
		involution::init();
	});
}

class ecsolver_impl {
	struct seed_t {
		ecoord ec;
		ccoord cc;
		int last_axis;
		moveseq moves;
		int prune;
		eprune::rec_t r;

		seed_t(cube c, int last_axis, moveseq moves) : ec(c), cc(c), last_axis(last_axis), moves(moves) {
			prune = eprune::probe(ec);
			r = eprune::lookup(ec);
		}
	};

	static constexpr int SAFE_DEDUPE_DEPTH = 3;
	std::vector<std::vector<seed_t>> seeds;
	uint64_t self_sym;
	int search_depth;
	ecoord ec0;
	cube c0;
	moveseq moves;

	cprune cpr;
	int threshold = 0;

	tracker::handle &handle;
	bool parity;

    public:
	ecsolver_impl(tracker::handle &handle) : seeds(SAFE_DEDUPE_DEPTH + 1), handle(handle) {
		parity = handle.parity();
		ec0 = handle.ec();

		c0 = cube{ec0};

		self_sym = ec0.selfsym();
		seeds = find_seeds(ec0);

		rebuild_cpr();
	}

	void rebuild_cpr() {
		std::vector<ccoord> unsolved;
		for (auto cc : involution::corners()[parity]) {
			if (handle.is_unsolved(cc)) {
				unsolved.push_back(cc);
			}
		}
		if (unsolved.size() != handle.todo) abort();

		cpr.generate(unsolved);
		threshold = handle.todo / 2;
	}

	cube sym_rep(cube c) {
		cube rep = c;
		for (auto s : bits(self_sym)) {
			rep = std::min(rep, c.sym(s));
		}
		return rep;
	}

	decltype(seeds) find_seeds(cube c) {
		std::set<cube> seen = { c };
		std::vector<std::vector<seed_t>> seeds(SAFE_DEDUPE_DEPTH + 1);

		seeds[0].emplace_back(c, -1, moveseq{});

		for (int depth = 0; depth < SAFE_DEDUPE_DEPTH; depth++) {
			for (auto [ ec, cc, last_axis, moves, prune, r ] : seeds[depth]) {
				moves.push_back(0);
				cube c = cube(ec) * cube(cc);
				for (int m = 0; m < N_MOVES; m++) {
					moves.back() = m;
					int axis = m / 3;
					if (axis == last_axis || axis + 3 == last_axis) continue;
					cube c_m = c.move(m);
					if (seen.insert(sym_rep(c_m)).second) {
						seeds[depth + 1].emplace_back(c_m, axis, moves);
					}
				}
			}
		}

		return seeds;
	}

	void solve(int max_depth) {
		search_depth = handle.proven_min();
		for (; handle.todo && search_depth <= max_depth; search_depth++) {
			int seed_depth = std::min(search_depth, SAFE_DEDUPE_DEPTH);
			for (auto [ ec, cc, last_axis, seed_moves, prune, r ] : seeds[seed_depth]) {
				int remaining_depth = search_depth - seed_depth;
				if (remaining_depth < prune) {
					continue;
				}

				moves = seed_moves;

				if (remaining_depth == 0) {
					candidate(cc);
				} else {
					search(ec, cc, remaining_depth, r, prune, last_axis);
					if (interrupt::terminated()) {
						return;
					}
				}
			}

			handle.update_proven_min(search_depth + 1);
		}
	}

	void search(ecoord ec, ccoord cc, int depth, eprune::rec_t r, int prune, int last_axis) {
		if (cpr.prune(cc, depth)) {
			return;
		}

		depth--;
		moves.push_back(0);

		constexpr uint32_t ALL_MOVES = (1 << N_MOVES) - 1;
		if (depth == 0) {
			uint32_t move_mask = prune ? r.down : (ALL_MOVES ^ r.up);
			for (auto sym_m : bits(move_mask)) {
				int m = sym::movei(sym_m, sym::inv(ec.sym()));

				int axis = m / 3;
				if (axis == last_axis || axis + 3 == last_axis) {
					continue;
				}

				moves.back() = m;

				candidate(cc.move(m));

				if (!handle.todo) break;
			}
		} else {
			uint32_t move_mask;
			if (prune < depth) {
				move_mask = ALL_MOVES;
			} else if (prune == depth) {
				move_mask = ALL_MOVES ^ r.up;
			} else {
				move_mask = r.down;
			}

			for (auto sym_m : bits(move_mask)) {
				int m = sym::movei(sym_m, sym::inv(ec.sym()));

				int axis = m / 3;
				if (axis == last_axis || axis + 3 == last_axis) {
					continue;
				}

				auto ec_m = ec.move(m);
				auto cc_m = cc.move(m);
				auto r_m = eprune::lookup(ec_m);

				moves.back() = m;

				int next_prune = prune;
				next_prune += 1 & (r.up >> sym_m);
				next_prune -= 1 & (r.down >> sym_m);

				search(ec_m, cc_m, depth, r_m, next_prune, axis);
				if (!handle.todo) break;
				if (interrupt::terminated()) break;
			}
		}

		moves.pop_back();
	}

	void candidate(ccoord cc) {
		if (!handle.is_unsolved(cc)) {
			return;
		}

		cube c = cc;
		if (c * c != cube{}) return;

		handle.solution(moves);

		if (handle.todo && handle.todo <= threshold) {
			rebuild_cpr();
		}
	}
};

void ecsolver::solve(tracker::handle &handle, int depth) {
	ecsolver_impl solver{handle};
	solver.solve(depth);
}
