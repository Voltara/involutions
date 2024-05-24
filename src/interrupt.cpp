#include <signal.h>
#include "interrupt.h"

void interrupt::sig_terminate(int) {
	terminated_ = true;
}

void interrupt::setup_signals() {
	struct sigaction sa = {};
	sa.sa_handler = sig_terminate;
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
}
