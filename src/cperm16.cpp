#include "cperm16.h"
#include "thread.h"

void cperm16::init() {
	static std::once_flag flag;
	std::call_once(flag, [&]() {
		cube::init();
		sym::init();
		generate();
	});
}

cperm16::cperm16(cube c) : idx() {
	cperm_t cp = c.getCornerPerm();
	for (int s = 1; s < N_SYM16; s++) {
		cperm_t cp_s = c.sym(s).getCornerPerm();
		if (cp_s < cp) {
			cp = cp_s;
			idx = s;
		}
	}

	auto it = std::lower_bound(s2r, s2r + N_CPERM16, cp);
	idx = sym::inv(idx);
	idx = (idx << 12) | std::distance(s2r, it);
}

void cperm16::generate() {
	self = new uint16_t[N_CPERM16]{};
	s2r = new cperm_t[N_CPERM]{};
	s2r_sym = new syms_t[N_CPERM]{};
	moves = new moves_t[N_CPERM16]{};

	std::vector<coord_t> r2s(N_CPERM);

	self[0] = (1 << N_SYM16) - 2;

	coord_t idx = 1;
	for (cperm_t cp = 1; cp < N_CPERM; cp++) {
		if (r2s[cp]) continue;

		s2r[idx] = cp;
		r2s[cp] = idx;

		cube c = cube{}.setCornerPerm(cp);

		for (int s = 1; s < N_SYM16; s++) {
			auto cp_s = c.sym(s).getCornerPerm();
			if (cp_s == cp) {
				self[idx] |= 1 << s;
			}
			if (!r2s[cp_s]) {
				r2s[cp_s] = idx | (coord_t(s) << 12);
			}
		}

		idx++;
	}

	if (idx != N_CPERM16) {
		abort();
	}

	for (coord_t idx = 0; idx < N_CPERM16; idx++) {
		cube c = cube{}.setCornerPerm(s2r[idx]);
		for (int m = 0; m < N_MOVES; m++) {
			moves[idx][m] = r2s[c.move(m).getCornerPerm()];
		}
		for (int s = 0; s < N_SYM16; s++) {
			s2r_sym[idx][s] = c.sym(s).getCornerPerm();
		}
	}
}
