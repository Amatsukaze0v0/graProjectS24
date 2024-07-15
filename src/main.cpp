#include <iostream>
#include <fstream>
#include <string>
#include <systemc>
#include <systemc.h>
#include "cache.hpp"  // 假设您的缓存实现放在这个头文件中
#include "rahmensprogramm.h"  // 包含命令行解析的头文件



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

    // 初始化缓存模块
    // 假设您有一个名为 `DataCache` 的缓存模块类
    DataCache l1_cache("L1Cache", config.l1_lines, config.cacheline_size);
    DataCache l2_cache("L2Cache", config.l2_lines, config.cacheline_size);

    // 其他代码，例如加载输入文件，运行模拟等

    return 0;
}
