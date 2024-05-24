#ifndef INVL_ALLOC_H
#define INVL_ALLOC_H

class alloc {
	static constexpr size_t ALLOC_GB = 3;

    public:
	template<typename T>
	static T * huge(size_t n) {
		return (T *) huge_impl(n * sizeof(T));
	}

	template<typename T>
	static T * shared(size_t n, uint32_t key) {
		return (T *) shared_impl(n * sizeof(T), key);
	}

	static void shared_free(uint32_t key) {
		shared_free_impl(key);
	}

	template<typename T>
	static std::pair<T *, int> mmap_file(size_t n, const std::string &path) {
		auto [ mem, fd ] = mmap_file_impl(n * sizeof(T), path);
		return std::make_pair((T *) mem, fd);
	}

    private:
	static void * huge_impl(size_t n);
	static void * shared_impl(size_t n, uint32_t key);
	static void shared_free_impl(uint32_t key);
	static std::pair<void *, int> mmap_file_impl(size_t n, const std::string &path);
};

#endif
