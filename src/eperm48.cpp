#include "eperm48.h"
#include "thread.h"
#include "alloc.h"
#include "tables.h"
#include "bits.h"

using tables::load;
using tables::save;

void eperm48::init() {
	static std::once_flag flag;
	std::call_once(flag, [&]() {
		cube::init();
		sym::init();

		s2r = alloc::huge<eperm_t>(N_EPERM48);
		moves = alloc::huge<moves_t>(N_EPERM48);
		self = alloc::huge<uint64_t>(N_EPERM48);

		bool ok =
			load(FNAME_S2R, s2r, N_EPERM48) &&
			load(FNAME_MOVE, moves, N_EPERM48) &&
			load(FNAME_SELF, self, N_EPERM48);
		if (!ok) {
			std::cerr << "generating eperm48 tables\n";
			generate();

			bool ok =
				save(FNAME_S2R, s2r, N_EPERM48) &&
				save(FNAME_MOVE, moves, N_EPERM48) &&
				save(FNAME_SELF, self, N_EPERM48);
			if (!ok) {
				std::cerr << "error saving eperm48 tables\n";
			}
		}
	});
}

eperm48::eperm48(cube c) : idx() {
	eperm_t ep = c.getEdgePerm();
	for (int s = 1; s < N_SYM48; s++) {
		eperm_t ep_s = c.sym(s).getEdgePerm();
		if (ep_s < ep) {
			ep = ep_s;
			idx = s;
		}
	}

	auto it = std::lower_bound(s2r, s2r + N_EPERM48, ep);
	idx = sym::inv(idx);
	idx = (idx << 24) | std::distance(s2r, it);
}

void eperm48::generate() {
	std::vector<coord_t> r2s(N_EPERM);

	std::barrier barrier(N_WORKERS);
	std::vector<int> rep_base(N_WORKERS + 1);

	parallel workers([&](size_t id) {
		size_t block = N_EPERM / N_WORKERS;
		size_t ep_start = id * block, ep_end = ep_start + block;
		if (id == N_WORKERS) ep_end = N_EPERM;

		std::vector<std::pair<eperm_t, uint64_t>> reps;

		for (eperm_t ep = ep_start; ep < ep_end; ep++) {
			cube c = cube{}.setEdgePerm(ep);

			uint64_t self_sym = 0;
			for (int s = 1; s < N_SYM48; s++) {
				auto ep_s = c.sym(s).getEdgePerm();
				if (ep_s == ep) {
					self_sym |= 1LL << s;
				} else if (ep_s < ep) {
					self_sym = -1;
					break;
				}
			}
			if (self_sym != -1) {
				reps.emplace_back(ep, self_sym);
			}
		}

		rep_base[id + 1] = reps.size();

		barrier.arrive_and_wait();

		if (id == 0) {
			for (int i = 1; i <= N_WORKERS; i++) {
				rep_base[i] += rep_base[i - 1];
			}
		}

		barrier.arrive_and_wait();

		coord_t idx = rep_base[id];
		for (auto [ ep, self_sym ] : reps) {
			self[idx] = self_sym;
			s2r[idx] = ep;
			r2s[ep] = idx;

			cube c = cube{}.setEdgePerm(ep);

			uint64_t non_self = ((1LL << N_SYM48) - 2) ^ self_sym;
			for (auto s : bits(non_self)) {
				r2s[c.sym(s).getEdgePerm()] = (coord_t(s) << 24) | idx;
			}

			idx++;
		}

		barrier.arrive_and_wait();

		idx = rep_base[id];
		for (auto [ ep, self_sym ] : reps) {
			cube c = cube{}.setEdgePerm(ep);

			for (int m = 0; m < N_MOVES; m++) {
				moves[idx][m] = r2s[c.move(m).getEdgePerm()];
			}

			idx++;
		}
	});

	workers.join();
}
