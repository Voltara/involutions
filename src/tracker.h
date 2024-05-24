#ifndef INVL_TRACKER_H
#define INVL_TRACKER_H

#include <cstdint>
#include <array>
#include <bitset>
#include "ccoord.h"
#include "corner_hash.h"
#include "ecoord.h"
#include "moveseq.h"
#include "thread.h"

constexpr size_t MAX_DEPTH = 20;

class tracker {
	static constexpr char TRACKER_FILE[] = "invo.dat";
    public:
	struct header {
		uint32_t offset;
		uint32_t ep;
		uint16_t eo;
		uint16_t n_cubes;
		uint16_t n_solved;
		std::array<uint16_t, MAX_DEPTH + 1> n_length;
		uint32_t _reserved0;
		uint8_t proven_min;
		uint8_t parity;
		uint8_t prune;
		uint8_t _reserved1;

		ecoord get_ec() const {
			return ecoord{ep, eo};
		}
	};

	struct solution {
		uint8_t info;
		uint8_t moves[10];

		solution() : info(), moves() {
		}

		solution(moveseq);

		operator moveseq () const;

		int length() const {
			return (info & 0b00011111);
		}

		bool valid() const {
			return (info & 0b10000000) != 0;
		}
	};

	struct handle {
		uint64_t self_sym;
		const int idx;
		std::bitset<65536> *corner_set;
		int todo;
		header *h;

		handle(int idx) : idx(idx) {
			ec_mutex[idx].lock();
			corner_set = get_corner_set(idx);
			todo = corner_set->count();
			h = &head[idx];
			self_sym = h->get_ec().selfsym();
		}

		~handle() {
			ec_mutex[idx].unlock();
		}

		bool solution(const moveseq &);

		bool is_unsolved(ccoord cc) {
			return corner_set->test(corner_hash(cc));
		}

		void update_proven_min(int depth) {
			if (h->proven_min < depth) {
				h->proven_min = depth;
			}
		}

		int proven_min() const {
			return h->proven_min;
		}

		bool parity() const {
			return h->parity;
		}

		ecoord ec() const {
			return ecoord{h->ep, h->eo};
		}

		int n_solved() const {
			return h->n_solved;
		}

	    private:
		bool solution(moveseq, cube, int hash);
	};

	static void init();
	static void create();
	static bool open();
	static void lock();
	static void unlock();
	static void reset(int idx);

	static void lock(int idx) { ec_mutex[idx].lock(); }
	static void unlock(int idx) { ec_mutex[idx].unlock(); }

	static std::vector<moveseq> get_solutions(int idx);

	static header get_header(int idx) {
		return head[idx];
	}

    private:
	static std::bitset<65536> * get_corner_set(int idx);

	inline static int fd = -1;
	inline static header *head = NULL;
	inline static solution *sol = NULL;
	inline static std::mutex *ec_mutex = NULL;
	inline static std::bitset<65536> *corner_sets = NULL;
	inline static uint8_t *is_initialized = NULL;

	inline static std::array<std::bitset<65536>, 2> corner_init = {};
};

#endif
