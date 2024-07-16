#include <iostream>
#include <fstream>
#include <string>
#include <systemc>
#include <systemc.h>
#include <sstream>
#include <vector>
#include "cache.hpp"  // 假设您的缓存实现放在这个头文件中
#include "rahmensprogramm.h"  // 包含命令行解析的头文件
#include "main.hpp"



void read_config(CacheConfig &config) {
    std::ifstream file("config.txt");
    if (!file) {
        std::cerr << "Failed to open config file" << std::endl;
        exit(EXIT_FAILURE);
    }

    file >> config.cycles;
    file >> config.cacheline_size;
    file >> config.l1_lines;
    file >> config.l2_lines;
    file >> config.l1_latency;
    file >> config.l2_latency;
    file >> config.memory_latency;
    file.ignore();
/*     std::getline(file, config.tracefile);
    std::getline(file, config.input_file); */

    file.close();
}


// 函数用于读取并解析输入文件
std::vector<Request> read_input_file(const std::string& filename) {
    std::cout << "successfully in the csv function." << std::endl;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open input file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    std::vector<Request> requests;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string type_str, addr_str, data_str;

        if (!std::getline(ss, type_str, ',') || !std::getline(ss, addr_str, ',')) {
            std::cerr << "Invalid line format: " << line << std::endl;
            exit(EXIT_FAILURE);
        }

        Request request;
        request.addr = std::stoul(addr_str, nullptr, 0);
        
        if (type_str == "W") {
            if (!std::getline(ss, data_str, ',')) {
                std::cerr << "Missing data for write operation: " << line << std::endl;
                exit(EXIT_FAILURE);
            }
            request.data = std::stoul(data_str, nullptr, 0);
            request.we = 1;  // Write operation
        } else if (type_str == "R") {
            request.data = 0;  // For read operations, data is not used
            request.we = 0;  // Read operation
        } else {
            std::cerr << "Invalid operation type: " << type_str << std::endl;
            exit(EXIT_FAILURE);
        }

        requests.push_back(request);
    }

    file.close();
    return requests;
}

//test
SC_MODULE(MODULE) {

  sc_port<sc_signal<int>> p;
  SC_CTOR(MODULE) {

    SC_THREAD(writer);
  }
  void writer() {
    int v = 1;
    while (true) {

      p->write(v++);

      wait(1, SC_SEC);
    }
  }
};

int sc_main(int argc, char* argv[]) {
    CacheConfig config;

    if (parse_args(argc, argv, &config) != 0) {
        return 1;
    }

    // 打印解析的参数（用于调试）
    std::cout << "Cycles: " << config.cycles << std::endl;
    std::cout << "Cacheline size: " << config.cacheline_size << " bytes" << std::endl;
    std::cout << "L1 lines: " << config.l1_lines << std::endl;
    std::cout << "L2 lines: " << config.l2_lines << std::endl;
    std::cout << "L1 latency: " << config.l1_latency << " cycles" << std::endl;
    std::cout << "L2 latency: " << config.l2_latency << " cycles" << std::endl;
    std::cout << "Memory latency: " << config.memory_latency << " cycles" << std::endl;
    if (config.tracefile) {
        std::cout << "Trace file: " << config.tracefile << std::endl;
    }
    std::cout << "Input file: " << config.input_file << std::endl;
    // 读取输入文件
    std::vector<Request> requests = read_input_file(config.input_file);
    // 初始化缓存模块
    // 假设您有一个名为 `DataCache` 的缓存模块类
    DataCache l1_cache("L1Cache", config.l1_lines, config.cacheline_size);
    DataCache l2_cache("L2Cache", config.l2_lines, config.cacheline_size);

    //创建（打开）tracefile
    sc_trace_file *traceFile;
    if (config.tracefile != NULL) {
        traceFile = sc_create_vcd_trace_file(config.tracefile);
        //test
        MODULE module("module");
        sc_signal<int> s;
        module.p(s);
        sc_trace(traceFile, s, "signal");
        sc_start(5, SC_SEC);
        //test end here
    }

    // 其他代码，例如加载输入文件，运行模拟等
    for (const Request& req : requests){
        //Todo: 执行模拟
        break;
    }

    sc_close_vcd_trace_file(traceFile);
    return 0;
}
