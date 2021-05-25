#ifndef PROJ2_PIXEL_H
#define PROJ2_PIXEL_H

#include "serv_event.h"

class Pixel : public ServEvent {
   public:
    Pixel(uint32_t event_no, uint8_t player_number, uint32_t x, uint32_t y);

    Pixel(data_t& data, uint32_t len, uint32_t event_no,
          player_by_number_t &player_by_number);

    ~Pixel() override = default;

    data_t serialize() override;

    std::string text_repr() override;

    [[nodiscard]] uint8_t get_player_number() const { return player_number; }

    [[nodiscard]] std::string get_player_name() const { return player_name; }

    [[nodiscard]] uint32_t get_x() const { return x; }

    [[nodiscard]] uint32_t get_y() const { return y; }

   private:
    uint8_t player_number;
    std::string player_name;
    uint32_t x;
    uint32_t y;
};

#endif  // PROJ2_PIXEL_H
