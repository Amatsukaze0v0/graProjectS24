#include <systemc>
#include <systemc.h>
#include <iostream>
#include <cstdint>


SC_MODULE(DataCache) {
    // Cache的参数
    int dcache_size;
    int dcache_data_per_line;
    int dcache_set;

    // Cache行的结构
    struct DCACHE_LineStruct {
        bool Valid;
        uint32_t Tag;
        std::vector<uint8_t> Data;
    };

    std::vector<DCACHE_LineStruct> DCache;

    // 构造函数，传入外部参数
    //todo: 搞懂
    SC_HAS_PROCESS(DataCache);
    DataCache(sc_module_name name, int size, int line_size) : sc_module(name),
                                                              dcache_size(size),
                                                              dcache_data_per_line(line_size),
                                                              dcache_set(size / line_size),
                                                              DCache(size / line_size, {false, 0, std::vector<uint8_t>(line_size, 0)}) {
        SC_METHOD(init);
        //todo: 搞懂
        dont_initialize();
    }

    // 初始化
    void init() {
        for (int i = 0; i < dcache_set; i++) {
            DCache[i].Valid = false;
        }
        std::cout << "Data Cache initialized with size: " << dcache_size << " bytes, line size: " 
                    << dcache_data_per_line << " bytes, sets: " << dcache_set << std::endl;
    }

    // 从内存加载数据到 Cache
    // 刘： 疑问1 主存在哪？
    //     3.2第一段末尾有一个“eine Liste von Speicherzugriffen”,但是这个类似操作而非内存
    void load_cache_line_from_memory(uint64_t Address, uint32_t CacheLineAddress) {
        uint64_t AlignAddress = Address & ~(dcache_data_per_line - 1);
        for (int i = 0; i < dcache_data_per_line / 8; i++) {
            uint64_t ReadData = read_memory(AlignAddress + 8LL * i);
            std::memcpy(DCache[CacheLineAddress].Data.data() + 8 * i, &ReadData, sizeof(ReadData));
        }
    }

    // 将 Cache 数据写回内存
    void store_cache_line_to_memory(uint64_t Address, uint32_t CacheLineAddress) {
        uint64_t AlignAddress = Address & ~(dcache_data_per_line - 1);
        for (int i = 0; i < dcache_data_per_line / 8; i++) {
            uint64_t WriteData;
            std::memcpy(&WriteData, DCache[CacheLineAddress].Data.data() + 8 * i, sizeof(WriteData));
            write_memory(AlignAddress + 8LL * i, WriteData);
        }
    }

    // 访问 Cache
    uint8_t access_data_cache(uint64_t Address, char Operation, uint8_t DataSize, uint64_t StoreValue, uint64_t* LoadResult) {
        //index
        uint32_t CacheLineAddress = (Address >> static_cast<int>(log2(dcache_data_per_line))) % dcache_set;
        //offset
        uint8_t BlockOffset = Address % dcache_data_per_line;
        //tag
        uint64_t AddressTag = (Address >> static_cast<int>(log2(dcache_data_per_line))) >> static_cast<int>(log2(dcache_set));

        *LoadResult = 0;
        uint8_t MissFlag = 'M';

        if (DCache[CacheLineAddress].Valid && DCache[CacheLineAddress].Tag == AddressTag) {
            MissFlag = 'H';

            if (Operation == 'L') {
                uint64_t ReadValue = 0;
                switch (DataSize) {
                case 1:
                    ReadValue = DCache[CacheLineAddress].Data[BlockOffset];
                    break;
                case 2:
                    BlockOffset &= 0xFE;
                    std::memcpy(&ReadValue, DCache[CacheLineAddress].Data.data() + BlockOffset, 2);
                    break;
                case 4:
                    BlockOffset &= 0xFC;
                    std::memcpy(&ReadValue, DCache[CacheLineAddress].Data.data() + BlockOffset, 4);
                    break;
                case 8:
                    BlockOffset &= 0xF8;
                    std::memcpy(&ReadValue, DCache[CacheLineAddress].Data.data() + BlockOffset, 8);
                    break;
                }
                *LoadResult = ReadValue;
            } else if (Operation == 'S' || Operation == 'M') {
                switch (DataSize) {
                case 1:
                    DCache[CacheLineAddress].Data[BlockOffset] = StoreValue & 0xFF;
                    break;
                case 2:
                    BlockOffset &= 0xFE;
                    std::memcpy(DCache[CacheLineAddress].Data.data() + BlockOffset, &StoreValue, 2);
                    break;
                case 4:
                    BlockOffset &= 0xFC;
                    std::memcpy(DCache[CacheLineAddress].Data.data() + BlockOffset, &StoreValue, 4);
                    break;
                case 8:
                    BlockOffset &= 0xF8;
                    std::memcpy(DCache[CacheLineAddress].Data.data() + BlockOffset, &StoreValue, 8);
                    break;
                }
            }
        } else {
            if (DCache[CacheLineAddress].Valid) {
                uint64_t OldAddress = (DCache[CacheLineAddress].Tag << static_cast<int>(log2(dcache_set)) << static_cast<int>(log2(dcache_data_per_line))) | (static_cast<uint64_t>(CacheLineAddress) << static_cast<int>(log2(dcache_data_per_line)));
                store_cache_line_to_memory(OldAddress, CacheLineAddress);
            }
            load_cache_line_from_memory(Address, CacheLineAddress);
            DCache[CacheLineAddress].Valid = true;
            DCache[CacheLineAddress].Tag = AddressTag;
            if (Operation == 'S' || Operation == 'M') {
                switch (DataSize) {
                case 1:
                    DCache[CacheLineAddress].Data[BlockOffset] = StoreValue & 0xFF;
                    break;
                case 2:
                    BlockOffset &= 0xFE;
                    std::memcpy(DCache[CacheLineAddress].Data.data() + BlockOffset, &StoreValue, 2);
                    break;
                case 4:
                    BlockOffset &= 0xFC;
                    std::memcpy(DCache[CacheLineAddress].Data.data() + BlockOffset, &StoreValue, 4);
                    break;
                case 8:
                    BlockOffset &= 0xF8;
                    std::memcpy(DCache[CacheLineAddress].Data.data() + BlockOffset, &StoreValue, 8);
                    break;
                }
            }
        }
        return MissFlag;
    }

    uint64_t read_memory(uint64_t address) {
        // 这里应实现从模拟内存读取数据的逻辑
        return 0;
    }

    void write_memory(uint64_t address, uint64_t data) {
        // 这里应实现向模拟内存写入数据的逻辑
    }
};

/* // 主测试函数
int sc_main(int argc, char* argv[]) {
    int cache_size = 16384; // 外部输入设置
    int line_size = 64; // 外部输入设置

    DataCache cache("DataCache", cache_size, line_size);
    sc_start();
    return 0;
} */