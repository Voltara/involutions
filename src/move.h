#ifndef INVL_MOVE_H
#define INVL_MOVE_H

class move {
    public:
	static int inv(int m) {
		constexpr int lookup = 0444444;
		int delta = (lookup >> m) & 0b110;
		return m + delta - 2;
	}
};

#endif
