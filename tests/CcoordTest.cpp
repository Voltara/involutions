#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "ccoord.h"

TEST_GROUP(Ccoord) {
};

TEST(Ccoord, Cube) {
	for (int i = 0; i < 100; i++) {
		cube c = t::random_cube().setEdgePerm(0);
		CHECK(c == ccoord{c});
	}
}

TEST(Ccoord, IsSolved) {
	CHECK_TRUE(ccoord{cube{}}.is_solved());
	CHECK_FALSE(ccoord{t::random_cube()}.is_solved());
}

TEST(Ccoord, Normalize) {
	bool any_different = false;
	for (int i = 0; i < 100; i++) {
		cube c = t::random_cube().setEdgePerm(0);
		cube d = ccoord{c}.normalize();
		any_different |= (c != d);
		CHECK(c.symRep16() == d.symRep16());

		auto [ cp, co ] = ccoord{c}.coord();
		CHECK(cube(ccoord{cp, co}) == cube(ccoord{c}.normalize()));
	}
	CHECK_TRUE(any_different);
}

TEST(Ccoord, Real) {
	for (int i = 0; i < 100; i++) {
		cube c = t::random_cube();
		auto [ cp, co ] = ccoord{c}.real();
		CHECK(cp == c.getCornerPerm());
		CHECK(co == c.getCornerOrient());
	}
}

TEST(Ccoord, Move) {
	cube c = t::random_cube();
	ccoord cc = c;

	for (int m = 0; m < N_MOVES; m++) {
		c = c.move(m);
		cc = cc.move(m);
	}

	CHECK(c.setEdgePerm(0) == cube{cc});
}

TEST(Ccoord, PruneIdx) {
	CHECK(ccoord{}.prune_idx() == 0);
	for (int i = 0; i < 100; i++) {
		ccoord cc{t::random_cube()};
		auto [ cp, co ] = cc.coord();
		CHECK(cc.prune_idx() == cp * N_CORIENT + co);
	}
}
