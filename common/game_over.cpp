#include <netinet/in.h>
#include <cstring>

#include "crc32.h"
#include "game_over.h"

GameOver::GameOver(uint32_t event_no) {
    this->event_type = GAME_OVER;
    this->event_no = event_no;

    serialized_size = sizeof(len) + sizeof(event_no) + sizeof(event_type) + sizeof(crc32);
    len = serialized_size - sizeof(len) - sizeof(crc32);
}

GameOver::GameOver(data_t &data, uint32_t len, uint32_t event_no) {
    this->event_type = GAME_OVER;
    this->len = len;
    this->event_no = event_no;

    auto *buf = data.data();
    memcpy(&crc32, buf, sizeof(crc32));
    crc32 = ntohl(crc32);
}

data_t GameOver::serialize() {
    uint32_t len_net = htonl(len);
    uint32_t event_no_net = htonl(event_no);
    uint8_t event_type_net = event_type;

    size_t buf_size = serialized_size;
    auto *buf = new uint8_t[buf_size];
    size_t pos = 0;

    memcpy(buf, &len_net, sizeof(len_net));
    pos += sizeof(len_net);

    memcpy(buf + pos, &event_no_net, sizeof(event_no_net));
    pos += sizeof(event_no_net);

    memcpy(buf + pos, &event_type_net, sizeof(event_type_net));
    pos += sizeof(event_type_net);

    crc32 = compute_crc32(buf, pos);
    uint32_t crc32_net = htonl(crc32);

    memcpy(buf + pos, &crc32_net, sizeof(crc32_net));
    pos += sizeof(crc32_net);

    serialized.insert(serialized.begin(), buf, buf + pos);
    delete[] buf;

    return serialized;
}

std::string GameOver::text_repr() { return "GAME_OVER"; }