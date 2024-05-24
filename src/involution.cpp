#include <bitset>
#include <map>
#include "involution.h"
#include "bits.h"
#include "thread.h"

// hide the map from cpputest
static std::array<std::map<uint64_t, size_t>, 2> corner_ssym_count = { };

template<typename T>
static void invo_perms(bool parity, std::vector<T> &out, T arr, int idx = 0, int mask = 0) {
	while (idx < arr.size() && ((mask >> idx) & 1)) {
		idx++;
	}

	if (idx == arr.size()) {
		if (!parity) out.push_back(arr);
		return;
	}

	arr[idx] = idx;
	invo_perms(parity, out, arr, idx + 1, mask);

	for (int k = idx + 1; k < arr.size(); k++) {
		if ((mask >> k) & 1) continue;
		arr[idx] = k;
		arr[k] = idx;
		invo_perms(!parity, out, arr, idx + 1, mask | (1 << k));
	}
}

static void expand_ep(std::vector<ecoord> &out, cube c) {
	eperm48 ep = c;
	if (ep.sym() != 0) {
		return;
	}

	std::bitset<N_EORIENT> seen;

	for (eorient eo = 0; eo < N_EORIENT; eo = eo + 1) {
		if (seen.test(eo)) continue;

		c = c.setEdgeOrient(eo);
		if (c * c != cube{}) continue;
		out.emplace_back(c);

		for (auto s : bits(eperm48::selfsym(ep.index()))) {
			seen.set(eo.symi(s));
		}
	}
}

static void expand_cp(std::vector<ccoord> &out, cube c) {
	for (corient co = 0; co < N_CORIENT; co = co + 1) {
		c = c.setCornerOrient(co);
		if (c * c != cube{}) continue;
		out.emplace_back(c);
	}
}

static std::array<std::vector<ecoord>, 2> gen_edges() {
	std::array<std::vector<ecoord>, 2> invo = { };

	for (int parity = 0; parity < 2; parity++) {
		std::vector<cube::edge_array_t> perms;
		invo_perms(parity, perms, {});

		for (auto &perm : perms) {
			cube c = cube{}.setEdges(perm);
			expand_ep(invo[parity], c);
		}
	}

	return invo;
}

static std::array<std::vector<ccoord>, 2> gen_corners() {
	std::array<std::vector<ccoord>, 2> invo = { };

	for (int parity = 0; parity < 2; parity++) {
		std::vector<cube::corner_array_t> perms;
		invo_perms(parity, perms, {});

		for (auto &perm : perms) {
			cube c = cube{}.setCorners(perm);
			expand_cp(invo[parity], c);
		}
	}

	return invo;
}

void involution::init() {
	static std::once_flag flag;
	std::call_once(flag, [&]() {
		eperm48::init();
		cperm16::init();
		eorient::init();
		corient::init();

		edges_ = gen_edges();
		corners_ = gen_corners();

		for (int parity = 0; parity < 2; parity++) {
			for (cube c : corners_[parity]) {
				corner_ssym_count[parity][c.selfSym48()]++;
			}
		}
	});
}

size_t involution::cube_count(ecoord ec) {
	cube c = ec;
	if (c * c != cube{}) abort();

	auto esym = c.selfSym48();
	size_t count = 0;

	for (auto [ csym, n ] : corner_ssym_count[c.parity()]) {
		count += n * std::popcount(esym & csym);
	}

	return count / std::popcount(esym);
}
