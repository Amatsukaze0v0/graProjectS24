#include <cstdint>
#include <stddef.h>
#include "cache.cpp"
#include "l2Cache.cpp"

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

Result run_simulation  (int cycles,
                        unsigned l1CacheLines,
                        unsigned l2CacheLines,
                        unsigned cacheLineSize,
                        unsigned l1CacheLatency,
                        unsigned l2CacheLatency,
                        unsigned memoryLatency,
                        size_t numRequests,
                        struct Request requests[],
                        const char* tracefile) {
    //Todo: 创建两个cache                        
    DataCache l1 = DataCache("L1Cache", l1CacheLines, cacheLineSize);
    DataCache l2 = DataCache("L2Cache", l2CacheLines, cacheLineSize);

    // 创建并打开tracefile（如果不为NULL）
    sc_trace_file *file;
    if (tracefile != nullptr) {
        file = sc_create_vcd_trace_file("my Tracefile");
        //Todo : 也就是此处需要将file作为参数传入一个抽象层，记录数据
        // e.g.   sc_trace(file, out_a, "Out A");
        //    sc_trace(file, out_b, "Out B");
        //    sc_trace(file, done, "done signal");
    }

    //Todo: 比较请求数与周期数
    //    Idea.1： 粗略估计 1 request ~~ 200 cycles
    //     Idea.2 : 设置counter并与cycles比较，如超越则size Max

    //Todo： for each执行request数组，需要等待指令执行完毕


    // 关闭tracefile
    sc_close_vcd_trace_file(file);

    //Todo : 包装result
    
};