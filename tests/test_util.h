#include <random>
#include "cube.h"

struct t {
    public:
	static void init() {
		rng.seed(rng.default_seed);
	}

	static int rand(int max) {
		std::uniform_int_distribution<> dis(0, max - 1);
		return dis(rng);
	}

	static cube random_cube() {
		cube c;
		c = c.setEdgePerm(rand(N_EPERM));
		do {
			c = c.setCornerPerm(rand(N_CPERM));
		} while (c.parity());
		c = c.setEdgeOrient(rand(N_EORIENT));
		c = c.setCornerOrient(rand(N_CORIENT));
		return c;
	}

	static moveseq random_moves(int count) {
		moveseq moves;
		for (int i = 0; i < count; i++) {
			moves.push_back(rand(N_MOVES));
		}
		return moves;
	}

    private:
	static std::mt19937 rng;
};
