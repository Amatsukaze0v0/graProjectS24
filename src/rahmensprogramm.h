#ifndef RAHMENSPROGRAMM_H
#define RAHMENSPROGRAMM_H

typedef struct {
    int cycles;
    int cacheline_size;
    int l1_lines;
    int l2_lines;
    int l1_latency;
    int l2_latency;
    int memory_latency;
    const char* tracefile;
    char *input_file;
} CacheConfig;

int parse_args(int argc, char *argv[], CacheConfig *config);

void print_usage(const char *prog_name);

#endif // RAHMENSPROGRAMM_H
