#include "Cachesystem.hpp"


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
        const char* tracefile) {

    CacheSystem cacheSystem("CacheSystem", l1CacheLines, l2CacheLines, cacheLineSize, l1CacheLatency, l2CacheLatency, memoryLatency);

    sc_clock clk("clk", 1, SC_NS);
    cacheSystem.clk(clk);

    cacheSystem.numRequests = numRequests;
    cacheSystem.requests = requests;
    sc_trace_file *traceFile;
    if (tracefile != nullptr) {
        traceFile = sc_create_vcd_trace_file(tracefile);
        // 跟踪时钟信号
        sc_trace(traceFile, clk, "clk");

        // 跟踪CacheSystem模块中的所有信号
        sc_trace(traceFile, cacheSystem.addr, "addr");
        sc_trace(traceFile, cacheSystem.l1_addr_to_l2, "l1_addr_to_l2");
        sc_trace(traceFile, cacheSystem.l2_addr_to_mem, "l2_addr_to_mem");
        sc_trace(traceFile, cacheSystem.mem_addr_helper, "mem_addr_helper");
        sc_trace(traceFile, cacheSystem.we, "we");
        sc_trace(traceFile, cacheSystem.l1_we_to_l2, "l1_we_to_l2");
        sc_trace(traceFile, cacheSystem.l2_we_to_mem, "l2_we_to_mem");
        sc_trace(traceFile, cacheSystem.data_in, "data_in");
        sc_trace(traceFile, cacheSystem.data_out_l1, "data_out_l1");
        sc_trace(traceFile, cacheSystem.data_out_l2, "data_out_l2");
        sc_trace(traceFile, cacheSystem.mem_data, "mem_data");
        sc_trace(traceFile, cacheSystem.l1_data_to_l2, "l1_data_to_l2");
        sc_trace(traceFile, cacheSystem.l2_data_to_mem, "l2_data_to_mem");
        sc_trace(traceFile, cacheSystem.mem_back_l2, "mem_back_l2");
        sc_trace(traceFile, cacheSystem.l2_back_l1, "l2_back_l1");
        sc_trace(traceFile, cacheSystem.l1_unused, "l1_unused");
        sc_trace(traceFile, cacheSystem.hit_l1, "hit_l1");
        sc_trace(traceFile, cacheSystem.hit_l2, "hit_l2");
        sc_trace(traceFile, cacheSystem.hit_mem, "hit_mem");
        sc_trace(traceFile, cacheSystem.trigger, "trigger");
        sc_trace(traceFile, cacheSystem.l2_trigger, "l1cmp");
        sc_trace(traceFile, cacheSystem.mem_trigger, "l2cmp");
        sc_trace(traceFile, cacheSystem.memcmp, "memcmp");
        sc_trace(traceFile, cacheSystem.rev_ctr_l1, "rev_ctr_l1");
        sc_trace(traceFile, cacheSystem.mem_rev, "mem_rev");
        sc_trace(traceFile, cacheSystem.mem_reset, "mem_reset");
    }

    size_t hits = 0;
    size_t misses = 0;

    sc_start(cycles, SC_NS);

    hits = cacheSystem.hits;
    misses = cacheSystem.misses;

    struct Result result;
    sc_time simulation_time = sc_time_stamp();
    result.cycles = simulation_time / sc_time(1, SC_NS);
    result.misses = misses;
    result.hits = hits;
    result.primitiveGateCount = l1CacheLines * l2CacheLines * cacheLineSize; // 简化估算

    if (tracefile != nullptr) {
        sc_close_vcd_trace_file(traceFile);
    }

    return result;
}

