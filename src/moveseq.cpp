#include "moveseq.h"

moveseq moveseq::parse(const std::string &s) {
	moveseq moves;

	int face = -1;
	for (auto ch : s) {
		int f = -1, power = -1;
		switch (ch) {
		    case 'u': case 'U': f =  0; break;
		    case 'r': case 'R': f =  3; break;
		    case 'f': case 'F': f =  6; break;
		    case 'd': case 'D': f =  9; break;
		    case 'l': case 'L': f = 12; break;
		    case 'b': case 'B': f = 15; break;
		    case '3': case '\'': power = 2; break;
		    case '2': power = 1; break;
		    case '1': power = 0; break;
		    default:  power = 0;
		}

		if (f != -1) {
			if (face != -1) {
				moves.push_back(face);
			}
			face = f;
		} else if (power != -1 && face != -1) {
			moves.push_back(face + power);
			face = -1;
		}
	}

	if (face != -1) {
		moves.push_back(face);
	}

	return moves;
}

moveseq moveseq::canonical() const {
	if (empty()) {
		return {};
	}

	moveseq canon = *this;

	canon.push_back(canon.back() + 3);

	uint8_t last_axis = 0, power[2] = { 0, 0 };
	auto tail = canon.begin();
	for (auto m : canon) {
		auto axis = (m / 3) % 3;
		if (axis != last_axis) {
			for (int pole = 0; pole < 2; pole++) {
				if (power[pole] %= 4) {
					*tail++ = (last_axis * 3 + pole * 9) + (power[pole] % 4) - 1;
				}
				power[pole] = 0;
			}
			last_axis = axis;
		}
		power[m >= 9] += (m % 3) + 1;
	}

	canon.resize(tail - canon.begin());

	return canon;
}

std::string moveseq::to_string() const {
	const char *face = "URFDLB";
	const char *power = "123";

	std::string s;
	for (auto m : *this) {
		s.push_back(face[m / 3]);
		s.push_back(power[m % 3]);
	}

	while (!s.empty() && s.back() == ' ') {
		s.pop_back();
	}

	return s;
}
