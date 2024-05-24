#ifndef INVL_MOVESEQ_H
#define INVL_MOVESEQ_H

#include <cstdint>
#include <vector>
#include <string>

struct moveseq : public std::vector<uint8_t> {
	using std::vector<uint8_t>::vector;

	static moveseq parse(const std::string &);
	moveseq canonical() const;
	std::string to_string() const;
};

#endif
