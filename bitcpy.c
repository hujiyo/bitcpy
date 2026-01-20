/*
#版权所有 (c) HUJI 2024
#许可证协议:MIT
Email: Mhuixs.db@outlook.com
*/

#include "bitcpy.h"

void bitcpy(uint8_t* dest, uint8_t dest_bit,const uint8_t* src, uint8_t src_bit,uint64_t len){    
    if (len == 0) return;
    if ((src_bit | dest_bit) == 0 && (len & 7) == 0) {
        memcpy(dest, src, len >> 3);
        return;
    }
    static const uint8_t mask_low[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F};
    static const uint8_t mask_high[8]= {0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80};
    uint64_t remaining = len;
    
    // 阶段1：处理前导位，使目标对齐到字节边界
    if (dest_bit != 0) {
        uint8_t align_bits = 8 - dest_bit;
        if (align_bits > remaining) align_bits = (uint8_t)remaining;
        
        // 提取源数据（可能跨字节）
        uint16_t src_data = src[0];
        if (src_bit + align_bits > 8) {
            src_data |= (uint16_t)src[1] << 8;
        }
        src_data >>= src_bit;
        src_data &= mask_low[align_bits];
        
        // 写入目标（保留未修改的位）
        uint8_t write_mask = mask_low[align_bits] << dest_bit;
        dest[0] = (dest[0] & ~write_mask) | 
                  ((uint8_t)src_data << dest_bit);
        
        remaining -= align_bits;
        if (remaining == 0) return;
        
        // 更新指针和偏移
        dest++;
        uint8_t src_advance = src_bit + align_bits;
        src += src_advance >> 3;
        src_bit = src_advance & 7;
    }
    
    // 此时 dest 已字节对齐
    // 计算源可读字节数：(src_bit + remaining + 7) / 8
    // 64位块需要读取8字节（对齐）或9字节（非对齐）
    
    // 阶段2：64位块处理
    while (remaining >= 64) {
        uint64_t data;
        if (src_bit == 0) {
            // 源对齐：直接读取8字节
            memcpy(&data, src, 8);
        } else {
            // 源非对齐：需要读取9字节拼接
            // 检查：remaining >= 64 意味着至少还有64位
            // 从当前src读取需要 ceil((src_bit + 64) / 8) = (src_bit + 64 + 7) / 8 字节
            // 当 src_bit > 0 时，需要9字节，但我们只能保证 (src_bit + 64 - 1) / 8 + 1 字节可用
            // 即 (src_bit + 63) / 8 + 1 = 7 或 8 或 9 字节
            // 为安全起见，当 remaining 刚好等于64且src_bit!=0时，退回字节处理
            if (remaining == 64) break;
            
            uint64_t part1;
            memcpy(&part1, src, 8);
            uint8_t part2 = src[8];
            data = (part1 >> src_bit) | ((uint64_t)part2 << (64 - src_bit));
        }
        
        memcpy(dest, &data, 8);
        
        dest += 8;
        src += 8;
        remaining -= 64;
    }
    
    // 阶段3：字节级处理
    while (remaining >= 8) {
        uint8_t data;
        if (src_bit == 0) {
            data = *src;
        } else {
            // 需要从两个字节拼接
            // 检查：remaining >= 8 且 src_bit > 0
            // 需要 ceil((src_bit + 8) / 8) = 2 字节
            // 只有当 remaining == 8 且 src_bit > 0 时，第二字节可能只有部分有效
            // 但调用者保证了 src_bit + remaining 范围内的位都是有效的
            // 所以可以安全读取
            data = (src[0] >> src_bit) | (src[1] << (8 - src_bit));
        }
        
        *dest = data;
        
        dest++;
        src++;
        remaining -= 8;
    }
    
    // 阶段4：处理剩余位（1-7位）
    if (remaining > 0) {
        // 提取源数据
        uint16_t src_data = src[0];
        if (src_bit + remaining > 8) {
            src_data |= (uint16_t)src[1] << 8;
        }
        src_data >>= src_bit;
        src_data &= mask_low[remaining];
        
        // 写入目标（dest已对齐，dest_bit为0）
        // 保留高位，修改低位
        dest[0] = (dest[0] & mask_high[remaining]) | (uint8_t)src_data;
    }
}
