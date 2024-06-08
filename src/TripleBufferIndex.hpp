#pragma once
#ifndef TRIPLE_BUFFER_INDEX_HPP
#define TRIPLE_BUFFER_INDEX_HPP

#define K_TRIPLE_BUFFER_INDEX_USE_RELACY 1

#if defined(K_TRIPLE_BUFFER_INDEX_USE_RELACY)
	#include <relacy/relacy.hpp>
#else
	#include <atomic>
	#include <thread>
#endif

#include <cstdint>

// Single-producer-single-consumer triple buffer that allows to decouple production and consuption speed
// Inspired by: https://github.com/brilliantsugar/trio
class TripleBufferIndex
{
	#if defined(K_TRIPLE_BUFFER_INDEX_USE_RELACY)
	template <typename T>
	using AtomicType = rl::atomic<T>;
	static constexpr rl::memory_order kMemoryOrderRelaxed = rl::mo_relaxed;
	static constexpr rl::memory_order kMemoryOrderAcqRel  = rl::mo_acq_rel;
	#define K_TRIPLE_BUFFER_INDEX_VAR(name) name($)
	#else
	template <typename T>
	using AtomicType = std::atomic<T>;
	static constexpr std::memory_order kMemoryOrderRelaxed = std::memory_order_relaxed;
	static constexpr std::memory_order kMemoryOrderAcqRel  = std::memory_order_acq_rel;
	#define K_TRIPLE_BUFFER_INDEX_VAR(name) name
	#endif

	public:
		struct Index
		{
			uint64_t index  : 2;
			uint64_t bDirty : 1;
			uint64_t version;

			Index(uint64_t i = 0): index(i), bDirty(0), version(-1) { }

			#if defined(K_TRIPLE_BUFFER_INDEX_USE_RELACY)
			friend std::ostream & operator<<(std::ostream & os, Index idx)
			{
				os << "{ idx: " << idx.index << " | dirty: " << idx.bDirty << " }";
				return os;
			}
			#endif
		};

	public:
		TripleBufferIndex() = default;

		// Consumer API
		//   Returns the index to the most recent data.
		//   If there's a new value recently written -> swap indices and return the newest index.
		//   Otherwise, will return the index of the most recent (stale) data.
		uint32_t getReadIndexAndSwapIfNewVersion(bool * bNewVersion = nullptr, uint64_t * version = nullptr)
		{
			const Index middleBufferIdx = K_TRIPLE_BUFFER_INDEX_VAR(m_middleBuffer).load(kMemoryOrderRelaxed);

			if(bNewVersion)
				*bNewVersion = middleBufferIdx.bDirty;

			if(middleBufferIdx.bDirty)
			{
				m_frontBuffer = K_TRIPLE_BUFFER_INDEX_VAR(m_middleBuffer).exchange(m_frontBuffer, kMemoryOrderAcqRel);
				m_frontBuffer.bDirty = 0;
			}

			if(version)
				*version = m_frontBuffer.version;

			return (uint32_t)m_frontBuffer.index;
		}

		// Producer API
		//   Returns the index to which the producer can safely write.
		//   Once finished filling the data commitWriteAndSwap() must be called in order to update the
		//   most recent index.
		uint32_t getWriteIndex()
		{
			return (uint32_t)m_backBuffer.index;
		}

		// Producer API
		//   Update the read index to points to the most recent data index and update the write index.
		void commitWriteAndSwap()
		{
			m_backBuffer.bDirty = 1;
			m_backBuffer.version = version++;
			m_backBuffer = K_TRIPLE_BUFFER_INDEX_VAR(m_middleBuffer).exchange(m_backBuffer, kMemoryOrderAcqRel);
			m_backBuffer.bDirty = 0;
		}

	private:

		static constexpr size_t kCacheLineSizeInBytes = 64; // Byte alignment to avoid false sharing
		
		AtomicType<Index> m_middleBuffer{1};
		uint64_t version{0};
		alignas(kCacheLineSizeInBytes) Index m_backBuffer{0};
		alignas(kCacheLineSizeInBytes) Index m_frontBuffer{2};
};

#endif //TRIPLE_BUFFER_INDEX_HPP