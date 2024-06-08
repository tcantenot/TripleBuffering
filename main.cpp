#include <TripleBufferIndex.hpp>

#include <relacy/relacy.hpp>

// One for consumer, one for producer
static constexpr size_t kThreads = 2;

// Each buffer is just a single integer.
// Testing for:
// - Order of updates is the same on producer and consumer.
// - Last written value by producer is never lost
// - Other conditions that relacy checks automatically:
//   data races, deadlocks, livelocks.
struct TripleBufferTest : rl::test_suite<TripleBufferTest, kThreads>
{
	static constexpr size_t kLastNumber = 25;

	TripleBufferIndex tripleBufferIndex;
	rl::var<int> values[3] = { rl::var<int>{0}, rl::var<int>{0}, rl::var<int>{0} };

	void thread(unsigned thread_index)
	{
		if(thread_index == 0) // Producer
		{
			// Writing 25 integers
			for(int i = 1; i <= kLastNumber; ++i)
			{
				const uint32_t writeIndex = tripleBufferIndex.getWriteIndex();
				auto rlval = values[writeIndex]($);
				rlval.store(i);
				tripleBufferIndex.commitWriteAndSwap();
			}
		}
		else // Consumer
		{
			// Read until we get 25, if 25 is never read Relacy will report livelock.
			int prevVal = 0;
			while(prevVal != kLastNumber)
			{
				bool bNewVersion = false;
				uint64_t version;
				const uint32_t readIndex = tripleBufferIndex.getReadIndexAndSwapIfNewVersion(&bNewVersion, &version);
				int val = values[readIndex]($);
				if(bNewVersion)
				{
					//printf("Version: %llu\n", version);

					// Check order: new value must be > than the previous
					// It will be > when it's new value.
					RL_ASSERT(val > prevVal);
					prevVal = val;
				}
				else
				{
					RL_ASSERT(val == prevVal);
				}
			}
		}
	}
};

int main()
{
	rl::simulate<TripleBufferTest>();
	return 0;
}