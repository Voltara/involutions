#include <map>
#include "cube.h"
#include "sym.h"
#include "thread.h"

static auto init_syms() {
	static std::array<uint8_t, N_SYM48> product[N_SYM48];
	static uint8_t inverse[N_SYM48];

	cube c = cube::from_moves("U1R1");

	std::map<cube, int> lookup;
	for (int i = 0; i < N_SYM48; i++) {
		lookup.emplace(c.sym(i), i);
	}

	for (int i = 0; i < N_SYM48; i++) {
		cube c_i = c.sym(i);
		for (int j = 0; j < N_SYM48; j++) {
			product[i][j] = lookup[c_i.sym(j)];
		}
		inverse[i] = lookup[c.symi(i)];
	}

	return std::tuple(product, inverse);
}

static auto init_moves() {
	static std::array<uint8_t, N_SYM48> moves[N_MOVES];

	std::map<cube, int> lookup;
	for (int m = 0; m < N_MOVES; m++) {
		lookup.emplace(cube{}.move(m), m);
	}

	for (int m = 0; m < N_MOVES; m++) {
		auto c = cube{}.move(m);
		for (int s = 0; s < N_SYM48; s++) {
			moves[m][s] = lookup[c.symi(s)];
		}
	}

	return moves;
}

void sym::init() {
	static std::once_flag flag;
	std::call_once(flag, [&]() {
		cube::init();
		std::tie(product, inverse) = init_syms();
		moves = init_moves();
	});
}
