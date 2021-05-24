#include "pixel.h"
#include <netinet/in.h>
#include <cstring>
#include "crc32.h"

Pixel::Pixel(uint32_t event_no, uint8_t player_number, std::string &player_name,
             uint32_t x, uint32_t y) {
    this->event_type = PIXEL;
    this->event_no = event_no;
    this->player_number = player_number;
    this->player_name = player_name;
    this->x = x;
    this->y = y;

    serialized_size = sizeof(len) + sizeof(event_no) + sizeof(event_type) +
                      sizeof(player_number) + sizeof(x) + sizeof(y) +
                      sizeof(crc32);
    len = serialized_size - sizeof(len) - sizeof(crc32);
}

Pixel::Pixel(data_t &data, uint32_t len, uint32_t event_no,
             uint8_t player_number, std::string &player_name) {
    this->event_type = PIXEL;
    this->len = len;
    this->event_no = event_no;
    this->player_number = player_number;
    this->player_name = player_name;

    auto *buf = data.data();
    size_t pos = 0;

    memcpy(&x, buf, sizeof(x));
    pos += sizeof(x);

    memcpy(&y, buf + pos, sizeof(y));
    pos += sizeof(y);

    memcpy(&crc32, buf + pos, sizeof(crc32));

    x = ntohl(x);
    y = ntohl(y);
    crc32 = ntohl(crc32);
}

data_t Pixel::serialize() {
    uint32_t len_net = htonl(len);
    uint32_t event_no_net = htonl(event_no);
    uint8_t event_type_net = event_type;
    uint8_t player_number_net = player_number;
    uint32_t x_net = htonl(x);
    uint32_t y_net = htonl(y);

    size_t buf_size = serialized_size;
    auto *buf = new uint8_t[buf_size];
    size_t pos = 0;

    memcpy(buf, &len_net, sizeof(len_net));
    pos += sizeof(len_net);

    memcpy(buf + pos, &event_no_net, sizeof(event_no_net));
    pos += sizeof(event_no_net);

    memcpy(buf + pos, &event_type_net, sizeof(event_type_net));
    pos += sizeof(event_type_net);

    memcpy(buf + pos, &player_number_net, sizeof(player_number_net));
    pos += sizeof(player_number_net);

    memcpy(buf + pos, &x_net, sizeof(x_net));
    pos += sizeof(x_net);

    memcpy(buf + pos, &y_net, sizeof(y_net));
    pos += sizeof(y_net);

    crc32 = compute_crc32(buf, pos);
    uint32_t crc32_net = htonl(crc32);

    memcpy(buf + pos, &crc32_net, sizeof(crc32_net));
    pos += sizeof(crc32_net);

    serialized.insert(serialized.begin(), buf, buf + pos);
    delete[] buf;

    return serialized;
}

std::string Pixel::text_repr() {
    return "PIXEL " + std::to_string(x) + " " + std::to_string(y) + " " +
           player_name;
}