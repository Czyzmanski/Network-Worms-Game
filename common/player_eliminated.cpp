#include <netinet/in.h>
#include <cstring>

#include "crc32.h"
#include "player_eliminated.h"
#include "utils.h"

PlayerEliminated::PlayerEliminated(uint32_t event_no, uint8_t player_number) {
    this->event_type = PLAYER_ELIMINATED;
    this->event_no = event_no;
    this->player_number = player_number;

    serialized_size = sizeof(len) + sizeof(event_no) + sizeof(event_type) +
                      sizeof(player_number) + sizeof(crc32);
    len = serialized_size - sizeof(len) - sizeof(crc32);
}

PlayerEliminated::PlayerEliminated(data_t &data, uint32_t len,
                                   uint32_t event_no,
                                   player_by_number_t &player_by_number) {
    this->event_type = PLAYER_ELIMINATED;
    this->len = len;
    this->event_no = event_no;

    auto *buf = data.data();
    size_t pos = 0;

    memcpy(&player_number, buf, sizeof(player_number));
    pos += sizeof(player_number);
    if (player_by_number.find(player_number) == player_by_number.end()) {
        print_invalid_value_msg_and_exit("Invalid player number: " + std::to_string(player_number));
    }
    player_name = player_by_number[player_number];

    memcpy(&crc32, buf + pos, sizeof(crc32));
    crc32 = ntohl(crc32);
}

data_t PlayerEliminated::serialize() {
    uint32_t len_net = htonl(len);
    uint32_t event_no_net = htonl(event_no);
    uint8_t event_type_net = event_type;
    uint8_t player_number_net = player_number;

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

    crc32 = compute_crc32(buf, pos);
    uint32_t crc32_net = htonl(crc32);

    memcpy(buf + pos, &crc32_net, sizeof(crc32_net));
    pos += sizeof(crc32_net);

    serialized.insert(serialized.begin(), buf, buf + pos);
    delete[] buf;

    return serialized;
}

std::string PlayerEliminated::text_repr() {
    return "PLAYER_ELIMINATED " + player_name;
}