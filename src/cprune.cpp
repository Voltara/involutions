#include "cprune.h"
#include "bits.h"
#include "alloc.h"

void cprune::init() {
	static std::once_flag flag;
	std::call_once(flag, [&]() {
		cperm16::init();
		corient::init();
	});
}

static int set(uint8_t *table, ccoord cc, int depth) {
	auto idx = cc.prune_idx();
	if (table[idx] == 0xff) {
		table[idx] = depth;
		return 1;
	}
	return 0;
};

static int set_sym(uint8_t *table, ccoord cc, int depth) {
	int found = set(table, cc, depth);
	if (found) {
		auto [ cp, co ] = cc.coord();
		for (auto s : bits(cperm16::selfsym(cp))) {
			found += set(table, ccoord{cp, co.symi(s)}, depth);
		}
	}
	return found;
};

template<bool REVERSE>
static int generate_depth(uint8_t *table, int depth) {
	const uint8_t want = REVERSE ? 0xff : depth;
	const uint8_t neighbor = REVERSE ? depth : 0xff;

	int found = 0;

	for (int idx = 0; idx < N_CPRUNE; idx++) {
		if (table[idx] != want) continue;
		ccoord cc(idx / N_CORIENT, idx % N_CORIENT);
		for (int m = 0; m < N_MOVES; m++) {
			auto cc_m = cc.move(m);
			if (table[cc_m.prune_idx()] != neighbor) continue;
			found += set_sym(table, REVERSE ? cc : cc_m, depth + 1);
			if (REVERSE) break;
		}
	}

	return found;
}

cprune::~cprune() {
	if (table) {
		std::unique_lock lock(mtx);
		free_list.push_back(table);
		table = NULL;
	}
}

void cprune::generate(const std::vector<ccoord> &seed) {
	if (!table) {
		std::unique_lock lock(mtx);
		if (free_list.empty()) {
			table = alloc::huge<uint8_t>(N_CPRUNE);
		} else {
			table = free_list.back();
			free_list.pop_back();
		}
	}

	std::fill(&table[0], &table[N_CPRUNE], 0xff);

	size_t todo = N_CPRUNE;

	for (auto cc : seed) {
		todo -= set_sym(table, cc, 0);
	}

	for (max_prune = 0; todo; max_prune++) {
		if (todo < N_CPRUNE / 2) {
			todo -= generate_depth<true>(table, max_prune);
		} else {
			todo -= generate_depth<false>(table, max_prune);
		}
	}
}
