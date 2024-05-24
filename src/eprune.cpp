#include <iostream>
#include "eprune.h"
#include "bits.h"
#include "thread.h"
#include "alloc.h"

class eprune_generator {
	std::barrier<> *barrier;
	std::vector<std::mutex> ep_mutex;
	std::mutex mtx;
	uint8_t *table;
	size_t todo;

    public:
	eprune_generator() : ep_mutex(N_EPERM48) {
		barrier = new std::barrier(N_WORKERS);
	}

	~eprune_generator() {
		delete barrier;
	}

	size_t set(ecoord ec, int depth) {
		auto idx = ec.prune_idx();
		if (table[idx] == 0xff) {
			table[idx] = depth;
			return 1;
		}
		return 0;
	}

	size_t set_sym(ecoord ec, int depth) {
		size_t found = set(ec, depth);
		if (found) {
			auto [ ep, eo ] = ec.coord();
			for (auto s : bits(eperm48::selfsym(ep))) {
				found += set(ecoord{ep, eo.symi(s)}, depth);
			}
		}
		return found;
	};

	template<bool REVERSE>
	size_t scan(size_t ep_start, size_t ep_end, int depth) {
		const uint8_t want = REVERSE ? 0xff : depth;
		const uint8_t neighbor = REVERSE ? depth : 0xff;

		size_t found = 0;
		std::vector<eorient> active;

		for (auto ep = ep_start; ep < ep_end; ep++) {
			auto idx_base = ep * N_EORIENT;

			active.clear();
			for (int eo = 0; eo < N_EORIENT; eo++) {
				if (table[idx_base + eo] == want) {
					active.push_back(eo);
				}
			}

			for (int m = 0; m < N_MOVES && !active.empty(); m++) {
				auto [ ep_m, s_m ] = eperm48::raw_move(ep, m);

				if (!REVERSE) ep_mutex[ep_m].lock();

				for (auto eo : active) {
					if (table[idx_base + eo] != want) continue;

					auto eo_m = eo.movei(m, s_m);
					if (table[ep_m * N_EORIENT + eo_m] != neighbor) continue;

					auto ec = REVERSE ? ecoord(ep, eo) : ecoord(ep_m, eo_m);
					found += set_sym(ec, depth + 1);
				}

				if (!REVERSE) ep_mutex[ep_m].unlock();
			}
		}

		return found;
	}

	void worker(size_t id) {
		size_t block = N_EPERM48 / N_WORKERS;
		size_t ep_start = block * id, ep_end = ep_start + block;
		if (id == N_WORKERS - 1) ep_end = N_EPERM48;

		std::fill(&table[ep_start * N_EORIENT], &table[ep_end * N_EORIENT], 0xff);
		if (!id) table[0] = 0;

		for (int depth = 0; ; depth++) {
			barrier->arrive_and_wait();
			if (!todo) break;
			bool reverse = (todo < N_EPRUNE / 2);

			barrier->arrive_and_wait();
			size_t found;
			if (reverse) {
				found = scan<true>(ep_start, ep_end, depth);
			} else {
				found = scan<false>(ep_start, ep_end, depth);
			}

			std::unique_lock lock(mtx);
			todo -= found;
		}
	}

	const uint8_t * generate() {
		table = new uint8_t[N_EPRUNE];
		todo = N_EPRUNE - 1;

		parallel workers([this](size_t id) { worker(id); });
		workers.join();

		return table;
	}
};

void eprune::init() {
	static std::once_flag flag;
	constexpr size_t N_CACHE_LINE = N_EPERM48 * STRIPE;

	std::call_once(flag, [&]() {
		eperm48::init();
		eorient::init();

		index = alloc::shared<cache_line_t>(N_CACHE_LINE + 1, SHM_KEY);
		if (!index) {
			std::cerr << "error getting shared memory for eprunei table\n";
			abort();
		}
		auto &magic = *(uint64_t *) &index[N_CACHE_LINE];
		if (magic != MAGIC) {
			std::cerr << "generating eprunei table\n";
			generate();
			magic = MAGIC;
		}
	});
}

void eprune::free() {
	alloc::shared_free(SHM_KEY);
}

void eprune::generate() {
	const auto table = eprune_generator{}.generate();

	parallel workers([table](size_t id) {
		size_t block = N_EPERM48 / N_WORKERS;
		size_t ep_start = block * id, ep_end = ep_start + block;
		if (id == N_WORKERS - 1) ep_end = N_EPERM48;

		// in case of partial failed load
		std::fill(&index[ep_start * STRIPE], &index[ep_end * STRIPE], cache_line_t{});

		for (auto ep = ep_start; ep < ep_end; ep++) {
			auto cl_base = ep * STRIPE;

			for (int m = 0; m < N_MOVES; m++) {
				auto [ ep_m, s_m ] = eperm48::raw_move(ep, m);

				size_t sub = -1;
				auto cl = &index[cl_base];

				for (int eo = 0; eo < N_EORIENT; eo++) {
					if (++sub == CL) {
						sub = 0;
						cl++;
					}

					auto eo_m = eorient(eo).movei(m, s_m);

					auto p0 = table[ep * N_EORIENT + eo];
					auto p1 = table[ep_m * N_EORIENT + eo_m];

					if (p0 < p1) {
						cl->set_up(sub, m);
					} else if (p0 > p1) {
						cl->set_down(sub, m);
					}
				}
			}
		}
	});

	workers.join();

	delete table;
}
