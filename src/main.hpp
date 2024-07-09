#include <cstdint>
#include <stddef.h>

struct Request {
 uint32_t addr;
 uint32_t data;
 int we;
};

struct Result {
 size_t cycles;
 size_t misses;
 size_t hits;
 size_t primitiveGateCount;
};

struct Result run_simulation(
 int cycles,
 unsigned l1CacheLines,
 unsigned l2CacheLines,
 unsigned cacheLineSize,
 unsigned l1CacheLatency,
 unsigned l2CacheLatency,
 unsigned memoryLatency,
 size_t numRequests,
 struct Request requests[],
 const char* tracefile);