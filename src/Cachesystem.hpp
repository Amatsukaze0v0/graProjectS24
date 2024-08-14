#ifndef SRC_CACHESYSTEM_HPP
#define SRC_CACHESYSTEM_HPP

#include <systemc.h>
#include "Cache.hpp"
#include "Memory.hpp"

struct Request {
    uint32_t addr;
    uint32_t data;
    int we;
};

SC_MODULE(CacheSystem) {
    Cache l1Cache;
    Cache l2Cache;
    Memory memory;

    sc_in<bool> clk;
    sc_signal<uint32_t> addr, l1_addr_to_l2, l2_addr_to_mem, mem_addr_helper;
    sc_signal<bool> we, l1_we_to_l2, l2_we_to_mem;
    sc_signal<sc_uint<32>> data_in, data_out_l1, data_out_l2, mem_data, l1_data_to_l2,  l2_data_to_mem, mem_back_l2, l2_back_l1, l1_unused;
    sc_signal<bool> hit_l1, hit_l2, hit_mem;
    sc_signal<bool> trigger, l2_trigger, mem_trigger, memcmp, rev_ctr_l1, mem_rev, mem_reset;



    sc_event l1_done, l2_done, mem_done;

    unsigned l1CacheLines, l2CacheLines, cacheLineSize, l1CacheLatency, l2CacheLatency, memoryLatency;

    SC_HAS_PROCESS(CacheSystem);

    CacheSystem(sc_module_name name, unsigned l1CacheLines, unsigned l2CacheLines, unsigned cacheLineSize, unsigned l1CacheLatency, unsigned l2CacheLatency, unsigned memoryLatency)
        : sc_module(name), 
          l1Cache("L1Cache", l1CacheLines, cacheLineSize, l1CacheLatency),
          l2Cache("L2Cache", l2CacheLines, cacheLineSize, l2CacheLatency),
          memory("Memory", 1024 * 1024, memoryLatency),  // 1 MB memory
          l1CacheLines(l1CacheLines),
          l2CacheLines(l2CacheLines),
          cacheLineSize(cacheLineSize),
          l1CacheLatency(l1CacheLatency),
          l2CacheLatency(l2CacheLatency),
          memoryLatency(memoryLatency) {
        
        // Connect L1 Cache

        l1Cache.trg.bind(trigger);
        l1Cache.addr.bind(addr);
        l1Cache.we.bind(we);
        l1Cache.data_in.bind(data_in);
        l1Cache.data_out.bind(data_out_l1);
        l1Cache.hit.bind(hit_l1);
        l1Cache.next_addr.bind(l1_addr_to_l2);
        l1Cache.next_data_out.bind(l1_data_to_l2);
        l1Cache.next_we.bind(l1_we_to_l2);
        l1Cache.next_data_in.bind(data_out_l2);
        l1Cache.next_hit.bind(rev_ctr_l1);
        l1Cache.wb_addr.bind(l2_back_l1);
        l1Cache.wb_addr_o.bind(l1_unused);

        // Connect L2 Cache

        l2Cache.trg.bind(l2_trigger);
        l2Cache.addr.bind(l1_addr_to_l2);
        l2Cache.we.bind(l1_we_to_l2);
        l2Cache.data_in.bind(l1_data_to_l2);
        l2Cache.data_out.bind(data_out_l2);
        l2Cache.hit.bind(hit_l2);
        l2Cache.next_addr.bind(l2_addr_to_mem);
        l2Cache.next_data_out.bind(l2_data_to_mem);
        l2Cache.next_we.bind(l2_we_to_mem);
        l2Cache.next_data_in.bind(mem_data);
        l2Cache.next_hit.bind(hit_mem);
        l2Cache.wb_addr.bind(mem_back_l2);
        l2Cache.wb_addr_o.bind(l2_back_l1);

        // Connect Memory
        memory.trg.bind(mem_trigger);
        memory.addr.bind(l2_addr_to_mem);
        memory.data_in.bind(l2_data_to_mem);
        memory.we.bind(l2_we_to_mem);
        memory.data_out.bind(mem_data);
        memory.wb.bind(hit_mem);
        memory.wb_addr.bind(mem_back_l2);
        memory.addr_helper.bind(mem_addr_helper);
        memory.reverse.bind(mem_rev);
        memory.reset.bind(mem_reset);
        

        SC_THREAD(process_requests);
        sensitive << clk.pos();

    }

    void process_request(Request& req) {
        if (req.addr & 3 != 0 || req.addr > 0xFFFFF) {
            throw std::runtime_error("Address illegal! For our 1MB Memory must the Address under 0xFFFFF and Divisible by 4");
        }
        addr.write(req.addr);
        we.write(req.we);
        data_in.write(req.data);
        wait(clk.posedge_event());

        // Process L1 Cache
        std::cout << "L1cache process started." << std::endl;
        trigger.write(1);
        
        std::cout << "L1 WE is : " << we.read() << std::endl;
        wait(l1Cache.done_event);
        wait(SC_ZERO_TIME);     // wait while trigger not 0; index 为0时thread返回过快，trigger未归零, 别动
        trigger.write(0);
        if (!we.read()) {
            req.data = l1Cache.data_out.read();
        }
        std::cout << "L1cache process done." << std::endl;
        std::cout << "we :" << we.read() << std::endl;
        std::cout << "hit_l1 :" << hit_l1.read() << std::endl;
        if ( (!hit_l1.read() ) || we.read()) {
            // L1 Miss, process L2 Cache
            std::cout << "L2cache process started." << std::endl;
            l2_trigger.write(1);
            wait(l2Cache.done_event);
            l2_trigger.write(0);
            if (!we.read()) {
                req.data = l2Cache.data_out.read();
            }
            std::cout << "L2cache process done." << std::endl;
            std::cout << "L2 Hit is : " << hit_l2 << std::endl;
            std::cout << "L1 Hit is : " << hit_l1 << std::endl;
            if ( (!hit_l2.read()) || we.read()) {
                // L2 Miss, process Memory
                std::cout << "Memory process started." << std::endl;
                
                mem_trigger.write(1);
                wait(memory.done_event);
                mem_trigger.write(0);
                
                std::cout << "Memory process done." << std::endl; 
                //data should being written in request; 这里data应已写入request
                if (!we.read()) {
                    req.data = memory.data_out.read();
                }
                if (hit_mem.read()) {
                    unsigned offset_bits = log2(cacheLineSize);
                    std::cout << "offset bits is " << offset_bits << std::endl;
                    sc_uint<32> startAddress = addr.read();
                    startAddress >>= offset_bits;
                    startAddress <<= offset_bits;
                    
                    int maxTime = cacheLineSize / 4;

                    rev_ctr_l1.write(1);
                    mem_rev.write(1);
                    wait(SC_ZERO_TIME);
                    for (int i = 0; i < maxTime; i++) {
                        std::cout << "write back loop at the " << i + 1 << ". time!" << std::endl;
                        mem_addr_helper.write(startAddress);
                        std::cout << "with StartAddress: " << startAddress << std::endl;
                        
                        mem_trigger.write(1);
                        wait(memory.done_event);
                        wait(SC_ZERO_TIME);  // make sure the signal is stable; 确保信号稳定
                        mem_trigger.write(0);
                        
                        //from here: send new addresse coresponde to data from mem; 此刻开始，将新地址对应data传出mem
                        std::cout << "Memory starts to write L2." << std::endl;
                        l2_trigger.write(1);
                        wait(SC_ZERO_TIME);  // make sure the signal is stable; 确保信号稳定
                        wait(l2Cache.done_event);
                        l2_trigger.write(0);
            
                        std::cout << "L2 writed back." << std::endl;

                        std::cout << "L2 starts to write L1." << std::endl;
                        
                        trigger.write(1);
                        wait(SC_ZERO_TIME);  // make sure the signal is stable; 确保信号稳定
                        wait(l1Cache.done_event);
                        trigger.write(0);
                        wait(SC_ZERO_TIME);  // make sure the signal is stable; 确保信号稳定
                        std::cout << "L1 writed back." << std::endl;
                        
                        startAddress += 4;
                    }
                    mem_rev.write(0);
                    rev_ctr_l1.write(0);
                    wait(SC_ZERO_TIME);

                    mem_trigger.write(1);
                    mem_reset = 1;
                    wait(SC_ZERO_TIME);
                    wait(memory.done_event);
                    mem_trigger.write(0);
                    mem_reset.write(0);
                    wait(SC_ZERO_TIME);
                }

            }
        }
    }

    void process_requests() {
        int counter = 0;
        std::cout << "Numrequests is " << numRequests << std::endl;
        while (true) {
            wait();  // wait for rising edge; 等待时钟上升沿

            if (current_request_index < numRequests) {
                counter++;
                std::cout << "This is the " << counter << " time." << std::endl;

                process_request(requests[current_request_index]);  // transmit referrence; 传递引用

                if (hit_l1.read() || hit_l2.read()) {
                    ++hits;
                } else {
                    ++misses;
                }
                ++current_request_index;
            } else {
                sc_stop();
                break;
            }
        }
    }


    size_t numRequests = 0;
    Request* requests = nullptr;
    size_t hits = 0;
    size_t misses = 0;

private:
    size_t current_request_index = 0;

    sc_time latency_to_time(unsigned latency) {
        return sc_time(latency, SC_NS);
    }
};

#endif // SRC_CACHESYSTEM_HPP