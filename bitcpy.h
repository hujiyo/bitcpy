#ifndef BITCPY_H
#define BITCPY_H

#include <stdint.h>
#include <string.h>

/**
 * @brief 位级别内存拷贝
 * @param dest      目标缓冲区（指向起始字节），必须非 NULL
 * @param dest_bit  目标起始位偏移（0-7），0 表示最低有效位
 * @param src       源缓冲区（指向起始字节），必须非 NULL
 * @param src_bit   源起始位偏移（0-7），0 表示最低有效位
 * @param len       要拷贝的位数，必须 >= 0
 *
 * === 参数要求（调用者保证） ===
 * - dest 必须是非 NULL 指针
 * - src 必须是非 NULL 指针
 * - dest_bit 必须在 [0, 7] 范围内
 * - src_bit 必须在 [0, 7] 范围内
 * - len 必须 >= 0
 * - dest 缓冲区至少需要 (dest_bit + len + 7) / 8 字节
 * - src 缓冲区至少需要 (src_bit + len + 7) / 8 字节
 * - 当 len > 0 时，dest 和 src 可以指向同一内存区域（支持自拷贝）
 *
 * === 未定义行为 ===
 * 如果违反以下任何条件，行为未定义：
 * 1. dest 为 NULL
 * 2. src 为 NULL
 * 3. dest_bit 不在 [0, 7] 范围内
 * 4. src_bit 不在 [0, 7] 范围内
 * 5. dest 缓冲区空间不足
 * 6. src 缓冲区空间不足
 *
 * === 特殊情况 ===
 * - 当 len == 0 时，函数立即返回，不执行任何操作（即使 dest 或 src 为 NULL）
 * - 当 dest_bit == 0 且 src_bit == 0 且 len 是 8 的倍数时，使用 memcpy 优化
 *
 * === 位序说明 ===
 * 位序为小端序（Little Endian）：
 * - 位 0 是字节的最低有效位（LSB）
 * - 位 7 是字节的最高有效位（MSB）
 *
 * === 底层优化策略 ===
 * 函数采用多阶段优化策略，按从快到慢的顺序处理：
 *
 * 1. 快速路径（Fast Path）
 *    - 条件：dest_bit == 0 && src_bit == 0 && (len % 8 == 0)
 *    - 策略：直接调用 memcpy(dest, src, len/8)
 *    - 优势：利用标准库 memcpy 的 SIMD 优化
 *
 * 2. 阶段1：目标字节对齐
 *    - 目的：使 dest 指针对齐到字节边界（dest_bit = 0）
 *    - 策略：复制 (8 - dest_bit) 位到目标的前导位置
 *    - 优势：后续阶段可以按字节写入，避免位掩码操作
 *
 * 3. 阶段2：64位块处理
 *    - 条件：remaining >= 64 位
 *    - 策略：每次处理 64 位（8字节）
 *    - 实现：
 *      - 如果 src_bit == 0：直接 memcpy 8字节
 *      - 如果 src_bit != 0：读取9字节拼接成64位（需保证剩余 > 64）
 *    - 优势：最大化内存带宽利用率
 *
 * 4. 阶段3：字节级处理
 *    - 条件：remaining >= 8 位
 *    - 策略：每次处理 1 字节
 *    - 实现：
 *      - 如果 src_bit == 0：直接读取1字节
 *      - 如果 src_bit != 0：从2字节拼接
 *    - 优势：处理中等长度的剩余数据
 *
 * 5. 阶段4：剩余位处理
 *    - 条件：remaining > 0 且 < 8
 *    - 策略：使用位掩码提取和写入 1-7 位
 *    - 实现：mask_low[] 和 mask_high[] 查找表
 *    - 优势：精确处理尾部数据
 *
 * === 性能特点 ===
 * - 最优情况：字节对齐且长度为8的倍数，接近 memcpy 性能
 * - 次优情况：目标对齐后的大块数据（>=64位），充分利用内存带宽
 * - 一般情况：通过分阶段处理减少位操作次数
 * - 查找表优化：mask_low[] 和 mask_high[] 避免运行时计算
 */
void bitcpy(uint8_t* dest, uint8_t dest_bit,const uint8_t* src, uint8_t src_bit,uint64_t len);

#endif // BITCPY_H

