#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "ecoord.h"

TEST_GROUP(Ecoord) {
};

TEST(Ecoord, Cube) {
	for (int i = 0; i < 100; i++) {
		cube c = t::random_cube().setCornerPerm(0);
		CHECK(c == ecoord{c});
	}
}

TEST(Ecoord, IsSolved) {
	CHECK_TRUE(ecoord{cube{}}.is_solved());
	CHECK_FALSE(ecoord{t::random_cube()}.is_solved());
}

TEST(Ecoord, Normalize) {
	bool any_different = false;
	for (int i = 0; i < 100; i++) {
		cube c = t::random_cube().setCornerPerm(0);
		cube d = ecoord{c}.normalize();
		any_different |= (c != d);
		CHECK(c.symRep48() == d.symRep48());

		auto [ cp, co ] = ecoord{c}.coord();
		CHECK(cube(ecoord{cp, co}) == cube(ecoord{c}.normalize()));
	}
	CHECK_TRUE(any_different);
}

TEST(Ecoord, Move) {
	cube c = t::random_cube();
	ecoord ec = c;

	for (int m = 0; m < N_MOVES; m++) {
		c = c.move(m);
		ec = ec.move(m);
	}

	CHECK(c.setCornerPerm(0) == cube{ec});
}

TEST(Ecoord, SelfSym) {
	for (int i = 0; i < 100; i++) {
		cube c = t::random_cube();
		CHECK_EQUAL(c.selfSym48() ^ 1, ecoord{c}.selfsym());
	}
}
