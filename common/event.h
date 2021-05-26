#ifndef PROJ2_EVENT_H
#define PROJ2_EVENT_H

#include <cstdint>
#include <memory>
#include <vector>

using data_t = std::vector<uint8_t>;

class Event {
   public:
    Event() = default;

    virtual ~Event() = default;

    virtual data_t serialize() = 0;

    [[nodiscard]] uint32_t get_serialized_size() const { return serialized_size; }

   protected:
    data_t serialized;
    uint32_t serialized_size{};
};

#endif  // PROJ2_EVENT_H
