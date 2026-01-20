#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bitcpy.h"

#ifdef _WIN32
#include <windows.h>
static double get_time_ms(void) {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1000.0 / (double)freq.QuadPart;
}
#else
#include <sys/time.h>
static double get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}
#endif

#define NUM_SAMPLES 10000
#define NUM_ITERATIONS 10000
#define MAX_BITS 200

// 测试样例结构
typedef struct {
    uint8_t* src;           // 源缓冲区（紧凑位存储）
    uint8_t* src_expanded;  // 源缓冲区（1字节=1位）
    uint8_t* dest;          // 目标缓冲区（紧凑）
    uint8_t* dest_expanded; // 目标缓冲区（1字节=1位）
    uint8_t src_bit;
    uint8_t dest_bit;
    uint64_t len;
    size_t src_bytes;
    size_t dest_bytes;
} TestCase;

static TestCase samples[NUM_SAMPLES];

// 基准实现1：用1字节模拟1位的拷贝（展开存储）
void bitcpy_expanded(uint8_t* dest_expanded, uint8_t dest_bit,
                     const uint8_t* src_expanded, uint8_t src_bit,
                     uint64_t len) {
    // 每个字节存储一个位（0或1）
    // 直接按字节拷贝即可
    memcpy(dest_expanded + dest_bit, src_expanded + src_bit, len);
}

// 基准实现2：真正逐位拷贝（紧凑存储）
void bitcpy_bitwise(uint8_t* dest, uint8_t dest_bit,
                    const uint8_t* src, uint8_t src_bit,
                    uint64_t len) {
    for (uint64_t i = 0; i < len; i++) {
        // 读取源位
        uint64_t src_pos = src_bit + i;
        int bit_val = (src[src_pos >> 3] >> (src_pos & 7)) & 1;
        
        // 写入目标位
        uint64_t dest_pos = dest_bit + i;
        uint64_t byte_idx = dest_pos >> 3;
        uint8_t bit_idx = dest_pos & 7;
        if (bit_val) {
            dest[byte_idx] |= (1 << bit_idx);
        } else {
            dest[byte_idx] &= ~(1 << bit_idx);
        }
    }
}

// 从紧凑格式展开到1字节=1位格式
void expand_bits(const uint8_t* compact, uint8_t bit_offset, 
                 uint8_t* expanded, uint64_t len) {
    for (uint64_t i = 0; i < len; i++) {
        uint64_t bit_pos = bit_offset + i;
        expanded[i] = (compact[bit_pos >> 3] >> (bit_pos & 7)) & 1;
    }
}

// 从1字节=1位格式压缩回紧凑格式
void compact_bits(const uint8_t* expanded, 
                  uint8_t* compact, uint8_t bit_offset, uint64_t len) {
    for (uint64_t i = 0; i < len; i++) {
        uint64_t bit_pos = bit_offset + i;
        uint64_t byte_idx = bit_pos >> 3;
        uint8_t bit_idx = bit_pos & 7;
        if (expanded[i]) {
            compact[byte_idx] |= (1 << bit_idx);
        } else {
            compact[byte_idx] &= ~(1 << bit_idx);
        }
    }
}

// 初始化测试样例
void init_samples(void) {
    srand(12345);  // 固定种子保证可重复
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        TestCase* tc = &samples[i];
        
        tc->src_bit = rand() % 8;
        tc->dest_bit = rand() % 8;
        tc->len = 1 + rand() % MAX_BITS;
        
        tc->src_bytes = (tc->src_bit + tc->len + 7) / 8;
        tc->dest_bytes = (tc->dest_bit + tc->len + 7) / 8;
        
        // 分配紧凑格式缓冲区
        tc->src = (uint8_t*)malloc(tc->src_bytes);
        tc->dest = (uint8_t*)malloc(tc->dest_bytes);
        
        // 分配展开格式缓冲区（1字节=1位）
        tc->src_expanded = (uint8_t*)malloc(tc->src_bit + tc->len);
        tc->dest_expanded = (uint8_t*)malloc(tc->dest_bit + tc->len);
        
        // 随机填充源数据
        for (size_t j = 0; j < tc->src_bytes; j++) {
            tc->src[j] = rand() & 0xFF;
        }
        
        // 展开源数据
        expand_bits(tc->src, tc->src_bit, tc->src_expanded, tc->len);
    }
}

// 释放测试样例
void free_samples(void) {
    for (int i = 0; i < NUM_SAMPLES; i++) {
        free(samples[i].src);
        free(samples[i].dest);
        free(samples[i].src_expanded);
        free(samples[i].dest_expanded);
    }
}

// 运行 bitcpy 性能测试
double benchmark_bitcpy(void) {
    double start = get_time_ms();
    
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        for (int i = 0; i < NUM_SAMPLES; i++) {
            TestCase* tc = &samples[i];
            bitcpy(tc->dest, tc->dest_bit, tc->src, tc->src_bit, tc->len);
        }
    }
    
    double end = get_time_ms();
    return end - start;
}

// 运行 bitwise 性能测试（真正逐位拷贝）
double benchmark_bitwise(void) {
    double start = get_time_ms();
    
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        for (int i = 0; i < NUM_SAMPLES; i++) {
            TestCase* tc = &samples[i];
            bitcpy_bitwise(tc->dest, tc->dest_bit, tc->src, tc->src_bit, tc->len);
        }
    }
    
    double end = get_time_ms();
    return end - start;
}

// 运行 expanded 性能测试（1字节=1位）
double benchmark_expanded(void) {
    double start = get_time_ms();
    
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        for (int i = 0; i < NUM_SAMPLES; i++) {
            TestCase* tc = &samples[i];
            bitcpy_expanded(tc->dest_expanded, tc->dest_bit,
                           tc->src_expanded, tc->src_bit, tc->len);
        }
    }
    
    double end = get_time_ms();
    return end - start;
}

int main(void) {
    printf("Initializing %d test samples...\n", NUM_SAMPLES);
    init_samples();
    
    printf("Running performance benchmark...\n");
    printf("  - Samples: %d\n", NUM_SAMPLES);
    printf("  - Iterations: %d\n", NUM_ITERATIONS);
    printf("  - Total operations: %lld\n\n", 
           (long long)NUM_SAMPLES * NUM_ITERATIONS);
    
    // 预热
    for (int i = 0; i < 100; i++) {
        TestCase* tc = &samples[i % NUM_SAMPLES];
        bitcpy(tc->dest, tc->dest_bit, tc->src, tc->src_bit, tc->len);
        bitcpy_bitwise(tc->dest, tc->dest_bit, tc->src, tc->src_bit, tc->len);
        bitcpy_expanded(tc->dest_expanded, tc->dest_bit,
                       tc->src_expanded, tc->src_bit, tc->len);
    }
    
    // 正式测试
    double time_bitcpy = benchmark_bitcpy();
    double time_bitwise = benchmark_bitwise();
    double time_expanded = benchmark_expanded();
    
    printf("=== Performance Results ===\n");
    printf("bitcpy (optimized):   %8.2f ms\n", time_bitcpy);
    printf("bitwise (bit-by-bit): %8.2f ms\n", time_bitwise);
    printf("expanded (1byte/bit): %8.2f ms\n", time_expanded);
    printf("\n");
    
    printf("bitcpy vs bitwise:  %.2fx faster\n", time_bitwise / time_bitcpy);
    printf("bitcpy vs expanded: %.2fx %s\n", 
           time_bitcpy < time_expanded ? time_expanded / time_bitcpy : time_bitcpy / time_expanded,
           time_bitcpy < time_expanded ? "faster" : "slower");
    
    // 计算内存效率
    uint64_t total_bits = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        total_bits += samples[i].len;
    }
    uint64_t compact_bytes = 0;
    uint64_t expanded_bytes = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        compact_bytes += samples[i].src_bytes + samples[i].dest_bytes;
        expanded_bytes += (samples[i].src_bit + samples[i].len) + 
                          (samples[i].dest_bit + samples[i].len);
    }
    
    printf("\n=== Memory Efficiency ===\n");
    printf("Total bits processed: %llu\n", (unsigned long long)total_bits);
    printf("Compact storage:      %llu bytes\n", (unsigned long long)compact_bytes);
    printf("Expanded storage:     %llu bytes\n", (unsigned long long)expanded_bytes);
    printf("Memory ratio:         %.2fx\n", (double)expanded_bytes / compact_bytes);
    
    free_samples();
    return 0;
}
