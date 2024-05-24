#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "cperm16.h"
#include "bits.h"

TEST_GROUP(Cperm) {
};

TEST(Cperm, Cube) {
	for (int i = 0; i < 100; i++) {
		cube c = t::random_cube();
		CHECK(c.getCornerPerm() == cperm16{c}.real());
	}
}

TEST(Cperm, SelfSym) {
	auto self_sym = [](cube c) {
		int self = 0;
		for (int s = 1; s < N_SYM16; s++) {
			if (c == c.sym(s)) {
				self |= 1 << s;
			}
		}
		return self;
	};

	CHECK(self_sym(cube{}) == cperm16::selfsym(0));
	for (int i = 0; i < 100; i++) {
		cube c = cube{}.setCornerPerm(t::rand(N_CPERM));
		c = c.setCornerPerm(cperm16{c}.rep());
		CHECK(self_sym(c) == cperm16::selfsym(cperm16{c}.index()));
	}
}

TEST(Cperm, SelfSymInverses) {
	for (int i = 0; i < 100; i++) {
		auto self_sym = cperm16::selfsym(t::rand(N_CPERM16));
		for (auto s : bits(self_sym)) {
			CHECK_TRUE(self_sym & (1 << sym::inv(s)));
		}
	}
}

TEST(Cperm, Move) {
	cube c = t::random_cube();
	cperm16 cp{c};
	for (int m = 0; m < N_MOVES; m++) {
		c = c.move(m);
		cp = cp.move(m);
	}
	CHECK(c.getCornerPerm() == cp.real());
}

TEST(Cperm, Rep) {
	auto min_cperm = [](cube c) {
		cperm_t min = N_CPERM;
		for (int s = 0; s < N_SYM16; s++ ){
			min = std::min(min, c.sym(s).getCornerPerm());
		}
		return min;
	};

	for (int i = 0; i < 100; i++) {
		cube c = cube{}.setCornerPerm(t::rand(N_CPERM));
		CHECK(min_cperm(c) == cperm16{c}.rep());
	}
}
