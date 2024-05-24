#ifndef INVL_TABLES_H
#define INVL_TABLES_H

#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>

namespace tables {
	namespace {
		static inline bool rename(const std::string &old_path, const std::string &new_path) {
			std::error_code ec;
			std::filesystem::rename(old_path, new_path, ec);
			return !ec;
		}

		static inline bool resize(const std::string &path, size_t size) {
			std::error_code ec;
			std::filesystem::resize_file(path, size, ec);
			return !ec;
		}
	}

	static inline std::string full_path(std::string filename) {
		return "tables/" + filename;
	}

	static inline bool exists(std::string filename) {
		return std::filesystem::exists(full_path(filename));
	}

	template<class T>
	static bool load(std::string filename, T *table, size_t size) {
		auto path = full_path(filename);
		std::ifstream f(path, std::ifstream::binary);
		return f && f.read((char *) table, size * sizeof(*table));
	}

	template<class T>
	static bool save(std::string filename, const T *table, size_t size) {
		auto path = full_path(filename);
		auto tmp = path + ".tmp";
		std::ofstream f(tmp, std::ofstream::binary);
		return f && f.write((const char *) table, size * sizeof(*table)) &&
			f.flush() && rename(tmp, path);
	}

	template<class T>
	static bool save_and_extend(std::string filename, const T *table, size_t size, size_t full_size) {
		auto path = full_path(filename);
		auto tmp = path + ".resize";
		return save(tmp, table, size) && resize(tmp, full_size) && rename(tmp, path);
	}
};

#endif
