#ifndef INVL_SYM48_H
#define INVL_SYM48_H

#include <cstdint>
#include <vector>
#include <array>

class sym {
	using sym_t = uint8_t;

	inline static const std::array<uint8_t, N_SYM48> *moves = NULL;
	inline static const std::array<sym_t, N_SYM48> *product = NULL;
	inline static const sym_t *inverse = NULL;

    public:
	static void init();

	static sym_t inv(sym_t s) {
		return inverse[s];
	}

	static sym_t compose(sym_t lhs, sym_t rhs) {
		return product[lhs][rhs];
	}

	static uint8_t movei(int m, sym_t s) {
		return moves[m][s];
	}

	static uint8_t move(int m, sym_t s) {
		return moves[m][inv(s)];
	}
};

#endif
