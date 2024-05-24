#ifndef INVL_CUBE_H
#define INVL_CUBE_H

#include <array>
#include <cstdint>
#include "avx2_cube.h"
#include "moveseq.h"
#include "move.h"

constexpr int N_MOVES = 18;
constexpr int N_SYM16 = 16;
constexpr int N_SYM48 = 48;
constexpr size_t N_EPERM = 479001600;
constexpr size_t N_CPERM = 40320;
constexpr size_t N_EORIENT = 2048;
constexpr size_t N_CORIENT = 2187;

using eperm_t = uint32_t;
using cperm_t = uint16_t;
using eorient_t = uint16_t;
using corient_t = uint16_t;

class cube {
	inline static const cube *moves = NULL;
	inline static const cube *syms = NULL;
	inline static const uint8_t *sym_inv = NULL;

	__m256i v;

	constexpr cube(__m256i v) : v(v) {
	}

	cube symRep(int n_sym) const {
		cube r = *this;
		for (int s = 1; s < n_sym; s++) {
			r = std::min(r, sym(s));
		}
		return r;
	}

    public:
	static void init();

	constexpr cube() : v(avx2::identity) {
	}

	constexpr cube(uint64_t corners, uint64_t edges_high, uint64_t edges_low) :
		v(avx2::literal(corners, edges_high, edges_low))
	{
	}

	static cube from_moveseq(const moveseq &);

	static cube from_moves(const std::string &s) {
		return from_moveseq(moveseq::parse(s));
	}

	std::string to_sing() const;

	bool operator == (const cube &rhs) const {
		return avx2::equals(v, rhs.v);
	}

	bool operator != (const cube &rhs) const {
		return !(*this == rhs);
	}

	bool operator < (const cube &o) const {
		return avx2::less_than(v, o.v);
	}

	cube operator * (cube rhs) const {
		return compose(rhs, false);
	}

	cube operator % (cube rhs) const {
		return compose(rhs, true);
	}

	cube operator ~ () const {
		return avx2::invert(v);
	}

	cube compose(const cube &rhs, bool mirror = false) const {
		return avx2::compose(v, rhs.v, mirror);
	}

	cube move(int m) const {
		return compose(moves[m]);
	}

	cube premove(int m) const {
		return moves[m].compose(*this);
	}

	cube sym(int s) const {
		cube c_s = syms[s], c_si = syms[sym_inv[s]];
		return c_si.compose(*this, s & 1).compose(c_s, s & 1);
	}

	cube symi(int s) const {
		cube c_s = syms[s], c_si = syms[sym_inv[s]];
		return c_s.compose(*this, s & 1).compose(c_si, s & 1);
	}

	eperm_t getEdgePerm() const;

	cperm_t getCornerPerm() const;

	eorient_t getEdgeOrientRaw() const {
		return avx2::bitmask(v, 4) & 0xfff;
	}

	eorient_t getEdgeOrient() const {
		return getEdgeOrientRaw() & 0x7ff;
	}

	corient_t getCornerOrient() const {
		return avx2::corner_orient(v);
	}

	cube setEdgePerm(eperm_t eperm) const;

	cube setCornerPerm(cperm_t cperm) const;

	cube setEdgeOrient(eorient_t eorient) const;

	cube setCornerOrient(corient_t corient) const;

	bool parity() const {
		return avx2::parity(v);
	}

	cube symRep16() const {
		return symRep(N_SYM16);
	}

	cube symRep48() const {
		return symRep(N_SYM48);
	}

	uint64_t selfSym48() const {
		uint64_t self_sym = 1;
		for (int s = 1; s < N_SYM48; s++) {
			if (*this == sym(s)) {
				self_sym |= 1LL << s;
			}
		}
		return self_sym;
	}

	using edge_array_t = std::array<uint8_t, 12>;
	edge_array_t getEdges() const;
	cube setEdges(const edge_array_t &) const;

	using corner_array_t = std::array<uint8_t, 8>;
	corner_array_t getCorners() const;
	cube setCorners(const corner_array_t &) const;
};

#endif
