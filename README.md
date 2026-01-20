# bitcpy

## EN
This is a tiny C implementation library for copying bits between buffers with arbitrary bit-level offsets. It provides a robust `bitcpy` function with boundary checks for performance-critical scenarios where buffer sizes are known to be safe. This library is ideal for bit-level data manipulation.

- `bitcpy.c` and `bitcpy.h`: Core library files implementing the `bitcpy` function for efficient bit-level copying.
- `test.c`: Test program for benchmarking the performance of `bitcpy` against other implementations.
- `check.c`: Test program for verifying the correctness of the `bitcpy` function through random tests.

Email: Mhuixs.db@outlook.com

### Benchmark

Test environment: 10,000 samples × 10,000 iterations, total 100 million operations

| Implementation | Time | Relative Performance |
|---------------|------|----------------------|
| **bitcpy (optimized)** | 2163 ms | Baseline |
| bitwise (bit-by-bit) | 15172 ms | 7.01x slower |
| expanded (1byte/bit) | 1326 ms | 1.63x faster |

**Memory Efficiency**: Compact storage saves **7.74x** memory compared to expanded storage

- **bitcpy** is **7x** faster than bit-by-bit copying while maintaining compact storage
- Expanded storage is faster but uses **8 times** more memory than compact storage

## CN

这是一个用于在缓冲区之间以任意位级偏移复制位的微型C实现库。它提供了一个带有边界检查的健壮的`bitcpy`函数，用于缓冲区大小已知安全的性能关键场景。这个库非常适合位级的数据操作。

- `bitcpy.c` 和 `bitcpy.h`：核心库文件，实现高效的位级拷贝函数`bitcpy`。
- `test.c`：测试程序，用于基准测试`bitcpy`与其他实现的性能。
- `check.c`：通过随机测试验证`bitcpy`函数正确性的测试程序。

Email: Mhuixs.db@outlook.com

### Benchmark

测试环境：10000 样例 × 10000 次迭代，共 1 亿次操作

| 实现 | 耗时 | 相对性能 |
|------|------|----------|
| **bitcpy (optimized)** | 2163 ms | 基准 |
| bitwise (bit-by-bit) | 15172 ms | 7.01x 慢 |
| expanded (1byte/bit) | 1326 ms | 1.63x 快 |

**内存效率**：紧凑存储比展开存储节省 **7.74x** 内存

- **bitcpy** 比逐位拷贝快 **7x**，同时保持紧凑存储
- 展开存储虽然更快，但内存占用是紧凑存储的 **8 倍**