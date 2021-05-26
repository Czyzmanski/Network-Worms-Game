#ifndef PROJ2_CRC32_H
#define PROJ2_CRC32_H

#include <cstddef>
#include <cstdint>

uint32_t compute_crc32(const void *buf, size_t size);

#endif  // PROJ2_CRC32_H
