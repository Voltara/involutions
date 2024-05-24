#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "involution.h"

TEST_GROUP(Involution) {
};

TEST(Involution, Edges) {
	auto edges = involution::edges();

	CHECK_EQUAL(172988, edges[0].size());
	CHECK_EQUAL(163016, edges[1].size());

	for (int parity = 0; parity < 2; parity++) {
		std::pair<int, int> prev = { -1, -1 };
		for (auto ec : edges[parity]) {
			CHECK(cube(ec) == cube(ec.normalize()));
			CHECK(cube(ec) * cube(ec) == cube{});
			std::pair<int, int> cur = ec.coord();
			CHECK(prev < cur);
			prev = cur;
		}
	}
}

TEST(Involution, Corners) {
	auto corners = involution::corners();

	CHECK_EQUAL(10396, corners[0].size());
	CHECK_EQUAL(11424, corners[1].size());

	for (int parity = 0; parity < 2; parity++) {
		std::pair<int, int> prev = { -1, -1 };
		for (auto cc : corners[parity]) {
			CHECK(cube(cc) * cube(cc) == cube{});
			std::pair<int, int> cur = cc.real();
			CHECK(prev < cur);
			prev = cur;
		}
	}
}
