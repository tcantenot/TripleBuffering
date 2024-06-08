# Triple buffering

Single producer, single consumer, triple buffering.

Based on: https://brilliantsugar.github.io/posts/how-i-learned-to-stop-worrying-and-love-juggling-c++-atomics/

```cpp

Data data[3];
TripleBufferIndex tripleBufferIndex;

// Produder thread
while(...)
{
    const uint32_t writeIdx = tripleBufferIndex.getWriteIndex();
    data[writeIdx] = ...;
    tripleBufferIndex.commitWriteAndSwap();
}

// Consumer thread
while(...)
{
    bool bNewVersion = false;
    uint64_t version;
    const uint32_t readIndex = tripleBufferIndex.getReadIndexAndSwapIfNewVersion(&bNewVersion, &version);
    const Data & mostRecentData = data[readIdx];
    if(bNewVersion)
    {
        printf("New version: %llu\n", version);
    }
    else
    {
        printf("Old version: %llu\n", version);
    }
}

```

## Relacy

Checked for race conditions with Relacy (https://github.com/dvyukov/relacy)