#ifndef PROJ2_CLIENT_H
#define PROJ2_CLIENT_H

#include <string>

class Client {
   public:
    Client(int argc, const char *argv[]);
    void start();

   private:
    uint64_t session_id;
    uint8_t turn_direction;
    uint32_t next_expected_event_no;
    std::string player_name;
    static constexpr nfds_t FDS_COUNT = 3;
    struct pollfd timer_fd {};
    struct pollfd gui_serv_fd {};
    struct pollfd game_serv_fd {};
    struct sockaddr game_serv_addr {};
    socklen_t game_serv_addr_len;

    void init_timer();
    void connect_with_gui_server(const std::string &server,
                                 const std::string &port);
    void init_game_server_sockfd(const std::string &server,
                                 const std::string &port);
    void send_data_to_game_server();
    void read_data_from_game_server();
    void send_data_to_gui_server();
    void read_data_from_gui_server();
};

#endif  // PROJ2_CLIENT_H
