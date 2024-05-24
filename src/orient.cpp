#include <functional>
#include "orient.h"
#include "sym.h"
#include "thread.h"

template<typename T>
static auto init_moves(T moves, std::function<cube(int)> set, std::function<int(cube)> get) {
	for (int m = 0; m < N_MOVES; m++) {
		for (int o = 0; o < moves->size(); o++) {
			moves[m][o] = get(set(o).move(m));
		}
	}
	return moves;
}

template<int N_SYM, typename T>
static auto init_syms(T syms, std::function<cube(int)> set, std::function<int(cube)> get) {
	for (int s = 0; s < N_SYM; s++) {
		for (int o = 0; o < syms->size(); o++) {
			syms[s][o] = get(set(o).symi(s));
		}
	}
	return syms;
}

template<int N_SYM, typename T, typename S>
static auto init_self(T self, S syms) {
	for (int o = 0; o < syms->size(); o++) {
		uint64_t self_sym = 0;
		for (int s = 1; s < N_SYM; s++) {
			if (syms[s][o] == o) {
				self_sym |= 1LL << s;
			}
		}
		self[o] = self_sym;
	}
	return self;
}

template<>
void eorient::init() {
	static lookup_t moves_[N_MOVES], syms_[N_SYM48];
	static uint64_t self_[N_EORIENT];

	static std::once_flag flag;
	std::call_once(flag, [&]() {
		cube::init();
		sym::init();

		auto set = [](int o) { return cube{}.setEdgeOrient(o); };
		auto get = [](cube c) { return c.getEdgeOrient(); };

		moves = init_moves(moves_, set, get);
		syms = init_syms<N_SYM48>(syms_, set, get);
		self = init_self<N_SYM48>(self_, syms_);
	});
}

template<>
void corient::init() {
	static lookup_t moves_[N_MOVES], syms_[N_SYM16];

	static std::once_flag flag;
	std::call_once(flag, [&]() {
		cube::init();
		sym::init();

		auto set = [](int o) { return cube{}.setCornerOrient(o); };
		auto get = [](cube c) { return c.getCornerOrient(); };

		moves = init_moves(moves_, set, get);
		syms = init_syms<N_SYM16>(syms_, set, get);
		// ignore self_sym
	});
}
