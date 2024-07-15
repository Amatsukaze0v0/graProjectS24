#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "rahmensprogramm.h"

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [options] <input_file>\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -c, --cycles <num>          Number of cycles (default: 1000)\n");
    fprintf(stderr, "  --cacheline-size <num>      Size of a cache line in bytes (default: 64)\n");
    fprintf(stderr, "  --l1-lines <num>            Number of lines in L1 cache (default: 128)\n");
    fprintf(stderr, "  --l2-lines <num>            Number of lines in L2 cache (default: 256)\n");
    fprintf(stderr, "  --l1-latency <num>          L1 cache latency in cycles (default: 1)\n");
    fprintf(stderr, "  --l2-latency <num>          L2 cache latency in cycles (default: 5)\n");
    fprintf(stderr, "  --memory-latency <num>      Main memory latency in cycles (default: 50)\n");
    fprintf(stderr, "  --tf <filename>             Output trace file name\n");
    fprintf(stderr, "  -h, --help                  Display this help message\n");
}

int parse_args(int argc, char *argv[], CacheConfig *config) {
    int opt;
    int option_index = 0;

    static struct option long_options[] = {
        {"cycles", required_argument, 0, 'c'},
        {"cacheline-size", required_argument, 0, 0},
        {"l1-lines", required_argument, 0, 0},
        {"l2-lines", required_argument, 0, 0},
        {"l1-latency", required_argument, 0, 0},
        {"l2-latency", required_argument, 0, 0},
        {"memory-latency", required_argument, 0, 0},
        {"tf", required_argument, 0, 0},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    // 设置默认值
    config->cycles = 1000;
    config->cacheline_size = 64;
    config->l1_lines = 128;
    config->l2_lines = 256;
    config->l1_latency = 1;
    config->l2_latency = 5;
    config->memory_latency = 50;
    config->tracefile = NULL;
    config->input_file = NULL;

    while ((opt = getopt_long(argc, argv, "c:h", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'c':
            config->cycles = atoi(optarg);
            break;
        case 'h':
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        case 0:
            if (strcmp(long_options[option_index].name, "cacheline-size") == 0) {
                config->cacheline_size = atoi(optarg);
            } else if (strcmp(long_options[option_index].name, "l1-lines") == 0) {
                config->l1_lines = atoi(optarg);
            } else if (strcmp(long_options[option_index].name, "l2-lines") == 0) {
                config->l2_lines = atoi(optarg);
            } else if (strcmp(long_options[option_index].name, "l1-latency") == 0) {
                config->l1_latency = atoi(optarg);
            } else if (strcmp(long_options[option_index].name, "l2-latency") == 0) {
                config->l2_latency = atoi(optarg);
            } else if (strcmp(long_options[option_index].name, "memory-latency") == 0) {
                config->memory_latency = atoi(optarg);
            } else if (strcmp(long_options[option_index].name, "tf") == 0) {
                config->tracefile = optarg;
            }
            break;
        default:
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        config->input_file = argv[optind];
        if(strlen(config->input_file) < 4 || strcmp(config->input_file + strlen(config->input_file) - 4, ".csv") != 0) {
            fprintf(stderr, "Input file with wrong format.\n");
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        }
        printf("Input file: %s\n", config->input_file);
    } else {
        fprintf(stderr, "Input file is required\n");
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
