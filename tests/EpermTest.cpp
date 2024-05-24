#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "eperm48.h"
#include "sym.h"
#include "bits.h"

TEST_GROUP(Eperm) {
	eperm_t get_real(eperm48 ep) {
		return cube{}
			.setEdgePerm(ep.rep())
			.sym(ep.sym())
			.getEdgePerm();
	}
};

TEST(Eperm, Cube) {
	for (int i = 0; i < 100; i++) {
		cube c = t::random_cube();
		CHECK(c.getEdgePerm() == get_real(eperm48{c}));
	}
}

TEST(Eperm, SelfSym) {
	auto self_sym = [](cube c) {
		uint64_t self = 0;
		for (int s = 1; s < N_SYM48; s++) {
			if (c == c.sym(s)) {
				self |= 1LL << s;
			}
		}
		return self;
	};

	CHECK(self_sym(cube{}) == eperm48::selfsym(0));
	for (int i = 0; i < 100; i++) {
		cube c = cube{}.setEdgePerm(t::rand(N_EPERM));
		c = c.setEdgePerm(eperm48{c}.rep());
		CHECK(self_sym(c) == eperm48::selfsym(eperm48{c}.index()));
	}
}

TEST(Eperm, SelfSymInverses) {
	for (int i = 0; i < 100; i++) {
		auto self_sym = eperm48::selfsym(t::rand(N_EPERM48));
		for (auto s : bits(self_sym)) {
			CHECK_TRUE(self_sym & (1 << sym::inv(s)));
		}
	}
}

TEST(Eperm, Move) {
	cube c = t::random_cube();
	eperm48 ep{c};
	for (int m = 0; m < N_MOVES; m++) {
		c = c.move(m);
		ep = ep.move(m);
	}
	CHECK(c.getEdgePerm() == get_real(ep));
}

TEST(Eperm, Rep) {
	auto min_eperm = [](cube c) {
		eperm_t min = N_CPERM;
		for (int s = 0; s < N_SYM48; s++ ){
			min = std::min(min, c.sym(s).getEdgePerm());
		}
		return min;
	};

	for (int i = 0; i < 100; i++) {
		cube c = cube{}.setEdgePerm(t::rand(N_CPERM));
		CHECK(min_eperm(c) == eperm48{c}.rep());
	}
}
