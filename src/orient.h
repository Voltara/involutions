#ifndef INVL_ORIENT_H
#define INVL_ORIENT_H

#include <cstdint>
#include <array>
#include "cube.h"

template<int N_ELEM, int N_SYM>
class ori_elem {
	using lookup_t = std::array<ori_elem, N_ELEM>;

	inline static const lookup_t *moves = NULL;
	inline static const lookup_t *syms = NULL;
	inline static const uint64_t *self = NULL;

	uint16_t idx;

    public:
	static void init();

	ori_elem(uint16_t idx = 0) : idx(idx) {
	}

	operator uint16_t () const {
		return idx;
	}

	ori_elem movei(int m, int s) const {
		return moves[m][idx].symi(s);
	}

	ori_elem symi(int s) const {
		return syms[s][idx];
	}

	uint64_t selfsym() const {
		return self[idx];
	}
};

using eorient = ori_elem<N_EORIENT, N_SYM48>;
using corient = ori_elem<N_CORIENT, N_SYM16>;

#endif
