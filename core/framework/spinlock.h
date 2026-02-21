#pragma once

#include <atomic>

inline void Pause() {
#if defined(_MSC_VER) || defined(__SSE2__)
	_mm_pause();
#elifdef __arm__
	__yield();
#else
	__builtin_ia32_pause();
#endif
}

struct spinlock {
	std::atomic<bool> lock_ = { 0 };

	// Using snake_case to allow use with std mecanisms
	void lock() noexcept {
		for (;;) {
			// Optimistically assume the lock is free on the first try
			if (!lock_.exchange(true, std::memory_order_acquire)) {
				return;
			}
			// Wait for lock to be released without generating cache misses
			while (lock_.load(std::memory_order_relaxed)) {
				// Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
				// hyper-threads
				Pause();
			}
		}
	}

	bool try_lock() noexcept {
		// First do a relaxed load to check if lock is free in order to prevent
		// unnecessary cache misses if someone does while(!try_lock())
		return !lock_.load(std::memory_order_relaxed) && !lock_.exchange(true, std::memory_order_acquire);
	}

	void unlock() noexcept { lock_.store(false, std::memory_order_release); }
};