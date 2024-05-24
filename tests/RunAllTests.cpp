#include <CppUTest/CommandLineTestRunner.h>
#include "test_util.h"
#include "ecsolver.h"
#include "involution.h"
#include "tracker.h"

decltype(t::rng) t::rng;

int main(int argc, char **argv) {
	t::init();

	// cpputest leak detector is not thread safe
	MemoryLeakWarningPlugin::saveAndDisableNewDeleteOverloads();

	ecsolver::init();
	involution::init();
	tracker::init();

	return CommandLineTestRunner::RunAllTests(argc, argv);
}
