#ifndef SRC_CACHELINE_HPP
#define SRC_CACHELINE_HPP

#include <systemc.h>
#include <vector>

class CacheLine : public sc_module {
public:
    sc_vector<sc_signal<sc_uint<32>>> data;
    sc_signal<bool> valid;
    sc_signal<uint32_t> tag;

    SC_HAS_PROCESS(CacheLine);

    CacheLine(sc_module_name name, unsigned cache_line_size)
        : sc_module(name), data("data", cache_line_size / 4) { // 4 bytes per 32-bit data word
        for (int i = 0; i < data.size(); ++i) {
            data[i].write(0);
        }
        valid.write(false);
    }
};

#endif // SRC_CACHELINE_HPP
