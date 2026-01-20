#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bitcpy.h"

// 获取指定位的值
static inline int get_bit(const uint8_t* buf, uint64_t bit_pos) {
    return (buf[bit_pos >> 3] >> (bit_pos & 7)) & 1;
}

// 设置指定位的值
static inline void set_bit(uint8_t* buf, uint64_t bit_pos, int val) {
    uint64_t byte_idx = bit_pos >> 3;
    uint8_t bit_idx = bit_pos & 7;
    if (val) {
        buf[byte_idx] |= (1 << bit_idx);
    } else {
        buf[byte_idx] &= ~(1 << bit_idx);
    }
}

// 逐位验证
int verify_bitcpy(const uint8_t* src, uint8_t src_bit, 
                  const uint8_t* dest, uint8_t dest_bit, 
                  uint64_t len) {
    for (uint64_t i = 0; i < len; i++) {
        int src_val = get_bit(src, src_bit + i);
        int dest_val = get_bit(dest, dest_bit + i);
        if (src_val != dest_val) {
            printf("  [FAIL] Bit %llu mismatch: src=%d, dest=%d\n", 
                   (unsigned long long)i, src_val, dest_val);
            return 0;
        }
    }
    return 1;
}

// 单次随机测试
int run_random_test(int test_id, unsigned int seed, int verbose) {
    // 设置种子以便重现
    srand(seed);
    
    // 随机参数
    uint8_t src_bit = rand() % 8;
    uint8_t dest_bit = rand() % 8;
    uint64_t len = 1 + rand() % 200;  // 1-200位
    
    // 计算需要的字节数
    size_t src_bytes = (src_bit + len + 7) / 8;
    size_t dest_bytes = (dest_bit + len + 7) / 8;
    
    // 分配缓冲区
    uint8_t* src = (uint8_t*)malloc(src_bytes);
    uint8_t* dest = (uint8_t*)malloc(dest_bytes);
    uint8_t* dest_backup = (uint8_t*)malloc(dest_bytes);
    
    if (!src || !dest || !dest_backup) {
        printf("Memory allocation failed\n");
        free(src);
        free(dest);
        free(dest_backup);
        return 0;
    }
    
    // 随机填充源数据
    for (size_t i = 0; i < src_bytes; i++) {
        src[i] = rand() & 0xFF;
    }
    
    // 随机填充目标数据（用于测试保留未修改位）
    for (size_t i = 0; i < dest_bytes; i++) {
        dest[i] = rand() & 0xFF;
        dest_backup[i] = dest[i];
    }
    
    // 执行 bitcpy
    bitcpy(dest, dest_bit, src, src_bit, len);
    
    // 验证拷贝的位
    int copy_ok = verify_bitcpy(src, src_bit, dest, dest_bit, len);
    
    // 验证未修改的位（dest_bit 之前的位）
    int preserve_ok = 1;
    for (uint8_t i = 0; i < dest_bit; i++) {
        if (get_bit(dest, i) != get_bit(dest_backup, i)) {
            if (verbose) {
                printf("  [FAIL] Preserved bit %d was modified\n", i);
            }
            preserve_ok = 0;
            break;
        }
    }
    
    // 验证未修改的位（dest_bit + len 之后的位）
    uint64_t end_bit = dest_bit + len;
    uint64_t total_bits = dest_bytes * 8;
    for (uint64_t i = end_bit; i < total_bits; i++) {
        if (get_bit(dest, i) != get_bit(dest_backup, i)) {
            if (verbose) {
                printf("  [FAIL] Preserved bit %llu was modified\n", 
                       (unsigned long long)i);
            }
            preserve_ok = 0;
            break;
        }
    }
    
    int success = copy_ok && preserve_ok;
    
    if (!success || verbose) {
        printf("Test #%d: src_bit=%d, dest_bit=%d, len=%llu -> %s\n",
               test_id, src_bit, dest_bit, (unsigned long long)len,
               success ? "PASS" : "FAIL");
    }
    
    free(src);
    free(dest);
    free(dest_backup);
    
    return success;
}

int main() {
    const int NUM_TESTS = 10000;
    int passed = 0;
    int failed = 0;
    
    // 使用时间作为基础种子
    unsigned int base_seed = (unsigned int)time(NULL);
    
    printf("Running %d random bitcpy tests (seed=%u)...\n\n", NUM_TESTS, base_seed);
    
    for (int i = 1; i <= NUM_TESTS; i++) {
        unsigned int test_seed = base_seed + i;
        
        if (run_random_test(i, test_seed, 0)) {
            passed++;
        } else {
            failed++;
            // 失败时重新运行一次并显示详细信息
            printf("Reproducing failed test #%d with details (seed=%u):\n", i, test_seed);
            run_random_test(i, test_seed, 1);
        }
        
        // 进度显示
        if (i % 1000 == 0) {
            printf("Progress: %d/%d tests completed\n", i, NUM_TESTS);
        }
    }
    
    printf("\n=== Test Results ===\n");
    printf("Total:  %d\n", NUM_TESTS);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    printf("Success rate: %.2f%%\n", (passed * 100.0) / NUM_TESTS);
    
    return failed == 0 ? 0 : 1;
}
