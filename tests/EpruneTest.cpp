#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "eprune.h"

TEST_GROUP(Eprune) {
	cube solve(cube c) {
		for (int p0 = eprune::probe(c); p0 > 0; p0--) {
			int m;
			for (m = 0; m < N_MOVES; m++) {
				int p = eprune::probe(c.move(m));
				CHECK(std::abs(p - p0) <= 1);
				if (p < p0) {
					break;
				}
			}
			CHECK(m != N_MOVES);
			c = c.move(m);
		}
		return c;
	}
};

TEST(Eprune, Solve) {
	CHECK(eprune::probe(cube{}) == 0);

	for (int i = 0; i < 100; i++) {
		cube c = solve(t::random_cube());
		CHECK(c.setCornerPerm(0) == cube{});
	}
};
