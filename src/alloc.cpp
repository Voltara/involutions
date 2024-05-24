#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include "alloc.h"
#include "thread.h"

constexpr size_t ALIGN = 1LL << 6;
constexpr size_t PAGE_SIZE = 30;

static void *arena = NULL;
static size_t top = 0;
static std::mutex mtx;

#ifndef SHM_HUGE_SHIFT
#define SHM_HUGE_SHIFT MAP_HUGE_SHIFT
#endif

static size_t round_up(size_t n) {
	return (n + ALIGN - 1) / ALIGN * ALIGN;
}

static size_t num_pages(size_t n) {
	size_t mask = (1LL << PAGE_SIZE) - 1;
	return (n >> PAGE_SIZE) + !!(n & mask);
}

void * alloc::huge_impl(size_t n) {
	n = round_up(n);

	size_t pages = num_pages(top + n);
	if (pages > ALLOC_GB) {
		std::cerr << "ALLOC_GB too small (was " << ALLOC_GB << ", needed " << pages << ")\n";
		abort();
	}

	std::unique_lock lock(mtx);

	if (!arena) {
		int prot = PROT_READ | PROT_WRITE;
		int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | (PAGE_SIZE << MAP_HUGE_SHIFT);
		arena = mmap(NULL, ALLOC_GB << PAGE_SIZE, prot, flags, -1, 0);
		if (arena == MAP_FAILED) {
			perror("mmap");
			abort();
		}
	}

	void *mem = ((char *) arena) + top;
	top += n;

	return mem;
}

void * alloc::shared_impl(size_t n, uint32_t key) {
	n = num_pages(n);

	int flags = 0600 | IPC_CREAT | SHM_HUGETLB | (PAGE_SIZE << SHM_HUGE_SHIFT);

	int shm = shmget(key, n << PAGE_SIZE, flags);
	if (shm == -1) {
		return NULL;
	}

	void *mem = shmat(shm, NULL, 0);

	return (mem == (void *) -1) ? NULL : mem;
}

void alloc::shared_free_impl(uint32_t key) {
	int id = shmget(key, 0, 0);
	if (id != -1) {
		if (shmctl(id, IPC_RMID, NULL) != 0) {
			std::cerr << "Error freeing shm id " << id << '\n';
		}
	}
}

std::pair<void *, int> alloc::mmap_file_impl(size_t n, const std::string &path) {
	int fd = open(path.c_str(), O_RDWR);
	if (fd == -1) {
		return std::make_pair((void *) NULL, -1);
	}

	int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_SHARED;
	void *mem = mmap(NULL, n, prot, flags, fd, 0);

	if (mem == MAP_FAILED) {
		perror("mmap");
		abort();
	}

	return std::make_pair(mem, fd);
}
