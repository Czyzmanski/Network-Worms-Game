#ifndef PROJ2_UPDATE_FROM_PLAYER_H
#define PROJ2_UPDATE_FROM_PLAYER_H


#include "event.h"


class UpdateFromPlayer : public Event {
public:
    UpdateFromPlayer(
            uint64_t session_id, uint8_t turn_direction,
            uint32_t next_expected_event_no, std::string &player_name
    );

    explicit UpdateFromPlayer(data_t &data);

    data_t serialize() override;

    [[nodiscard]] u_int64_t get_session_id() const;

    [[nodiscard]] u_int8_t get_turn_direction() const;

    [[nodiscard]] uint32_t get_next_expected_event_no() const;

    [[nodiscard]] std::string get_player_name() const;
private:
    using player_name_t = std::vector<uint8_t>;

    uint64_t session_id;
    uint8_t turn_direction;
    uint32_t next_expected_event_no;
    player_name_t player_name;
};


#endif //PROJ2_UPDATE_FROM_PLAYER_H
