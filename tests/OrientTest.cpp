#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "orient.h"

TEST_GROUP(Orient) {
};

TEST(Orient, Edge) {
	for (int s = 0; s < N_SYM48; s++) {
		cube c = t::random_cube();
		eorient eo = c.getEdgeOrient();
		CHECK(eo.symi(s) == c.symi(s).getEdgeOrient());
		for (int m = 0; m < N_MOVES; m++) {
			CHECK(eo.movei(m, s) == c.move(m).symi(s).getEdgeOrient());
		}
	}
}

TEST(Orient, Corner) {
	for (int s = 0; s < N_SYM16; s++) {
		cube c = t::random_cube();
		corient co = c.getCornerOrient();
		CHECK(co.symi(s) == c.symi(s).getCornerOrient());
		for (int m = 0; m < N_MOVES; m++) {
			CHECK(co.movei(m, s) == c.move(m).symi(s).getCornerOrient());
		}
	}
}
