#ifndef INVL_ECOORD_H
#define INVL_ECOORD_H

#include "eperm48.h"
#include "orient.h"

class ecoord {
	eperm48 ep;
	eorient eo;
    public:
	ecoord() : ep(), eo() { }
	ecoord(eperm48 ep, eorient eo) : ep(ep), eo(eo) { }

	ecoord(const cube &c) {
		ep = c;
		eo = eorient{c.getEdgeOrient()}.symi(ep.sym());
	}

	operator cube() const {
		return cube{}
			.setEdgePerm(ep.rep())
			.setEdgeOrient(eo)
			.sym(ep.sym());
	}

	bool is_solved() const {
		return eo == 0 && ep.index() == 0;
	}

	ecoord normalize() const {
		return ecoord(ep.index(), eo);
	}

	auto coord() const {
		return std::make_pair(ep.index(), eo);
	}

	uint64_t selfsym() const {
		return eperm48::selfsym(ep.index()) & eo.selfsym();
	}

	size_t prune_idx() const {
		auto [ ep, eo ] = coord();
		return size_t(ep) * N_EORIENT + eo;
	}

	ecoord move(int m) const {
		ecoord ec;
		uint8_t s;
		std::tie(ec.ep, m, s) = ep.move_ex(m);
		ec.eo = eo.movei(m, s);
		return ec;
	}

	int sym() const {
		return ep.sym();
	}
};

#endif
