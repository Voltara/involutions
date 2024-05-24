#ifndef INVL_CPERM16_H
#define INVL_CPERM16_H

#include <cstdint>
#include "cube.h"
#include "sym.h"

constexpr size_t N_CPERM16 = 2768;

class cperm16 {
	using coord_t = uint16_t;
	using moves_t = coord_t[N_MOVES];
	using syms_t = cperm_t[N_SYM16];

	coord_t idx;

	cperm16(coord_t idx, coord_t sym) : idx(idx | (sym << 12)) {
	}

	static void generate();

    public:
	static void init();

	cperm16(coord_t idx = 0) : idx(idx) { }
	cperm16(cube c);

	coord_t index() const { return idx & 0xfff; }
	coord_t sym() const { return idx >> 12; }

	auto move_ex(int m) const {
		m = sym::movei(m, sym());
		cperm16 cp = moves[index()][m];
		auto s = cp.sym();
		cp = cperm16{cp.index(), sym::compose(s, sym())};
		return std::tuple(cp, m, s);
	}

	cperm16 move(int m) const {
		return std::get<0>(move_ex(m));
	}

	cperm_t rep() const {
		return s2r[index()];
	}

	cperm_t real() const {
		return s2r_sym[index()][sym()];
	}

	static uint16_t selfsym(size_t idx) {
		return self[idx];
	}

    private:
	inline static cperm_t *s2r = NULL;
	inline static syms_t *s2r_sym = NULL;
	inline static moves_t *moves = NULL;
	inline static uint16_t *self = NULL;
};

#endif
