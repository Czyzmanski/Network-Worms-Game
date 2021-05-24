#ifndef PROJ2_NEW_GAME_H
#define PROJ2_NEW_GAME_H

#include "serv_event.h"

using players_t = std::vector<std::string>;

class NewGame : public ServEvent {
   public:
    NewGame(uint32_t event_no, uint32_t maxx, uint32_t maxy,
            players_t &players);

    NewGame(data_t &data, uint32_t len, uint32_t event_no);

    ~NewGame() override = default;

    data_t serialize() override;

    std::string text_repr() override;

    [[nodiscard]] uint32_t get_maxx() const { return maxx; }

    [[nodiscard]] uint32_t get_maxy() const { return maxy; }

    [[nodiscard]] players_t get_players() const { return players; }

   private:
    uint32_t maxx;
    uint32_t maxy;
    players_t players;
};

#endif  // PROJ2_NEW_GAME_H
