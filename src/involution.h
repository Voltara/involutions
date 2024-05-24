#ifndef INVL_INVOLUTION_H
#define INVL_INVOLUTION_H

#include <array>
#include <vector>
#include "ccoord.h"
#include "ecoord.h"

// symmetry reduced
constexpr size_t N_INVO = 3562686140;
constexpr size_t N_EDGE_INVO = 336004;

class involution {
    public:
	static void init();

	static const std::array<std::vector<ecoord>, 2> & edges() {
		return edges_;
	}

	static const std::array<std::vector<ccoord>, 2> & corners() {
		return corners_;
	}

	static size_t cube_count(ecoord ec);

    private:
	static inline std::array<std::vector<ecoord>, 2> edges_ = { };
	static inline std::array<std::vector<ccoord>, 2> corners_ = { };
};

#endif
