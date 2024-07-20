#include <iostream>
#include <fstream>
#include <string>
#include <systemc>
#include <systemc.h>
#include <sstream>
#include <vector>
#include "Cache.hpp"  // 假设您的缓存实现放在这个头文件中
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

    file.close();
}


// 函数用于读取并解析输入文件
Request* read_input_file(const std::string& filename, size_t* num_requests) {
    std::cout << "Successfully in the CSV function." << std::endl;
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

    // 分配动态内存并复制请求
    *num_requests = requests.size();
    Request* requests_array = (Request*)malloc(*num_requests * sizeof(Request));
    if (requests_array == nullptr) {
        std::cerr << "Failed to allocate memory for requests" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::copy(requests.begin(), requests.end(), requests_array);

    std::cout << "Request Array complete." << std::endl;

    return requests_array;
}

void write_requests_to_csv(const std::string& filename, Request* requests, size_t num_requests) {
    std::ofstream outfile(filename);
    for (size_t i = 0; i < num_requests; ++i) {
        outfile << (requests[i].we ? "W," : "R,")
                << requests[i].addr << ","
                << requests[i].data << std::endl;
    }
    outfile.close();
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
    size_t num_requests;
    Request* requests = read_input_file(config.input_file, &num_requests);


    // 其他代码，例如加载输入文件，运行模拟等
    struct Result result;
    //cycle 不满足时的处理，直接重新执行simulation
    if (100 * num_requests > config.cycles ) {
        result = run_simulation(0x7FFFFFFF, config.l1_lines, config.l2_lines, config.cacheline_size, 
                    config.l1_latency, config.l2_latency, config.memory_latency,
                    num_requests, requests, config.tracefile ? config.tracefile : nullptr);
    } else {
        result = run_simulation(config.cycles, config.l1_lines, config.l2_lines, config.cacheline_size, 
                    config.l1_latency, config.l2_latency, config.memory_latency,
                    num_requests, requests, config.tracefile ? config.tracefile : nullptr);
    }
    //写回csv文件，证明data已更新入request中（写回csv无要求）
    write_requests_to_csv(config.input_file, requests, num_requests);
    std::cout << "Cycles: " << result.cycles << std::endl;
    std::cout << "Misses: " << result.misses << std::endl;
    std::cout << "Hits: " << result.hits << std::endl;
    std::cout << "Primitive Gate Count: " << result.primitiveGateCount << std::endl;

    return 0;
}