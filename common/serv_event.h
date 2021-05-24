#ifndef PROJ2_SERV_EVENT_H
#define PROJ2_SERV_EVENT_H

#include "event.h"

constexpr uint8_t NEW_GAME = 0;
constexpr uint8_t PIXEL = 1;
constexpr uint8_t PLAYER_ELIMINATED = 2;
constexpr uint8_t GAME_OVER = 3;

class ServEvent : public Event {
   public:
    ServEvent() = default;

    ~ServEvent() override = default;

    virtual std::string text_repr() = 0;

    [[nodiscard]] uint32_t get_len() const { return len; }

    [[nodiscard]] uint32_t get_event_no() const { return event_no; }

    [[nodiscard]] uint32_t get_event_type() const { return event_type; }

    [[nodiscard]] uint32_t get_crc32() const { return crc32; }

   protected:
    uint32_t len;
    uint32_t event_no;
    uint8_t event_type;
    uint32_t crc32;
};

using serv_event_ptr_t = std::shared_ptr<ServEvent>;

#endif  // PROJ2_SERV_EVENT_H
