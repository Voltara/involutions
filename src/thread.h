#ifndef INVL_THREAD_H
#define INVL_THREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <barrier>
#include <vector>

static const auto N_WORKERS = std::thread::hardware_concurrency();

template<typename F>
class parallel {
	std::vector<std::thread> workers;

    public:
	parallel(F fn) {
		for (int i = 0; i < N_WORKERS; i++) {
			workers.emplace_back(fn, i);
		}
	}

	~parallel() {
		join();
	}

	void join() {
		for (auto &w : workers) {
			w.join();
		}
		workers.clear();
	}
};

#endif
