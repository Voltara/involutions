#include <iostream>
#include <filesystem>
#include <sys/file.h>
#include "tracker.h"
#include "involution.h"
#include "eprune.h"
#include "thread.h"
#include "tables.h"
#include "corner_hash.h"
#include "alloc.h"

void tracker::init() {
	static std::once_flag flag;
	std::call_once(flag, [&]() {
		involution::init();

		ec_mutex = new std::mutex[N_EDGE_INVO]{};
		corner_sets = new std::bitset<65536>[N_EDGE_INVO]{};
		is_initialized = new uint8_t[N_EDGE_INVO]{};

		auto corners = involution::corners();
		for (int parity = 0; parity < 2; parity++) {
			auto corner_set = &corner_init[parity];
			for (auto cc : corners[parity]) {
				corner_set->set(corner_hash(cc));
			}
		}
	});
}

static size_t file_size() {
	size_t head = N_EDGE_INVO * sizeof(tracker::header);
	size_t solves = N_INVO * sizeof(tracker::solution);
	return head + solves;
}

void tracker::create() {
	eprune::init();

	std::string path = TRACKER_FILE, tmp = path + ".tmp";

	if (tables::exists(path)) {
		std::cerr << "Involution database already exists!\n";
		abort();
	}

	auto edges = involution::edges();

	std::vector<header> head(N_EDGE_INVO);

	uint32_t offset = 0, idx = 0;

	for (int parity = 0; parity < 2; parity++) {
		for (auto ec : edges[parity]) {
			auto &h = head[idx++];
			auto [ ep, eo ] = ec.coord();

			int prune = eprune::probe(ec);

			h.offset = offset;
			h.ep = ep;
			h.eo = eo;
			h.n_cubes = involution::cube_count(ec);
			h.n_solved = 0;
			h.n_length = {};
			h._reserved0 = 0;
			h.proven_min = prune;
			h.parity = parity;
			h.prune = prune;
			h._reserved1 = 0;

			offset += h.n_cubes;
		}
	}

	if (!tables::save(tmp, &head[0], N_EDGE_INVO)) {
		std::cerr << "Error writing header to involution database.\n";
		abort();
	}
	std::filesystem::resize_file(tables::full_path(tmp), file_size());
	std::filesystem::rename(tables::full_path(tmp), tables::full_path(path));
}

bool tracker::open() {
	std::string path = tables::full_path(TRACKER_FILE);
	auto [ mem, fd_ ] = alloc::mmap_file<uint8_t>(file_size(), path);
	if (!mem) return false;
	fd = fd_;
	head = (header *) &mem[0];
	sol = (solution *) &head[N_EDGE_INVO];
	return true;
}

void tracker::lock() {
	if (flock(fd, LOCK_EX | LOCK_NB) != 0) {
		perror("flock(LOCK_EX)");
		abort();
	}
}

void tracker::unlock() {
	if (flock(fd, LOCK_UN) != 0) {
		perror("flock(LOCK_UN)");
		abort();
	}
}

void tracker::reset(int idx) {
	auto &h = head[idx];
	h.n_solved = 0;
	h.n_length = {};
	h.proven_min = h.prune;

	auto s = &sol[h.offset];
	for (int i = 0; i < h.n_cubes; i++) {
		s[i] = {};
	}

	is_initialized[idx] = false;
	corner_sets[idx] = {};
}

std::vector<moveseq> tracker::get_solutions(int idx) {
	std::vector<moveseq> solutions;
	auto &h = head[idx];
	uint32_t n_solved = h.n_solved, offset = h.offset;
	for (int i = 0; i < n_solved; i++) {
		solutions.push_back(sol[offset++]);
	}
	return solutions;
}

std::bitset<65536> * tracker::get_corner_set(int idx) {
	auto corner_set = &corner_sets[idx];
	if (!is_initialized[idx]) {
		auto &h = head[idx];
		*corner_set = corner_init[h.parity];
		auto self_sym = ecoord(h.ep, h.eo).selfsym();
		for (const auto &moves : get_solutions(idx)) {
			cube c = cube::from_moveseq(moves);
			corner_set->reset(corner_hash(c));
			for (auto s : bits(self_sym)) {
				auto c_s = c.symi(s);
				auto h_s = corner_hash(c_s);
				corner_set->reset(h_s);
			}
		}
		is_initialized[idx] = 1;
	}
	return corner_set;
}

tracker::solution::solution(moveseq seq) : solution() {
	if (seq.size() > MAX_DEPTH) abort();

	seq = seq.canonical();

	info = 0b10000000 | seq.size();

	for (int i = 0; i < seq.size(); i++) {
		uint8_t m = seq[i];
		if (i == 0) {
			info |= (m & 1) << 6;
			m >>= 1;
		} else if (seq[i - 1] < m) {
			m -= 3;
		}
		if (i % 2 == 0) {
			m <<= 4;
		}
		moves[i / 2] |= m;
	}
}

tracker::solution::operator moveseq () const {
	int len = length();
	moveseq seq;
	uint8_t last_axis = 0xff;
	for (int i = 0; i < len; i++) {
		uint8_t m = moves[i / 2];
		if (i % 2 == 0) {
			m >>= 4;
		} else {
			m &= 0b00001111;
		}
		if (i == 0) {
			m = ((info >> 6) & 1) | (m << 1);
		}
		uint8_t axis = m / 3;
		if (last_axis <= axis) {
			m += 3;
			axis++;
		}
		last_axis = axis;
		seq.push_back(m);
	}
	return seq;
}

bool tracker::handle::solution(const moveseq &seq) {
	cube c = cube::from_moveseq(seq);
	return solution(seq, c, corner_hash(c));
}

bool tracker::handle::solution(moveseq seq, cube c, int hash) {
	if (!corner_set->test(hash)) {
		return false;
	}

	corner_set->reset(hash);
	todo--;

	for (auto s : bits(self_sym)) {
		auto h_s = corner_hash(c.symi(s));
		if (corner_set->test(h_s)) {
			corner_set->reset(h_s);
			todo--;
		}
	}

	seq = seq.canonical();
	sol[h->offset + h->n_solved] = seq;
	h->n_solved++;
	h->n_length[seq.size()]++;

	return true;
}
