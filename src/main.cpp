#include <iostream>
#include <sstream>
#include <format>
#include <string>
#include <vector>
#include <map>
#include "ecsolver.h"
#include "neighborsolver.h"
#include "involution.h"
#include "tracker.h"
#include "interrupt.h"
#include "thread.h"
#include "status.h"

void cmd_help(const std::string &argv0);
void cmd_count();
void cmd_countall();
void cmd_solutions();
void cmd_create();
void cmd_ecoset(int depth);
void cmd_neighbor();
void cmd_unsolved();
void cmd_ingest(bool optimal);
void cmd_free();

int main(int argc, char **argv) {
	if (argc < 2) {
		cmd_help(argv[0]);
		exit(EXIT_FAILURE);
	}

	std::string cmd = argv[1];

	if (cmd == "count") {
		cmd_count();
	} else if (cmd == "countall") {
		cmd_countall();
	} else if (cmd == "solutions") {
		cmd_solutions();
	} else if (cmd == "create") {
		cmd_create();
	} else if (cmd == "ecoset15") {
		cmd_ecoset(15);
	} else if (cmd == "ecoset18") {
		cmd_ecoset(18);
	} else if (cmd == "neighbor") {
		cmd_neighbor();
	} else if (cmd == "unsolved") {
		cmd_unsolved();
	} else if (cmd == "ingest") {
		cmd_ingest(false);
	} else if (cmd == "optimal") {
		cmd_ingest(true);
	} else if (cmd == "free") {
		cmd_free();
	} else {
		cmd_help(argv[0]);
		exit(EXIT_FAILURE);
	}

	return 0;
}

void cmd_help(const std::string &argv0) {
	std::cout <<
		"usage: " << argv0 << " COMMAND\n"
		"\n"
		"Commands:\n"
		"    count      show solution count by depth\n"
		"    countall   show solution count for all symmetries\n"
		"    solutions  output symmetry representative solutions\n"
		"    create     make a new, empty involution database\n"
		"    ecoset15   edge coset solver to depth 15 (for testing)\n"
		"    ecoset18   edge coset solver to depth 18\n"
		"    neighbor   find optimal neihghbors of known solutions\n"
		"    unsolved   output unsolved cubes in singmaster notation\n"
		"    ingest     ingest solution move sequences\n"
		"    optimal    ingest optimal solution move sequences\n"
		"    free       free shared memory\n"
		"\n";
}

void cmd_count() {
	if (!tracker::open()) {
		abort();
	}
	status::show_counts();
}

void cmd_countall() {
	tracker::init();
	if (!tracker::open()) {
		abort();
	}

	size_t counts[MAX_DEPTH + 1] = {};

	std::mutex mtx;
	size_t ec_idx = 0;

	parallel workers([&](size_t id) {
		std::unique_lock lock(mtx);
		while (ec_idx < N_EDGE_INVO) {
			auto idx = ec_idx++;
			lock.unlock();

			tracker::handle handle(idx);
			uint64_t edges_self = handle.self_sym;

			size_t my_counts[MAX_DEPTH + 1] = {};
			for (auto moves : tracker::get_solutions(idx)) {
				cube c = cube::from_moveseq(moves);
				uint64_t self = 1;
				for (auto s : bits(edges_self)) {
					if (c.symi(s) == c) {
						self |= 1LL << s;
					}
				}
				my_counts[moves.size()] += N_SYM48 / std::popcount(self);
			}

			lock.lock();
			for (int i = 0; i <= MAX_DEPTH; i++) {
				counts[i] += my_counts[i];
			}
		}
	});

	workers.join();

	for (int d = 0; d <= MAX_DEPTH; d++) {
		std::cout << std::format(
			"{} {}\n", d, counts[d]);
	}
}

void cmd_solutions() {
	if (!tracker::open()) {
		abort();
	}
	for (int idx = 0; idx < N_EDGE_INVO; idx++) {
		for (auto moves : tracker::get_solutions(idx)) {
			std::cout << moves.to_string() << '\n';
		}
	}
}

void cmd_create() {
	std::cout << "Creating empty involution database...\n";
	tracker::init();
	tracker::create();
	std::cout << "done\n";
}

void cmd_ecoset(int depth) {
	tracker::init();
	if (!tracker::open()) {
		abort();
	}
	tracker::lock();

	eprune::init();

	interrupt::setup_signals();

	std::mutex mtx;
	size_t ec_idx = 0;
	bool canceled = false;

	status progress(N_EDGE_INVO);

	parallel workers([&](size_t id) {
		std::unique_lock lock(mtx);
		while (true) {
			for (; ec_idx < N_EDGE_INVO; ec_idx++) {
				auto h = tracker::get_header(ec_idx);
				if (h.proven_min <= depth && h.n_solved < h.n_cubes) {
					break;
				}
				progress.increment();
			}
			if (ec_idx == N_EDGE_INVO) {
				break;
			}
			size_t idx = ec_idx++;

			lock.unlock();
			auto handle = tracker::handle(idx);
			ecsolver{}.solve(handle, depth);
			lock.lock();

			progress.increment();

			if (interrupt::terminated()) {
				canceled = true;
				break;
			}
		}
	});

	workers.join();
	progress.stop();

	if (canceled) {
		std::cout << "received terminate signal\n";
	} else {
		std::cout << "done\n";
	}
}

void cmd_neighbor() {
	neighborsolver::init();

	tracker::init();
	if (!tracker::open()) {
		abort();
	}
	tracker::lock();

	std::map<cube, std::pair<int, int>> ec_lookup;
	for (int idx = 0; idx < N_EDGE_INVO; idx++) {
		auto h = tracker::get_header(idx);
		cube edges = h.get_ec();
		for (int s = 0; s < N_SYM48; s++) {
			ec_lookup.emplace(edges.symi(s), std::make_pair(idx, s));
		}
	}

	interrupt::setup_signals();

	std::mutex mtx;
	size_t ec_idx = 0;
	bool canceled = false;

	status progress(N_EDGE_INVO);

	parallel workers([&](size_t id) {
		std::unique_lock lock(mtx);
		while (ec_idx < N_EDGE_INVO) {
			size_t idx = ec_idx++;
			lock.unlock();

			cube edges = tracker::get_header(idx).get_ec();

			std::map<size_t, std::vector<std::pair<int, int>>> neighbors;
			for (int m = 0; m < N_MOVES; m++) {
				cube edges_m = edges.premove(move::inv(m)).move(m).setCornerPerm(0);
				auto [ ec_m, s ] = ec_lookup[edges_m];
				if (idx <= ec_m) {
					neighbors[ec_m].emplace_back(m, s);
				}
			}

			auto handle = tracker::handle(idx);

			for (auto [ idx_m, moves ] : neighbors) {
				if (idx == idx_m) {
					for (auto [ m, s ] : moves) {
						neighborsolver(handle, handle).solve(m, s);
					}
				} else {
					auto handle_m = tracker::handle(idx_m);
					for (auto [ m, s ] : moves) {
						int si = sym::inv(s);
						int mi = move::inv(sym::movei(m, si));
						neighborsolver(handle, handle_m).solve(m, s);
						neighborsolver(handle_m, handle).solve(mi, si);
					}
				}
			}

			lock.lock();
			progress.increment();

			if (interrupt::terminated()) {
				canceled = true;
				break;
			}
		}
	});

	workers.join();
	progress.stop();

	if (canceled) {
		std::cout << "received terminate signal\n";
	} else {
		std::cout << "done\n";
	}
}

void cmd_unsolved() {
	tracker::init();
	if (!tracker::open()) {
		abort();
	}
	tracker::lock();

	auto corners = involution::corners();

	auto get_unsolved = [&](int idx) {
		std::vector<cube> cubes;

		auto h = tracker::get_header(idx);
		if (h.n_solved == h.n_cubes) {
			return cubes;
		}

		cube edges = h.get_ec();
		tracker::handle handle(idx);

		for (auto cc : corners[h.parity]) {
			if (!handle.corner_set->test(corner_hash(cc))) {
				continue;
			}

			cube c = edges * cube(cc);

			bool is_rep = true;
			for (auto s : bits(handle.self_sym)) {
				if (c.sym(s) < c) {
					is_rep = false;
					break;
				}
			}

			if (is_rep) {
				cubes.push_back(c);
			}
		}

		return cubes;
	};

	std::mutex mtx;
	std::vector<std::condition_variable> ec_cv(N_EDGE_INVO);
	size_t ec_idx = 0, output_idx = 0;

	parallel workers([&](size_t id) {
		std::unique_lock lock(mtx);

		while (ec_idx < N_EDGE_INVO) {
			size_t idx = ec_idx++;

			lock.unlock();

			std::ostringstream buf;
			auto cubes = get_unsolved(idx);
			for (auto c : cubes) {
				buf << c.to_sing() << '\n';
			}

			lock.lock();
			ec_cv[idx].wait(lock, [&] { return output_idx == idx; });

			std::cout << buf.str();

			output_idx++;
			if (output_idx < N_EDGE_INVO) {
				ec_cv[output_idx].notify_one();
			}
		}
	});

	workers.join();
}

void cmd_ingest(bool optimal) {
	tracker::init();
	if (!tracker::open()) {
		abort();
	}
	tracker::lock();

	std::map<cube, int> ec_lookup;
	for (int idx = 0; idx < N_EDGE_INVO; idx++) {
		cube edges = tracker::get_header(idx).get_ec();
		ec_lookup.emplace(edges, idx);
	}

	interrupt::setup_signals();

	std::mutex mtx;

	parallel workers([&](size_t id) {
		std::string line;
		while (!interrupt::terminated()) {
			mtx.lock();
			bool ok = bool(getline(std::cin, line));
			mtx.unlock();
			if (!ok) {
				break;
			}

			moveseq moves = moveseq::parse(line);
			cube c = cube::from_moveseq(moves);
			if (c * c != cube{}) {
				std::cerr << "not an involution: " << line << "\n";
				continue;
			}

			auto it = ec_lookup.find(c.setCornerPerm(0));
			if (it == ec_lookup.end()) {
				std::cerr << "edge symmetry not canonical: " << line << "\n";
				continue;
			}
			auto idx = it->second;

			auto handle = tracker::handle(idx);
			bool proven = optimal || (moves.size() == handle.proven_min());
			if (proven && handle.is_unsolved(c)) {
				handle.solution(moves.canonical());
			}
		}
	});

	workers.join();

	if (interrupt::terminated()) {
		std::cout << "received terminate signal\n";
	} else {
		std::cout << "done\n";
	}
}

void cmd_free() {
	eprune::free();
}
