#ifndef INVL_ECSOLVER_H
#define INVL_ECSOLVER_H

#include "eprune.h"
#include "tracker.h"

class ecsolver {
    public:
	static void init();

	void solve(tracker::handle &handle, int depth);
};

#endif
