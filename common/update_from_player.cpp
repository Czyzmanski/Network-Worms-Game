#include <netinet/in.h>
#include <cstring>

#include "update_from_player.h"

UpdateFromPlayer::UpdateFromPlayer(uint64_t session_id, uint8_t turn_direction,
                                   uint32_t next_expected_event_no,
                                   std::string &player_name) {
    this->session_id = session_id;
    this->turn_direction = turn_direction;
    this->next_expected_event_no = next_expected_event_no;
    for (auto &c : player_name) {
        this->player_name.push_back(c);
    }
    this->serialized_size = sizeof(session_id) + sizeof(turn_direction) +
                            sizeof(next_expected_event_no) + player_name.size();
}

UpdateFromPlayer::UpdateFromPlayer(data_t &data) {
    auto *buf = data.data();
    size_t pos = 0;

    memcpy(&session_id, buf, sizeof(session_id));
    pos += sizeof(session_id);

    memcpy(&turn_direction, buf + pos, sizeof(turn_direction));
    pos += sizeof(turn_direction);

    memcpy(&next_expected_event_no, buf + pos, sizeof(next_expected_event_no));
    pos += sizeof(next_expected_event_no);

    size_t player_name_len = data.size() - pos;
    auto *player_name_tmp_buf = new uint8_t[player_name_len];
    memcpy(player_name_tmp_buf, buf + pos, player_name_len);

    player_name.insert(player_name.begin(), player_name_tmp_buf,
                       player_name_tmp_buf + player_name_len);
    delete[] player_name_tmp_buf;

    session_id = be64toh(session_id);
    next_expected_event_no = ntohl(next_expected_event_no);
}

data_t UpdateFromPlayer::serialize() {
    uint64_t session_id_net = htobe64(session_id);
    uint8_t turn_direction_net = turn_direction;
    uint32_t next_expected_event_no_net = htonl(next_expected_event_no);
    player_name_t player_name_net = player_name;

    size_t buf_size = serialized_size;
    auto *buf = new uint8_t[buf_size];
    size_t pos = 0;

    memcpy(buf, &session_id_net, sizeof(session_id_net));
    pos += sizeof(session_id_net);

    memcpy(buf + pos, &turn_direction_net, sizeof(turn_direction_net));
    pos += sizeof(turn_direction_net);

    memcpy(buf + pos, &next_expected_event_no_net, sizeof(next_expected_event_no_net));
    pos += sizeof(next_expected_event_no_net);

    memcpy(buf + pos, player_name_net.data(), player_name_net.size());
    pos += player_name_net.size();

    serialized.insert(serialized.begin(), buf, buf + pos);
    delete[] buf;

    return serialized;
}

u_int64_t UpdateFromPlayer::get_session_id() const { return session_id; }

u_int8_t UpdateFromPlayer::get_turn_direction() const { return turn_direction; }

uint32_t UpdateFromPlayer::get_next_expected_event_no() const {
    return next_expected_event_no;
}

std::string UpdateFromPlayer::get_player_name() const {
    return std::string(player_name.begin(), player_name.end());
}
std::string UpdateFromPlayer::text_repr() const {
    return "session_id: " + std::to_string(session_id) +
           ", turn_direction: " + std::to_string(turn_direction) +
           ", next_expected_event_no: " + std::to_string(next_expected_event_no) +
           ", player_name: " + std::string(player_name.begin(), player_name.end());
}
