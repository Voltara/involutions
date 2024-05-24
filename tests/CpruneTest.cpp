#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "cprune.h"

TEST_GROUP(Cprune) {
       	cube solve(cprune &cpr, cube c) {
		for (int p0 = cpr.prune(c); p0 > 0; p0--) {
			int m;
			for (m = 0; m < N_MOVES; m++) {
				int p = cpr.prune(c.move(m));
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

TEST(Cprune, IdentitySolves) {
	cprune cpr;
	cpr.generate({ ccoord{} });

	CHECK(cpr.prune(cube{}) == 0);

	for (int i = 0; i < 100; i++) {
		cube c = solve(cpr, t::random_cube());
		CHECK(c.setEdgePerm(0) == cube{});
	}
};

TEST(Cprune, MultipleSeeds) {
	cube a = cube::from_moves("R1U1R3U1R1U2R3");
	cube b = cube::from_moves("U1R1F1D1L1B1");

	cprune cpr;
	cpr.generate({ a, b });

	a = a.setEdgePerm(0).symRep16();
	b = b.setEdgePerm(0).symRep16();

	bool found_a = false, found_b = false;

	for (int i = 0; i < 100; i++) {
		cube c = solve(cpr, t::random_cube());
		c = c.setEdgePerm(0).symRep16();
		if (c == a) {
			found_a = true;
		} else if (c == b) {
			found_b = true;
		} else {
			FAIL("solved cube was not one of the seeds");
		}
	}

	CHECK(found_a && found_b);
};
