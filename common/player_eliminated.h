#ifndef PROJ2_PLAYER_ELIMINATED_H
#define PROJ2_PLAYER_ELIMINATED_H

#include "serv_event.h"

class PlayerEliminated : public ServEvent {
   public:
    PlayerEliminated(uint32_t event_no, uint8_t player_number,
                     std::string& player_name);

    PlayerEliminated(data_t &data, uint32_t len, uint32_t event_no,
                     uint8_t player_number, std::string& player_name);

    ~PlayerEliminated() override = default;

    data_t serialize() override;

    std::string text_repr() override;

    [[nodiscard]] uint8_t get_player_number() const { return player_number; }

    [[nodiscard]] std::string get_player_name() const { return player_name; }

   private:
    uint8_t player_number;
    std::string player_name;
};

#endif  // PROJ2_PLAYER_ELIMINATED_H
