/*
#CopyRight (c) HuJi 2025.1
#Under MIT License
#Email:Mhuixs@outlook.com
*/
#ifndef BITCPY_H
#define BITCPY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <assert.h>

int bitcpy(uint8_t* destination, uint64_t dest_first_bit, 
        const uint8_t* source, uint64_t source_first_bit, 
            uint64_t len, uint64_t dest_buffer_size, 
            uint64_t source_buffer_size);

void bitcpy_fast(uint8_t* destination, uint64_t dest_first_bit, 
                 const uint8_t* source, uint64_t source_first_bit, 
                 uint64_t len);

#ifdef __cplusplus
}
#endif

#endif // BITCPY_H

