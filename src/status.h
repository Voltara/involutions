#ifndef INVL_STATUS_H
#define INVL_STATUS_H

#include "thread.h"

class status {
	std::mutex mtx;
	std::condition_variable cv_stop;
	bool done = false;
	size_t progress = 0, total = 0;
	std::thread worker;

    public:
	status(size_t total) : total(total) {
		worker = std::thread([this]() { status_thread(); });
	}

	~status() {
		stop();
	}

	void increment() {
		std::unique_lock lock(mtx);
		progress++;
	}

	void stop();

	static void show_counts();

    private:
	void status_thread();
};

#endif
