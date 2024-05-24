#include <numeric>
#include "cube.h"
#include "thread.h"

// Fundamental cubes; edge flip is counted in quarter turn parity
constexpr cube M_U    = {0x0706050402010003,0x0b0a0908,0x0706050412111013};
constexpr cube S_URF3 = {0x1226172321152410,0x02060400,0x0a070b0309050801};
constexpr cube S_U4   = {0x0605040702010003,0x1a19181b,0x1615141712111013};
constexpr cube S_LR2  = {0x0607040502030001,0x0a0b0809,0x0704050603000102};

static auto init_syms() {
	static cube syms[N_SYM48];
	static uint8_t sym_inv[N_SYM48];

	const auto S_F2 = S_URF3 * S_U4 * S_U4 * ~S_URF3;

	syms[0] = cube{};
	syms[1] = S_LR2;
	for (int i =  2; i <  4; i++) syms[i] = S_F2   * syms[i -  2];
	for (int i =  4; i < 16; i++) syms[i] = S_U4   * syms[i -  4];
	for (int i = 16; i < 48; i++) syms[i] = S_URF3 * syms[i - 16];

	for (int i = 0; i < 48; i++) {
		for (int j = 0; j < 48; j++) {
			if (syms[i].compose(syms[j], i & 1) == cube{}) {
				sym_inv[i] = j;
			}
		}
	}

	return std::make_pair(syms, sym_inv);
}

static const cube * init_moves() {
	static cube moves[N_MOVES];

	moves[0] = M_U;

	for (int i = 1; i <  3; i++) moves[i] =  moves[i - 1] * moves[0];
	for (int i = 3; i <  9; i++) moves[i] =  moves[i - 3].sym(16);
	for (int i = 9; i < 18; i++) moves[i] = ~moves[i - 9].sym(11);

	return moves;
}

void cube::init() {
	static std::once_flag flag;
	std::call_once(flag, [&]() {
		std::tie(syms, sym_inv) = init_syms();
		moves = init_moves();
	});
}

cube cube::from_moveseq(const moveseq &moves) {
	cube c;
	for (auto m : moves) {
		c = c.move(m);
	}
	return c;
}

namespace {
struct sing {
	static constexpr char edge_symbol[] =
		"URU     UFU     ULU     UBU     "
		"DRD     DFD     DLD     DBD     "
		"FRF     FLF     BLB     BRB     ";
	static constexpr cube::edge_array_t edge_map = { 1, 0, 3, 2, 5, 4, 7, 6, 8, 9, 11, 10 };
	static constexpr cube::edge_array_t edge_parity = { 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

	static constexpr char corner_symbol[] =
		"UFRUF   ULFUL   UBLUB   URBUR   "
		"DRFDR   DFLDF   DLBDL   DBRDB   ";
	static constexpr cube::corner_array_t corner_map = { 0, 3, 2, 1, 4, 5, 6, 7 };
	static constexpr cube::corner_array_t corner_parity = { };

	template<typename T>
	static void output(std::string &s, const T &arr, const T &map, const T &par, const char *symbol, int len) {
		for (auto idx : map) {
			int perm = arr[idx] & 0xf;
			int ori = (arr[idx] >> 4) ^ par[idx] ^ par[perm];
			int offset = perm * 8 + ori;
			s.append(symbol + offset, symbol + offset + len);
			s.push_back(' ');
		}
	}

	static void output_edges(std::string &s, const cube::edge_array_t &arr) {
		output(s, arr, edge_map, edge_parity, edge_symbol, 2);
	}

	static void output_corners(std::string &s, const cube::corner_array_t &arr) {
		output(s, arr, corner_map, corner_parity, corner_symbol, 3);
	}
};
}

std::string cube::to_sing() const {
	std::string s;
	sing::output_edges(s, getEdges());
	sing::output_corners(s, getCorners());
	s.pop_back();
	return s;
}

eperm_t cube::getEdgePerm() const {
	uint64_t table = 0xba9876543210;
	eperm_t eperm = 0;

	uint64_t e = avx2::edges_low(v) << 2;
	for (int i = 0; i < 8; i++) {
		int shift = e & 0x3c;
		eperm = eperm * (12 - i) + _bextr_u64(table, shift, 4);
		table -= 0x111111111110LL << shift;
		e >>= 8;
	}

	e = avx2::edges_high(v) << 2;
	for (int i = 8; i < 11; i++) {
		int shift = e & 0x3c;
		eperm = eperm * (12 - i) + _bextr_u64(table, shift, 4);
		table -= 0x111111111110LL << shift;
		e >>= 8;
	}

	return eperm;
}

cperm_t cube::getCornerPerm() const {
	uint32_t table = 0x76543210;
	cperm_t cperm = 0;

	uint64_t c = avx2::corners(v) << 2;
	for (int i = 0; i < 7; i++) {
		int shift = c & 0x3c;
		cperm = cperm * (8 - i) + _bextr_u64(table, shift, 4);
		table -= 0x11111110 << shift;
		c >>= 8;
	}

	return cperm;
}

cube cube::setEdgePerm(eperm_t eperm) const {
	constexpr uint32_t fc[] = { 39916800, 3628800, 362880, 40320, 5040, 720, 120, 24, 6, 2, 1 };
	uint64_t table = 0xba9876543210;

	union { __m128i v; uint8_t e[16]; } u = { .v = _mm256_extracti128_si256(v, 0) };

	// Special case, first iteration does not need "% 12"
	for (int i = 0; i < 1; i++) {
		int shift = eperm / fc[i] * 4;
		u.e[i] = _bextr_u64(table, shift, 4);
		table ^= (table ^ (table >> 4)) & (int64_t(-1) << shift);
	}

	for (int i = 1; i < 11; i++) {
		int shift = eperm / fc[i] % (12 - i) * 4;
		u.e[i] = _bextr_u64(table, shift, 4);
		table ^= (table ^ (table >> 4)) & (int64_t(-1) << shift);
	}

	u.e[11] = table;

	return _mm256_inserti128_si256(v, u.v, 0);
}

cube cube::setCornerPerm(cperm_t cperm) const {
	constexpr uint32_t fc[] = { 5040, 720, 120, 24, 6, 2, 1 };
	uint32_t table = 0x76543210;

	union { int64_t v; uint8_t c[8]; } u = { .v = _mm256_extract_epi64(v, 2) };

	for (int i = 0; i < 7; i++) {
		int shift = cperm / fc[i] % (8 - i) * 4;
		u.c[i] = _bextr_u64(table, shift, 4);
		table ^= (table ^ (table >> 4)) & (-1L << shift);
	}

	u.c[7] = table;

	return _mm256_insert_epi64(v, u.v, 2);
}

static eorient_t set_eorient_parity(eorient_t eorient) {
	eorient_t parity = eorient ^ (eorient << 6);
	parity ^= parity << 3;
	parity ^= (parity << 2) ^ (parity << 1);
	return eorient ^ (parity & 0x800);
}

cube cube::setEdgeOrient(eorient_t eorient) const {
	return avx2::xor_edge_orient(v, set_eorient_parity(eorient ^ getEdgeOrientRaw()));
}

cube cube::setCornerOrient(corient_t corient) const {
	uint64_t u = _mm256_extract_epi64(v, 2);
	u = (u & 0x0f0f0f0f0f0f0f0f) | avx2::unrank_corner_orient(corient);
	return _mm256_insert_epi64(v, u, 2);
}

template<int n>
static auto get(__m256i v) {
	std::array<uint8_t, 16> arr;
	__m128i w = _mm256_extracti128_si256(v, n);
	_mm_storeu_si128((__m128i *) &arr[0], w);
	return arr;
}

template<int n>
static auto set(__m256i v, const std::array<uint8_t, 16> &a) {
	__m128i w = _mm_loadu_si128((__m128i *) &a[0]);
	return _mm256_inserti128_si256(v, w, n);
}

cube::edge_array_t cube::getEdges() const {
	edge_array_t edges;
	auto a = get<0>(v);
	std::copy(&a[0], &a[12], &edges[0]);
	return edges;
}

cube cube::setEdges(const cube::edge_array_t &edges) const {
	std::array<uint8_t, 16> a;
	std::copy(&edges[0], &edges[12], &a[0]);
	std::iota(&a[12], &a[16], 12);
	return set<0>(v, a);
}

cube::corner_array_t cube::getCorners() const {
	corner_array_t corners;
	auto a = get<1>(v);
	std::copy(&a[0], &a[8], &corners[0]);
	return corners;
}

cube cube::setCorners(const cube::corner_array_t &corners) const {
	std::array<uint8_t, 16> a;
	std::copy(&corners[0], &corners[8], &a[0]);
	std::iota(&a[8], &a[16], 8);
	return set<1>(v, a);
}
