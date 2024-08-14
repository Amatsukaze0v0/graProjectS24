#ifndef SRC_MEMORY_HPP
#define SRC_MEMORY_HPP

#include <systemc.h>
#include <unordered_map>
#include <vector>

class Memory : public sc_module {
public:

    sc_in<bool> trg;
    sc_in<bool> we;
    sc_in<uint32_t> addr;
    sc_in<sc_uint<32>> data_in;
    sc_out<sc_uint<32>> data_out;
    sc_out<bool> wb;   //write back
    sc_out<sc_uint<32>> wb_addr;
    sc_in<uint32_t> addr_helper;
    sc_in<bool> reverse, reset;

    std::vector<sc_uint<32>> storage;
    unsigned latency;

    sc_event done_event;

    SC_HAS_PROCESS(Memory);

    Memory(sc_module_name name, unsigned size, unsigned mem_latency)
        : sc_module(name), storage(size / 4, 0), latency(mem_latency) { // size is in bytes
        SC_THREAD(process);
        sensitive << trg;
    }

    void process() {
        while (true) {
            wait(trg.posedge_event());  // waiting to trigger signal; 等待触发信号
            uint32_t address;
            if (reverse.read()) {
                address = addr_helper.read() / 4;
                data_out.write(storage[address]);

                //optional print expression for debug
                std::cout << "Get(R) Memory Address [" << address << "] 's value " << data_in.read() << std::endl;

                wb_addr.write(addr_helper.read());
                wb.write(1);
                wait(SC_ZERO_TIME);

                //optional print expression for debug
                std::cout << "Memory(R) send out Address " << address << std::endl;

            } else if (reset.read()) {
                wb.write(0);
            } else {
                address = addr.read() / 4;  // Addressing by 32-bit words 
                if (we.read()) {
                storage[address] = data_in.read();
                //optional print expression for debug
                std::cout << "Set Memory Address [" << address << "] with value " << data_in.read() << std::endl;
                wb.write(0);
                } else {
                    data_out.write(storage[address]);
                    //optional print expression for debug
                    std::cout << "Get Memory Address [" << address << "] 's value " << data_in.read() << std::endl;
                    wb.write(1);
                }
            }
            wait(latency_to_time(latency));  // simulate latency; 模拟延迟
            done_event.notify();  // notify: finished; 通知完成事件
        }
    }

private:
    sc_time latency_to_time(unsigned latency) {
        return sc_time(latency, SC_NS);
    }
};

#endif // SRC_MEMORY_HPP
