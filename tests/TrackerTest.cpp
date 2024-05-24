#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "tracker.h"

TEST_GROUP(Tracker) {
};

TEST(Tracker, Solution) {
	tracker::solution sol;
	CHECK_FALSE(sol.valid());
}

TEST(Tracker, EmptySolution) {
	tracker::solution sol = moveseq{};
	CHECK_TRUE(sol.valid());
	CHECK_EQUAL(0, sol.length());
	CHECK(moveseq(sol) == moveseq{});
}

TEST(Tracker, SolutionConvert) {
	for (int i = 0; i < 100; i++) {
		moveseq moves;
		int last_axis = -1;
		for (int i = 0; i < 20; i++) {
			int axis;
			do { axis = t::rand(6); } while (axis == last_axis);
			last_axis = axis;
			moves.push_back(axis * 3 + t::rand(3));

			tracker::solution sol = moves;

			CHECK_TRUE(sol.valid());
			CHECK_EQUAL(moves.canonical().size(), sol.length());
			CHECK(moves.canonical() == moveseq(sol));
		}
	}
}
