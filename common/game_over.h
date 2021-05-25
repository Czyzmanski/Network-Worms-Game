#ifndef PROJ2_GAME_OVER_H
#define PROJ2_GAME_OVER_H

#include "serv_event.h"

class GameOver : public ServEvent {
   public:
    explicit GameOver(uint32_t event_no);

    GameOver(data_t &data, uint32_t len, uint32_t event_no);

    ~GameOver() override = default;

    data_t serialize() override;

    std::string text_repr() override;
};

#endif  // PROJ2_GAME_OVER_H
