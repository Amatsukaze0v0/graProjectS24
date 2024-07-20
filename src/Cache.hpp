#ifndef CACHE_HPP
#define CACHE_HPP

#include <systemc.h>
#include <vector>
#include <cmath>
#include "CacheLine.hpp"

class Cache : public sc_module {
public:
    std::vector<CacheLine*> lines;
    unsigned size;
    unsigned lineSize;
    unsigned latency;

    sc_in<bool> trg;
    sc_in<bool> we;
    sc_in<uint32_t> addr;
    sc_in<sc_uint<32>> data_in;
    sc_out<sc_uint<32>> data_out;
    sc_out<bool> hit;
    sc_event done_event;

    sc_out<uint32_t> next_addr;
    sc_out<sc_uint<32>> next_data_out;
    sc_out<bool> next_we;
    sc_in<sc_uint<32>> next_data_in;
    sc_in<bool> next_hit;
    sc_in<sc_uint<32>> wb_addr;
    sc_out<sc_uint<32>> wb_addr_o;

    SC_HAS_PROCESS(Cache);

    Cache(sc_module_name name, unsigned num_lines, unsigned cache_line_size, unsigned cache_latency)
        : sc_module(name), size(num_lines), lineSize(cache_line_size), latency(cache_latency) {
        lines.resize(size, nullptr);
        for (int i = 0; i < size; i++) {
            lines[i] = new CacheLine(sc_gen_unique_name("line"), lineSize);
        }

        SC_THREAD(process);
        sensitive << trg;
    }

    ~Cache() {
        for (auto& line : lines) {
            delete line;
        }
    }

    void process() {
        while (true) {

            wait(trg.posedge_event());  // 等待触发信号


            if (next_hit.read()) {
                std::cout << "Reverse Process activ." << std::endl;
                unsigned offset_bits = log2(lineSize);
                unsigned index_bits = log2(size);
                unsigned tag_bits = 32 - offset_bits - index_bits;

                uint32_t address = wb_addr.read();
                wait(SC_ZERO_TIME);
                unsigned offset_mask = (1 << offset_bits) - 1;
                unsigned index_mask = ((1 << index_bits) - 1) << offset_bits;
                unsigned tag_mask = ~((1 << (offset_bits + index_bits)) - 1);

                unsigned offset = address & offset_mask;
                unsigned index = (address & index_mask) >> offset_bits;
                unsigned tag = (address & tag_mask) >> (offset_bits + index_bits);

                CacheLine* line = lines[index];

                line->data[offset / 4].write(next_data_in.read());
                line->valid.write(true);
                line->tag.write(tag);
                std::cout << "Reverse write Data by line: " << index << std::endl;
                std::cout << "By offset: " << offset << " and the value [ " << next_data_in.read() << "]" << std::endl;
                std::cout << "With Tag: " << tag << std::endl;
                data_out.write(next_data_in.read());
                wb_addr_o.write(address);
            } else {
                std::cout << "cache inner process started." << std::endl;

                unsigned offset_bits = log2(lineSize);
                unsigned index_bits = log2(size);
                unsigned tag_bits = 32 - offset_bits - index_bits;

                uint32_t address = addr.read();
                unsigned offset_mask = (1 << offset_bits) - 1;
                unsigned index_mask = ((1 << index_bits) - 1) << offset_bits;
                unsigned tag_mask = ~((1 << (offset_bits + index_bits)) - 1);

                unsigned offset = address & offset_mask;
                unsigned index = (address & index_mask) >> offset_bits;
                uint32_t tag = (address & tag_mask) >> (offset_bits + index_bits);

                CacheLine* line = lines[index];

                std::cout << "Index for this round: " << index << std::endl;
                std::cout << "Offset for this round: " << offset << std::endl;
                std::cout << "Tag for this round: " << tag << std::endl;

                std::cout << std::endl << "line valid is : " << line->valid.read() << std::endl;
                std::cout << "Tag inside cache is : " << line->tag.read() << std::endl;
            
                if (line->valid.read() && line->tag.read() == tag) {
                    hit.write(true);
                    if (we.read()) {
                        line->data[offset / 4].write(data_in.read());  // write data hit
                        next_addr.write(addr.read());
                        next_data_out.write(data_in.read());
                        next_we.write(true);  // Write-through to L2
                        wait(SC_ZERO_TIME);
                        std::cout << "Write hit. Data " << data_in.read() << " writed in cache." << std::endl;
                    } else {                                            // read hit
                        data_out.write(line->data[offset / 4].read());
                        wait(SC_ZERO_TIME);
                        std::cout << "Read hit with value: " << data_out.read() << std::endl;
                        hit.write(true);
                    }
                } else {
                    hit.write(false);
                    if (!we.read()) {
                        next_addr.write(addr.read());
                        next_we.write(false);  // Read operation from L2
                        wait(latency_to_time(latency));  // Simulate L2 latency
                        std::cout << "Read miss. Proceed to next Structur." << std::endl;
                    } else {
                        next_addr.write(addr.read());
                        next_data_out.write(data_in.read());
                        next_we.write(true);  // Write-through to L2
                        std::cout << "Write miss. Data not writed." << std::endl;
                    }
                }
            }
            done_event.notify();  // 通知完成事件
        }
    }

private:
    sc_time latency_to_time(unsigned latency) {
        return sc_time(latency, SC_NS);
    }
};

#endif // CACHE_HPP