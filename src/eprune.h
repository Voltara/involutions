#ifndef INVL_EPRUNE_H
#define INVL_EPRUNE_H

#include <cstdint>
#include <vector>
#include "ecoord.h"
#include "bits.h"

constexpr size_t N_EPRUNE = N_EPERM48 * N_EORIENT;

class eprune {
    private:
	static constexpr size_t CL = 14;
	static constexpr size_t STRIPE = 147;

	struct cache_line_t {
		uint32_t up_low;
		uint32_t down_low;
		uint16_t up[CL];
		uint16_t down[CL];

		uint32_t get_up(int i) const {
			uint32_t b = up_low >> (i * 2);
			return (b & 3) | uint32_t(up[i]) << 2;
		}

		uint32_t get_down(int i) const {
			uint32_t b = down_low >> (i * 2);
			return (b & 3) | uint32_t(down[i]) << 2;
		}

		void set_up(int i, int m) {
			m = 1 << m;
			if (m <= 0b10) {
				up_low |= m << (i * 2);
			} else {
				up[i] |= m >> 2;
			}
		}

		void set_down(int i, int m) {
			m = 1 << m;
			if (m <= 0b10) {
				down_low |= m << (i * 2);
			} else {
				down[i] |= m >> 2;
			}
		}
	};

    public:
	static void init();
	static void free();

	struct rec_t {
		uint32_t up;
		uint32_t down;
	};

	static rec_t lookup(size_t ep, size_t eo) {
		auto cl = &index[ep * STRIPE + eo / CL];
		auto sub = eo % CL;
		return rec_t{cl->get_up(sub), cl->get_down(sub)};
	}

	static rec_t lookup(ecoord ec) {
		auto [ ep, eo ] = ec.coord();
		return lookup(ep, eo);
	}

	static int probe(ecoord ec) {
		int depth = 0;
		while (!ec.is_solved()) {
			depth++;
			ec = ec.normalize();
			int m = *bits(lookup(ec).down);
			ec = ec.move(m);
		}
		return depth;
	}

    private:
	static void generate();

	inline static cache_line_t *index = NULL;
	static constexpr uint32_t SHM_KEY = 0x6e727065;
	static constexpr uint64_t MAGIC = 0x42d8375fde5b9c8b;
};

#endif
