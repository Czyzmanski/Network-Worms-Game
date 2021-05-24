#include "new_game.h"
#include <netinet/in.h>
#include <cstring>
#include "crc32.h"

NewGame::NewGame(uint32_t event_no, uint32_t maxx, uint32_t maxy,
                 players_t &players) {
    this->event_type = NEW_GAME;
    this->event_no = event_no;
    this->maxx = maxx;
    this->maxy = maxy;
    this->players = players;

    serialized_size = sizeof(len) + sizeof(event_no) + sizeof(event_type) +
                      sizeof(maxx) + sizeof(maxy) + sizeof(crc32);
    for (auto &player : players) {
        serialized_size += player.length() + 1;
    }

    len = serialized_size - sizeof(len) - sizeof(crc32);
}

NewGame::NewGame(data_t &data, uint32_t len, uint32_t event_no) {
    this->event_type = NEW_GAME;
    this->len = len;
    this->event_no = event_no;

    auto *buf = data.data();
    size_t pos = 0;
    size_t end = len - sizeof(event_no) - sizeof(event_type);

    memcpy(&maxx, buf, sizeof(maxx));
    pos += sizeof(maxx);

    memcpy(&maxy, buf + pos, sizeof(maxy));
    pos += sizeof(maxy);

    while (pos < end) {
        std::string player;
        while (pos < end && buf[pos] != '\0') {
            player += (char)buf[pos];
            pos++;
        }
        players.push_back(player);

        pos++;
    }

    if (pos > end) {
        pos--;
    }

    memcpy(&crc32, buf + pos, sizeof(crc32));

    maxx = ntohl(maxx);
    maxy = ntohl(maxy);
    crc32 = ntohl(crc32);
}

data_t NewGame::serialize() {
    uint32_t len_net = htonl(len);
    uint32_t event_no_net = htonl(event_no);
    uint8_t event_type_net = event_type;
    uint32_t maxx_net = htonl(maxx);
    uint32_t maxy_net = htonl(maxy);

    size_t buf_size = serialized_size;
    auto *buf = new uint8_t[buf_size];
    size_t pos = 0;

    memcpy(buf, &len_net, sizeof(len_net));
    pos += sizeof(len_net);

    memcpy(buf + pos, &event_no_net, sizeof(event_no_net));
    pos += sizeof(event_no_net);

    memcpy(buf + pos, &event_type_net, sizeof(event_type_net));
    pos += sizeof(event_type_net);

    memcpy(buf + pos, &maxx_net, sizeof(maxx_net));
    pos += sizeof(maxx_net);

    memcpy(buf + pos, &maxy_net, sizeof(maxy_net));
    pos += sizeof(maxy_net);

    char null_ch = '\0';
    for (auto &player : players) {
        memcpy(buf + pos, player.c_str(), player.length());
        pos += player.length();
        memcpy(buf + pos, &null_ch, sizeof(null_ch));
        pos += sizeof(null_ch);
    }

    crc32 = compute_crc32(buf, pos);
    uint32_t crc32_net = htonl(crc32);

    memcpy(buf + pos, &crc32_net, sizeof(crc32_net));
    pos += sizeof(crc32_net);

    serialized.insert(serialized.begin(), buf, buf + pos);
    delete[] buf;

    return serialized;
}

std::string NewGame::text_repr() {
    std::string repr =
        "NEW_GAME " + std::to_string(maxx) + " " + std::to_string(maxy);
    for (auto &player : players) {
        repr += " " + player;
    }

    return repr;
}
