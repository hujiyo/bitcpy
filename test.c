#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitcpy.h"

void test_bitcpy() {
    printf("Testing bitcpy...\n");
    
    // Test Case 1: Byte-aligned fast path
    uint8_t src1[] = {0xAB, 0xCD, 0xEF, 0x12};
    uint8_t dst1[4] = {0};
    
    int result = bitcpy(dst1, 0, src1, 0, 32, 4, 4);
    printf("Test 1 - Aligned copy: %s\n", 
           (result == 0 && memcmp(dst1, src1, 4) == 0) ? "PASS" : "FAIL");
    
    // Test Case 2: Unaligned bit copy
    uint8_t src2[] = {0b11111111, 0b00000000, 0b11111111}; // 0xFF, 0x00, 0xFF
    uint8_t dst2[3] = {0};
    // Expected: copy 10 bits from src2[0] bit 1 (0b1111111) and src2[1] bit 0-2 (0b00)
    // The 10 bits are: 1111111000
    // Write to dst2 from bit 3.
    // dst2[0] = 00011111
    // dst2[1] = 11000000
    // Correct values are not simple to check without a "get_bit" function.
    // We will just check if it runs without error.
    result = bitcpy(dst2, 3, src2, 1, 10, 3, 3);
    printf("Test 2 - Unaligned copy: %s\n", (result == 0) ? "PASS" : "FAIL");
     // A more detailed check for test 2
    printf("  - Result dst2[0]: 0x%02X\n", dst2[0]);
    printf("  - Result dst2[1]: 0x%02X\n", dst2[1]);
    // Expected: dst2 should be {0b11111000, 0b00000011, 0x00} = {0xF8, 0x03, 0x00}
    if (dst2[0] == 0xF8 && dst2[1] == 0x03) {
        printf("  - Detailed check: PASS\n");
    } else {
        printf("  - Detailed check: FAIL\n");
    }


    // Test Case 3: Boundary check
    uint8_t src3[] = {0xFF};
    uint8_t dst3[1] = {0};
    
    result = bitcpy(dst3, 0, src3, 0, 16, 1, 1); // Should fail
    printf("Test 3 - Boundary check (overflow): %s\n", (result == -2) ? "PASS" : "FAIL");
    
    result = bitcpy(dst3, 8, src3, 0, 1, 1, 1); // Should fail
    printf("Test 3 - Boundary check (dest start): %s\n", (result == -2) ? "PASS" : "FAIL");

    // Test Case 4: Parameter check
    result = bitcpy(NULL, 0, src3, 0, 8, 1, 1); // Should fail
    printf("Test 4 - Parameter check (null dest): %s\n", (result == -1) ? "PASS" : "FAIL");

    result = bitcpy(dst3, 0, NULL, 0, 8, 1, 1); // Should fail
    printf("Test 4 - Parameter check (null src): %s\n", (result == -1) ? "PASS" : "FAIL");

    result = bitcpy(dst3, 0, src3, 0, 0, 1, 1); // Should fail
    printf("Test 4 - Parameter check (zero length): %s\n", (result == -1) ? "PASS" : "FAIL");
    
    printf("\nAll tests completed.\n");
}

int main() {
    test_bitcpy();
    return 0;
}