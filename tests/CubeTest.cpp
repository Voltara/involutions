#include <numeric>
#include <set>
#include <algorithm>
#include <CppUTest/TestHarness.h>
#include "test_util.h"
#include "cube.h"

TEST_GROUP(Cube) {
};

TEST(Cube, Identity) {
	const cube c;
	LONGS_EQUAL(0, c.getEdgePerm());
	LONGS_EQUAL(0, c.getEdgeOrient());
	LONGS_EQUAL(0, c.getCornerPerm());
	LONGS_EQUAL(0, c.getCornerOrient());
}

TEST(Cube, Moves) {
	const cube c;
	for (int m = 0; m < N_MOVES; m += 3) {
		CHECK(c.move(m) != cube{});
		CHECK(c.move(m + 1) == c.move(m) * c.move(m));
		CHECK(c.move(m + 2) == c.move(m) * c.move(m) * c.move(m));
		CHECK(cube{} == c.move(m) * c.move(m) * c.move(m) * c.move(m));
	}
	CHECK(cube::from_moves("U1") == cube::from_moves("R1L3F2B2R1L3D1L1R3B2F2L1R3"));
}

TEST(Cube, Premove) {
	cube c;

	auto moves = t::random_moves(20);
	for (auto m : moves) {
		c = c.premove(m);
	}
	std::reverse(moves.begin(), moves.end());

	CHECK(c == cube::from_moveseq(moves));
}

TEST(Cube, QuarterTurnParity) {
	for (int m = 0; m < N_MOVES; m += 3) {
		int n_flips = std::popcount(cube{}.move(m).getEdgeOrientRaw());
		CHECK(n_flips == 4);
	}
}

TEST(Cube, EdgePerm) {
	CHECK(cube{}.setEdgePerm(N_EPERM - 1).getEdgePerm() == N_EPERM - 1);
	CHECK(t::random_cube().setEdgePerm(42).getEdgeOrient() == 0);

	cube c = t::random_cube().setEdgeOrient(0);
	CHECK(c.setEdgePerm(c.getEdgePerm()) == c);
}

TEST(Cube, CornerPerm) {
	CHECK(cube{}.setCornerPerm(N_CPERM - 1).getCornerPerm() == N_CPERM - 1);
	CHECK(t::random_cube().setCornerPerm(42).getCornerOrient() == 0);

	cube c = t::random_cube().setCornerOrient(0);
	CHECK(c.setCornerPerm(c.getCornerPerm()) == c);
}

TEST(Cube, EdgeOrient) {
	for (int eo = 0; eo < N_EORIENT; eo++) {
		CHECK(cube{}.setEdgeOrient(eo).getEdgeOrient() == eo);
	}
	const cube c = t::random_cube();
	CHECK(c.setEdgeOrient(c.getEdgeOrient()) == c);
}

TEST(Cube, CornerOrient) {
	for (int co = 0; co < N_CORIENT; co++) {
		CHECK(cube{}.setCornerOrient(co).getCornerOrient() == co);
	}
	const cube c = t::random_cube();
	CHECK(c.setCornerOrient(c.getCornerOrient()) == c);
}

TEST(Cube, Parity) {
	CHECK_EQUAL(cube{}.parity(), false);

	for (int m = 0; m < N_MOVES; m++) {
		CHECK_FALSE(cube{}.move(m).parity());
		bool expected = (m % 3) != 1;
		CHECK_EQUAL(cube{}.move(m).setCornerPerm(0).parity(), expected);
		CHECK_EQUAL(cube{}.move(m).setEdgePerm(0).parity(), expected);
	}

	for (int i = 0; i < 100; i++) {
		cube c = t::random_cube();
		CHECK_EQUAL(c.setEdgePerm(0).parity(), c.setCornerPerm(0).parity());
	}
}

TEST(Cube, AlternatingParity) {
	for (int i = 0; i < 100; i++) {
		cube c = t::random_cube();
		CHECK_EQUAL(true, c.setEdgePerm(c.getEdgePerm() ^ 1).parity());
		CHECK_EQUAL(true, c.setCornerPerm(c.getCornerPerm() ^ 1).parity());
	}
}

TEST(Cube, Equality) {
	auto test = [](cube c, cube d, bool are_equal) {
		CHECK_EQUAL(c == d, are_equal);
		CHECK_EQUAL(c != d, !are_equal);
	};
	test(cube{}, cube{}, true);
	test(cube{}, cube{}.setEdgeOrient(1), false);
	test(cube{}, cube{}.setEdgePerm(1), false);
	test(cube{}, cube{}.setCornerOrient(1), false);
	test(cube{}, cube{}.setCornerPerm(1), false);
}

TEST(Cube, LessThan) {
	cube c, d = t::random_cube();
	for (int i = 0; i < 100; i++) {
		c = d;
		do { d = t::random_cube(); } while (c == d);
		CHECK_TRUE((c < d) ^ (d < c));
	}
}

TEST(Cube, Invert) {
	CHECK(cube{} == ~cube{});
	for (int i = 0; i < 100; i++) {
		cube c = t::random_cube();
		CHECK(c * ~c == cube{});
	}
}

TEST(Cube, Symmetry) {
	cube c = t::random_cube();
	CHECK(c == c.sym(0));

	for (int s = 0; s < N_SYM48; s++) {
		CHECK(c == c.sym(s).symi(s));
	}

	auto self_sym = [](cube c) {
		std::set<cube> sym;
		for (int s = 0; s < N_SYM48; s++) {
			sym.insert(c.sym(s));
		}
		return sym.size();
	};

	CHECK_EQUAL(self_sym(cube{}), 1);
	CHECK_EQUAL(self_sym(cube::from_moves("U2")), 6);
	CHECK_EQUAL(self_sym(cube::from_moves("U1")), 12);
	CHECK_EQUAL(self_sym(cube::from_moves("U2R2")), 24);
	CHECK_EQUAL(self_sym(cube::from_moves("U1R1")), 48);
}

TEST(Cube, MoveParse) {
	cube superflip = cube{}.setEdgeOrient(N_EORIENT - 1);
	CHECK(cube::from_moves("U R2 F B R B2 R U2 L B2 R U' D' R2 F R' L B2 U2 F2\n") == superflip);
	CHECK(cube::from_moves("U1R2F1B1R1B2R1U2L1B2R1U3D3R2F1R3L1B2U2F2") == superflip);
}

TEST(Cube, NumericMoves) {
	CHECK(cube::from_moveseq({}) == cube());
	CHECK(cube::from_moveseq({0,3,6,9,12,15,1,4,7,10,13,16,2,5,8,11,14,17}) == cube::from_moves("URFDLBU2R2F2D2L2B2U'R'F'D'L'B'"));
}

TEST(Cube, Rep16) {
	cube c = t::random_cube();
	cube r = c.symRep16();
	for (int s = 0; s < N_SYM16; s++) {
		CHECK_FALSE(c.sym(s) < r);
	}
}

TEST(Cube, Rep48) {
	cube c = t::random_cube();
	cube r = c.symRep48();
	for (int s = 0; s < N_SYM48; s++) {
		CHECK_FALSE(c.sym(s) < r);
	}
}

TEST(Cube, GetSet) {
	cube c = t::random_cube();

	cube edges = cube{}.setEdges(c.getEdges());
	CHECK(edges == c.setCornerPerm(0));

	cube corners = cube{}.setCorners(c.getCorners());
	CHECK(corners == c.setEdgePerm(0));

	CHECK(c == edges.setCorners(corners.getCorners()));
	CHECK(c == corners.setEdges(edges.getEdges()));
}

TEST(Cube, ToSingmasterReid) {
	// identity
	CHECK_EQUAL(
		std::string("UF UR UB UL DF DR DB DL FR FL BR BL UFR URB UBL ULF DRF DFL DLB DBR"),
		cube{}.to_sing());

	// cube within cube
	CHECK_EQUAL(
		std::string("UF UR FL FD BR BU DB DL FR RD LU BL UFR FUL FLD FDR BUR BRD DLB BLU"),
		cube::from_moves("F1L1F1U3R1U1F2L2U3L3B1D3B3L2U1").to_sing());

	// random twenty #1
	CHECK_EQUAL(
		std::string("FD BU DR LD RU LU BD UF LB RB LF RF RBU LBD FLD FDR BLU RDB RUF FUL"),
		cube::from_moves("F1R3F3D3L1U3R3B1U2D1R3B3U2F3L2U1R2U2L2D1").to_sing());

	// random twenty #2
	CHECK_EQUAL(
		std::string("RU RD FU BD LD UL BL FD LF RB RF BU FLD LFU BLU RFD RDB BDL RUF BUR"),
		cube::from_moves("R3D2R3D2B2L1F2B3D1R1D2F3D2B3L1B1R2U3F3U3").to_sing());

	// random twenty #3
	CHECK_EQUAL(
		std::string("FD FU FR RD LD RU FL BU RB BD LB LU RFD RBU LUB DBR LFU DLB LDF RUF"),
		cube::from_moves("R1B2R2B2U2R1F2L3R3B3U1F1D1L3F2L3R3F3L3F1").to_sing());
}
