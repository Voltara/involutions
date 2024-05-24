#ifndef INVL_CPRUNE_H
#define INVL_CPRUNE_H

#include <cstdint>
#include <vector>
#include "ccoord.h"
#include "thread.h"

constexpr size_t N_CPRUNE = N_CPERM16 * N_CORIENT;

class cprune {
	uint8_t *table = NULL;
	int max_prune = 0;

	cprune(const cprune &) = delete;

    public:
	static void init();

	cprune() { }
	~cprune();

	void generate(const std::vector<ccoord> &seed);

	int prune(const ccoord &cc) const {
		return table[cc.prune_idx()];
	}

	bool prune(const ccoord &cc, int depth) const {
		return (depth < max_prune) && (depth < prune(cc));
	}

    private:
	inline static std::vector<uint8_t *> free_list;
	inline static std::mutex mtx;
};

#endif
