#include "crc32.h"

/* Based on: https://lxp32.github.io/docs/a-simple-example-crc32-calculation/ */
uint32_t compute_crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        uint8_t d = data[i];
        for (size_t j = 0; j < 8; j++) {
            uint32_t b = (d ^ crc) & 1;
            crc >>= 1;
            if (b) {
                crc = crc ^ 0xEDB88320;
            }
            d >>= 1;
        }
    }

    return ~crc;
}