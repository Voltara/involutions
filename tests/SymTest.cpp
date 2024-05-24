#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "sym.h"
#include "cube.h"

TEST_GROUP(Sym) {
};

TEST(Sym, Inverse) {
	cube c = cube::from_moves("U1R1");
	for (int s = 0; s < N_SYM48; s++) {
		CHECK(c.symi(s) == c.sym(sym::inv(s)));
	}
}

TEST(Sym, Product) {
	cube t = t::random_cube();
	for (int i = 0; i < N_SYM48; i++) {
		cube ti = t.sym(i);
		for (int j = 0; j < N_SYM48; j++) {
			CHECK(ti.sym(j) == t.sym(sym::compose(i, j)));
		}
	}
}

TEST(Sym, Move) {
	for (int m = 0; m < N_MOVES; m++) {
		for (int s = 0; s < N_SYM48; s++) {
			CHECK(cube{}.move(m).sym(s) == cube{}.move(sym::move(m, s)));
			CHECK(cube{}.move(m).symi(s) == cube{}.move(sym::movei(m, s)));
		}
	}
}
