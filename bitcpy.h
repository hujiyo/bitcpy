#ifndef BITCPY_H
#define BITCPY_H

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

#endif // BITCPY_H

