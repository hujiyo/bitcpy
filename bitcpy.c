#include "bitcpy.h"
/*
#版权所有 (c) HUJI 2024
#许可证协议:MIT
Email: Mhuixs.db@outlook.com
*/
/**
 * @param destination 目标缓冲区
 * @param dest_first_bit 目标起始位位置
 * @param source 源缓冲区
 * @param source_first_bit 源起始位位置
 * @param len 要拷贝的位数
 * @param dest_buffer_size 目标缓冲区大小（字节）
 * @param source_buffer_size 源缓冲区大小（字节）
 * @return 0 成功，-1 参数错误，-2 缓冲区越界
 */
int bitcpy(uint8_t* destination, uint64_t dest_first_bit, 
        const uint8_t* source, uint64_t source_first_bit, 
            uint64_t len, uint64_t dest_buffer_size, 
            uint64_t source_buffer_size) {
    
    // 参数有效性检查
    if (!destination || !source || !len) {
        return -1;
    }
    
    // 缓冲区边界检查
    uint64_t dest_last_bit = dest_first_bit + len - 1;
    uint64_t source_last_bit = source_first_bit + len - 1;
    
    if ((dest_last_bit >> 3) >= dest_buffer_size || 
        (source_last_bit >> 3) >= source_buffer_size) {
        return -2;
    }
    
    // 快速路径：源和目标都字节对齐且长度是8的倍数
    if (((source_first_bit | dest_first_bit) & 7) == 0 && (len & 7) == 0) {
        memcpy(destination + (dest_first_bit >> 3), 
               source + (source_first_bit >> 3), 
               len >> 3);
        return 0;
    }
    
    // 位掩码查找表
    static const uint8_t bit_mask_table[9] = {
        0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF
    };
    
    uint64_t bits_copied = 0;
    
    // 阶段1：处理前导位，使目标对齐到字节边界
    if ((dest_first_bit & 7) != 0) {
        uint8_t align_bits = 8 - (dest_first_bit & 7);
        if (align_bits > len) align_bits = len;
        
        uint64_t s_byte_idx = (source_first_bit + bits_copied) >> 3;
        uint8_t s_bit_offset = (source_first_bit + bits_copied) & 7;
        uint64_t d_byte_idx = (dest_first_bit + bits_copied) >> 3;
        uint8_t d_bit_offset = (dest_first_bit + bits_copied) & 7;
        
        // 安全边界检查
        if (s_byte_idx >= source_buffer_size || d_byte_idx >= dest_buffer_size) {
            return -2;
        }
        
        // 提取源数据
        uint16_t src_data = source[s_byte_idx];
        if (s_bit_offset + align_bits > 8 && s_byte_idx + 1 < source_buffer_size) {
            src_data |= (uint16_t)source[s_byte_idx + 1] << 8;
        }
        src_data >>= s_bit_offset;
        src_data &= bit_mask_table[align_bits];
        
        // 写入目标
        uint8_t dest_mask = bit_mask_table[align_bits] << d_bit_offset;
        destination[d_byte_idx] = (destination[d_byte_idx] & ~dest_mask) | 
                                  ((src_data << d_bit_offset) & dest_mask);
        
        bits_copied += align_bits;
    }
    
    // 阶段2：64位块操作（目标已字节对齐）
    while (bits_copied + 64 <= len) {
        uint64_t s_bit_pos = source_first_bit + bits_copied;
        uint64_t d_bit_pos = dest_first_bit + bits_copied;
        
        uint64_t s_byte_idx = s_bit_pos >> 3;
        uint8_t s_bit_offset = s_bit_pos & 7;
        uint64_t d_byte_idx = d_bit_pos >> 3;
        
        // 边界检查
        if (s_byte_idx + 8 >= source_buffer_size || d_byte_idx + 8 > dest_buffer_size) {
            break; // 退回到字节级处理
        }
        
        uint64_t data;
        
        if (s_bit_offset == 0) {
            // 源也是字节对齐的，直接复制
            memcpy(&data, source + s_byte_idx, 8);
        } else {
            // 源未对齐，需要位移操作
            uint64_t part1, part2 = 0;
            memcpy(&part1, source + s_byte_idx, 8);
            
            if (s_byte_idx + 8 < source_buffer_size) {
                part2 = source[s_byte_idx + 8];
            }
            
            data = (part1 >> s_bit_offset) | (part2 << (64 - s_bit_offset));
        }
        
        memcpy(destination + d_byte_idx, &data, 8);
        bits_copied += 64;
    }
    
    // 阶段3：字节级处理
    while (bits_copied + 8 <= len) {
        uint64_t s_bit_pos = source_first_bit + bits_copied;
        uint64_t d_bit_pos = dest_first_bit + bits_copied;
        
        uint64_t s_byte_idx = s_bit_pos >> 3;
        uint8_t s_bit_offset = s_bit_pos & 7;
        uint64_t d_byte_idx = d_bit_pos >> 3;
        
        // 边界检查
        if (s_byte_idx >= source_buffer_size || d_byte_idx >= dest_buffer_size) {
            break; // 退回到位级处理
        }
        
        uint8_t data;
        if (s_bit_offset == 0) {
            data = source[s_byte_idx];
        } else {
            uint16_t extended = source[s_byte_idx];
            if (s_byte_idx + 1 < source_buffer_size) {
                extended |= (uint16_t)source[s_byte_idx + 1] << 8;
            }
            data = extended >> s_bit_offset;
        }
        
        destination[d_byte_idx] = data;
        bits_copied += 8;
    }
    
    // 阶段4：处理剩余位
    if (bits_copied < len) {
        uint64_t remaining = len - bits_copied;
        
        uint64_t s_bit_pos = source_first_bit + bits_copied;
        uint64_t d_bit_pos = dest_first_bit + bits_copied;
        
        uint64_t s_byte_idx = s_bit_pos >> 3;
        uint8_t s_bit_offset = s_bit_pos & 7;
        uint64_t d_byte_idx = d_bit_pos >> 3;
        uint8_t d_bit_offset = d_bit_pos & 7;
        
        // 边界检查
        if (s_byte_idx >= source_buffer_size || d_byte_idx >= dest_buffer_size) {
            return -2;
        }
        
        if (remaining <= 8) {
            // 批量处理剩余位
            uint16_t src_data = source[s_byte_idx];
            if (s_bit_offset + remaining > 8 && s_byte_idx + 1 < source_buffer_size) {
                src_data |= (uint16_t)source[s_byte_idx + 1] << 8;
            }
            src_data >>= s_bit_offset;
            src_data &= bit_mask_table[remaining];
            
            uint8_t dest_mask = bit_mask_table[remaining] << d_bit_offset;
            destination[d_byte_idx] = (destination[d_byte_idx] & ~dest_mask) | 
                                      ((src_data << d_bit_offset) & dest_mask);
        } else {
            // 逐位处理（安全回退）
            while (bits_copied < len) {
                uint64_t s_bit_pos = source_first_bit + bits_copied;
                uint64_t d_bit_pos = dest_first_bit + bits_copied;
                
                uint64_t s_byte_idx = s_bit_pos >> 3;
                uint8_t s_bit_idx = s_bit_pos & 7;
                uint64_t d_byte_idx = d_bit_pos >> 3;
                uint8_t d_bit_idx = d_bit_pos & 7;
                
                if (s_byte_idx >= source_buffer_size || d_byte_idx >= dest_buffer_size) {
                    return -2;
                }
                
                uint8_t bit_val = (source[s_byte_idx] >> s_bit_idx) & 1;
                uint8_t mask = 1 << d_bit_idx;
                destination[d_byte_idx] = (destination[d_byte_idx] & ~mask) | 
                                          (bit_val << d_bit_idx);
                bits_copied++;
            }
        }
    }
    
    return 0;
}

// 便利函数：不需要显式缓冲区大小检查的版本（用于已知安全的场景）
void bitcpy_fast(uint8_t* destination, uint64_t dest_first_bit, 
                 const uint8_t* source, uint64_t source_first_bit, 
                 uint64_t len) {
    
    // 使用一个足够大的缓冲区大小来避免检查开销
    // 调用者必须确保缓冲区足够大
    uint64_t large_size = UINT64_MAX;
    bitcpy(destination, dest_first_bit, source, source_first_bit, 
                     len, large_size, large_size);
}
