#ifndef INVL_CCOORD_H
#define INVL_CCOORD_H

#include "cperm16.h"
#include "orient.h"

class ccoord {
	cperm16 cp;
	corient co;
    public:
	ccoord() : cp(), co() { }
	ccoord(cperm16 cp, corient co) : cp(cp), co(co) { }

	ccoord(const cube &c) {
		cp = c;
		co = corient{c.getCornerOrient()}.symi(cp.sym());
	}

	operator cube() const {
		return cube{}
			.setCornerPerm(cp.rep())
			.setCornerOrient(co)
			.sym(cp.sym());
	}

	bool is_solved() const {
		return co == 0 && cp.index() == 0;
	}

	ccoord normalize() const {
		return ccoord(cp.index(), co);
	}

	auto coord() const {
		return std::make_pair(cp.index(), co);
	}

	size_t prune_idx() const {
		auto [ cp, co ] = coord();
		return cp * N_CORIENT + co;
	}

	auto real() const {
		return std::make_pair(cp.real(), co.symi(sym::inv(cp.sym())));
	}

	ccoord move(int m) const {
		ccoord cc;
		uint8_t s;
		std::tie(cc.cp, m, s) = cp.move_ex(m);
		cc.co = co.movei(m, s);
		return cc;
	}
};

#endif
