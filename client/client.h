#ifndef PROJ2_CLIENT_H
#define PROJ2_CLIENT_H

#include <poll.h>
#include <sys/socket.h>
#include <map>
#include <set>
#include <string>

#include "../common/serv_event.h"

constexpr uint8_t TURN_LEFT = 2;
constexpr uint8_t TURN_RIGHT = 1;
constexpr uint8_t GO_STRAIGHT = 0;

class Client {
   public:
    Client(int argc, char *const argv[]);
    void start();

   private:
    using games_t = std::set<uint32_t>;
    using turn_direction_by_key_event_t = std::map<std::string, uint8_t>;

    uint64_t session_id;
    uint8_t turn_direction;
    uint32_t next_expected_event_no;
    std::string player_name;
    bool left_key_pressed;
    bool right_key_pressed;
    uint32_t curr_game_maxx;
    uint32_t curr_game_maxy;
    bool new_game_exp;
    uint32_t curr_game_id;
    games_t games{};
    player_by_number_t player_by_number{};
    turn_direction_by_key_event_t turn_direction_by_key_event{};

    static constexpr nfds_t FDS_COUNT = 3;
    struct pollfd timer_fd{};
    struct pollfd gui_serv_fd{};
    struct pollfd game_serv_fd{};

    void init_timer();
    void connect_with_gui_server(const std::string &server, const std::string &port);
    void connect_with_game_server(const std::string &server, const std::string &port);
    void send_data_to_game_server();
    void pass_data_between_game_server_and_gui_server();
    void read_data_from_gui_server();
    void send_data_to_gui_server(std::string &msg) const;
};

#endif  // PROJ2_CLIENT_H
