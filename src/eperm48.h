#ifndef INVL_EPERM48_H
#define INVL_EPERM48_H

#include <cstdint>
#include "cube.h"
#include "sym.h"

constexpr size_t N_EPERM48 = 9985968;

class eperm48 {
	using coord_t = uint32_t;
	using moves_t = coord_t[N_MOVES];

	coord_t idx;

	eperm48(coord_t idx, coord_t sym) : idx(idx | (sym << 24)) {
	}

	static void generate();

    public:
	static void init();

	eperm48(coord_t idx = 0) : idx(idx) { }
	eperm48(cube c);

	coord_t index() const { return idx & 0xffffff; }
	coord_t sym() const { return idx >> 24; }

	auto move_ex(int m) const {
		m = sym::movei(m, sym());
		eperm48 ep = moves[index()][m];
		auto s = ep.sym();
		ep = eperm48{ep.index(), sym::compose(s, sym())};
		return std::tuple(ep, m, s);
	}

	eperm48 move(int m) const {
		return std::get<0>(move_ex(m));
	}

	eperm_t rep() const {
		return s2r[index()];
	}

	static auto raw_move(size_t idx, int m) {
		coord_t idx_m = moves[idx][m];
		return std::tuple(idx_m & 0xffffff, idx_m >> 24);
	}

	static uint64_t selfsym(size_t idx) {
		return self[idx];
	}

    private:
	inline static eperm_t *s2r = NULL;
	inline static moves_t *moves = NULL;
	inline static uint64_t *self = NULL;

	static constexpr char FNAME_S2R[] = "eperm48S2R.dat";
	static constexpr char FNAME_MOVE[] = "eperm48Move.dat";
	static constexpr char FNAME_SELF[] = "eperm48SelfSym.dat";
};

#endif
