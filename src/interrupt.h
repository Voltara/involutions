#ifndef INVL_INTERRUPT_H
#define INVL_INTERRUPT_H

class interrupt {
    public:
	static bool terminated() {
		return terminated_;
	}

	static void setup_signals();

    private:
	inline static bool terminated_ = false;
	static void sig_terminate(int);
};

#endif
