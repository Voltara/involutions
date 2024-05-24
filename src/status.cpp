#include <iostream>
#include <format>
#include <chrono>
#include "status.h"
#include "tracker.h"
#include "involution.h"

using namespace std::chrono_literals;

void status::show_counts() {
	static const std::vector<size_t> expected = {
		1, 1, 1, 2, 4, 25, 41, 292, 506, 3501, 7741, 45543,
		146698, 700019, 3500419, 19478862, 130385528, 778842829,
		2184417694, 445145591, 10842
	};

	size_t counts[MAX_DEPTH + 1] = {};

	for (int idx = 0; idx < N_EDGE_INVO; idx++) {
		auto h = tracker::get_header(idx);
		for (int d = 0; d <= MAX_DEPTH; d++) {
			counts[d] += h.n_length[d];
		}
	}

	for (int d = 0; d <= MAX_DEPTH; d++) {
		std::cout << std::format(
			"{:2} {:10} / {}\n", d, counts[d], expected[d]);
	}
}

void status::status_thread() {
	std::unique_lock lock(mtx);

	auto show_status = [&]() {
		size_t idx = progress;
		lock.unlock();
		std::cout << std::format("\nCosets: {}/{}\n", idx, total);
		show_counts();
		lock.lock();
	};

	while (!done) {
		show_status();
		if (!done) {
			cv_stop.wait_for(lock, 5000ms);
		}
	}

	show_status();
}

void status::stop() {
	if (!done) {
		done = true;
		cv_stop.notify_all();
		worker.join();
	}
}
