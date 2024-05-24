#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "move.h"

TEST_GROUP(Move) {
};

TEST(Move, Inverse) {
	for (int m = 0; m < 18; m += 3) {
		CHECK_EQUAL(m + 2, move::inv(m));
	}
	for (int m = 1; m < 18; m += 3) {
		CHECK_EQUAL(m, move::inv(m));
	}
	for (int m = 2; m < 18; m += 3) {
		CHECK_EQUAL(m - 2, move::inv(m));
	}
}
