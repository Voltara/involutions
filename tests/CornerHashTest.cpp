#include <bitset>
#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "corner_hash.h"
#include "involution.h"

TEST_GROUP(CornerHash) {
};

TEST(CornerHash, InvolutionsHashPerfectly) {
	auto corners = involution::corners();

	for (int parity = 0; parity < 2; parity++) {
		std::bitset<65536> seen;
		for (auto cc : corners[parity]) {
			seen.set(corner_hash(cc));
		}
		CHECK_EQUAL(corners[parity].size(), seen.count());
	}
}
