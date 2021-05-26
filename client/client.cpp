#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "../common/crc32.h"
#include "../common/new_game.h"
#include "../common/options.h"
#include "../common/pixel.h"
#include "../common/player_eliminated.h"
#include "../common/update_from_player.h"
#include "../common/utils.h"
#include "client.h"

Client::Client(int argc, char *const argv[]) {
    if (argc == 1) {
        std::cerr << "Usage: ./screen-worms-client game_server [-n player_name] [-p "
                     "n] [-i gui_server] [-r n]"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    options_t options = parse_options(argc - 1, argv + 1);
    if (!is_player_name_valid(options.at("player_name"))) {
        print_invalid_value_msg_and_exit("Invalid player name: " + options.at("player_name"));
        exit(EXIT_FAILURE);
    }

    struct timeval tv {};
    gettimeofday(&tv, nullptr);
    session_id = tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
    turn_direction = 0;
    next_expected_event_no = 0;
    player_name = options.at("player_name");
    left_key_pressed = false;
    right_key_pressed = false;
    curr_game_maxx = 0;
    curr_game_maxy = 0;
    new_game_exp = true;
    curr_game_id = 0;

    timer_fd.events = POLLIN;
    gui_serv_fd.events = POLLIN;
    game_serv_fd.events = POLLIN;

    connect_with_gui_server(options.at("gui_server"), options.at("gui_server_port"));
    connect_with_game_server(argv[1], options.at("game_server_port"));

    turn_direction_by_key_event.insert({"LEFT_KEY_DOWN", TURN_LEFT});
    turn_direction_by_key_event.insert({"LEFT_KEY_UP", GO_STRAIGHT});
    turn_direction_by_key_event.insert({"RIGHT_KEY_DOWN", TURN_RIGHT});
    turn_direction_by_key_event.insert({"RIGHT_KEY_UP", GO_STRAIGHT});
}

void Client::start() {
    init_timer();
    struct pollfd fds[FDS_COUNT] = {timer_fd, gui_serv_fd, game_serv_fd};
    while (true) {
        int poll_count = poll(fds, FDS_COUNT, -1);
        if (poll_count == -1) {
            print_error_msg_and_exit("Poll failed");
        }

        for (auto &fd : fds) {
            if (fd.revents & POLLIN) {
                if (fd.fd == timer_fd.fd) {
                    char dummy_buf[1024];
                    if (read(fd.fd, dummy_buf, 1024) < 0) {
                        print_error_msg_and_exit("timerfd read failed");
                    }
                    send_data_to_game_server();
                } else if (fd.fd == gui_serv_fd.fd) {
                    read_data_from_gui_server();
                } else {
                    pass_data_between_game_server_and_gui_server();
                }
            }
        }
    }
}

void Client::send_data_to_game_server() {
    UpdateFromPlayer update{session_id, turn_direction, next_expected_event_no, player_name};
    data_t serialized = update.serialize();
    if (send(game_serv_fd.fd, serialized.data(), serialized.size(), 0) < 0) {
        print_error_msg_and_exit("Sending data to game server failed");
    }
}

void Client::send_data_to_gui_server(std::string &msg) const {
    msg += "\n";
    const char *buf = msg.c_str();
    size_t buf_len = msg.length();
    ssize_t sent;
    size_t pos = 0;
    do {
        sent = send(gui_serv_fd.fd, buf + pos, buf_len, 0);
        if (sent < 0) {
            print_error_msg_and_exit("Sending data to gui server failed");
        }
        pos += sent;
        buf_len -= sent;
    } while (sent > 0);
}

void Client::pass_data_between_game_server_and_gui_server() {
    size_t buf_len = 550;
    uint8_t buf[buf_len];
    ssize_t read = recv(game_serv_fd.fd, buf, buf_len, 0);
    if (read < 0) {
        print_error_msg_and_exit("Error while reading from game server");
    }
    if ((size_t)read <= sizeof(uint32_t)) {
        return;
    }

    size_t pos = 0;
    uint32_t game_id;
    memcpy(&game_id, buf, sizeof(game_id));
    game_id = ntohl(game_id);
    pos += sizeof(game_id);
    if (games.empty()) {
        new_game_exp = true;
        curr_game_id = game_id;
        next_expected_event_no = 0;
        games.insert(game_id);
    } else if (game_id != curr_game_id) {
        if (games.find(game_id) == games.end()) {
            new_game_exp = true;
            curr_game_id = game_id;
            next_expected_event_no = 0;
            games.insert(game_id);
        } else {
            // Datagram from a previous game.
            return;
        }
    }

    while (pos < (size_t)read) {
        uint32_t record_start = pos;
        uint32_t len;
        if (pos + sizeof(len) >= (size_t)read) {
            break;
        }
        memcpy(&len, buf + pos, sizeof(len));
        len = ntohl(len);
        pos += sizeof(len);
        if (pos + len + sizeof(uint32_t) > (size_t)read) {
            break;
        }

        uint32_t event_no;
        memcpy(&event_no, buf + pos, sizeof(event_no));
        event_no = ntohl(event_no);
        pos += sizeof(event_no);

        uint8_t event_type;
        memcpy(&event_type, buf + pos, sizeof(event_type));
        pos += sizeof(event_type);

        data_t data;
        size_t remaining_len = len - sizeof(event_no) - sizeof(event_type) + sizeof(uint32_t);
        for (size_t i = pos; i < pos + remaining_len; i++) {
            data.push_back(buf[i]);
        }

        uint32_t recv_crc32;
        memcpy(&recv_crc32, buf + pos + remaining_len - sizeof(recv_crc32),
               sizeof(recv_crc32));
        recv_crc32 = htonl(recv_crc32);
        uint32_t comp_crc32 = compute_crc32(buf + record_start, len + sizeof(len));
        if (recv_crc32 != comp_crc32) {
            break;
        }

        if (event_no == next_expected_event_no) {
            if (event_type == NEW_GAME) {
                if (new_game_exp) {
                    NewGame new_game{data, len, event_no};
                    curr_game_maxx = new_game.get_maxx();
                    curr_game_maxy = new_game.get_maxy();
                    new_game_exp = false;
                    player_by_number.clear();
                    players_t players = new_game.get_players();
                    for (size_t i = 0; i < players.size(); i++) {
                        player_by_number.insert({i, players[i]});
                    }
                    std::string text_repr = new_game.text_repr();
                    send_data_to_gui_server(text_repr);
                    next_expected_event_no++;
                }
            } else if (event_type == PIXEL) {
                if (!new_game_exp) {
                    Pixel pixel{data, len, event_no, player_by_number};
                    if (curr_game_maxx <= pixel.get_x()) {
                        print_error_msg_and_exit("Invalid x: " +
                                                 std::to_string(pixel.get_x()));
                    }
                    if (curr_game_maxy <= pixel.get_y()) {
                        print_error_msg_and_exit("Invalid y: " +
                                                 std::to_string(pixel.get_y()));
                    }
                    std::string text_repr = pixel.text_repr();
                    send_data_to_gui_server(text_repr);
                    next_expected_event_no++;
                }
            } else if (event_type == PLAYER_ELIMINATED) {
                if (!new_game_exp) {
                    PlayerEliminated player_eliminated{data, len, event_no, player_by_number};
                    std::string text_repr = player_eliminated.text_repr();
                    send_data_to_gui_server(text_repr);
                    next_expected_event_no++;
                }
            } else if (event_type == GAME_OVER) {
                if (!new_game_exp) {
                    new_game_exp = true;
                    next_expected_event_no++;
                }
            } else {
                // Unrecognized event type.
                break;
            }
        }
        pos = record_start + sizeof(len) + len + sizeof(uint32_t);
    }
}

void Client::read_data_from_gui_server() {
    size_t buf_len = 2048;
    data_t data;
    data.reserve(2048);
    ssize_t read;
    do {
        uint8_t buf[buf_len];
        read = recv(gui_serv_fd.fd, buf, buf_len, MSG_DONTWAIT);
        if (read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                print_error_msg_and_exit("Error while reading data from gui server");
            }
        } else if (read == 0) {
            print_error_msg_and_exit("Gui server disconnected");
        } else {
            for (size_t i = 0; i < (size_t)read; i++) {
                data.push_back(buf[i]);
            }
        }
    } while (read > 0);

    std::string key_event;
    for (auto &b : data) {
        if (b == '\n') {
            bool valid_key_event = turn_direction_by_key_event.find(key_event) !=
                                   turn_direction_by_key_event.end();
            if (valid_key_event) {
                if (key_event == "LEFT_KEY_DOWN") {
                    left_key_pressed = true;
                } else if (key_event == "RIGHT_KEY_DOWN") {
                    right_key_pressed = true;
                } else if (key_event == "LEFT_KEY_UP") {
                    left_key_pressed = false;
                } else if (key_event == "RIGHT_KEY_UP") {
                    right_key_pressed = false;
                }
                uint8_t new_turn_direction = turn_direction_by_key_event[key_event];
                if (new_turn_direction == GO_STRAIGHT) {
                    if (left_key_pressed) {
                        turn_direction = TURN_LEFT;
                    } else if (right_key_pressed) {
                        turn_direction = TURN_RIGHT;
                    } else {
                        turn_direction = GO_STRAIGHT;
                    }
                } else {
                    turn_direction = new_turn_direction;
                }
            }
            key_event.clear();
        } else {
            key_event += (char)b;
        }
    }
}

void Client::init_timer() {
    if ((timer_fd.fd = timerfd_create(CLOCK_MONOTONIC, 0)) == -1) {
        print_error_msg_and_exit("timerfd_create failed");
    }
    struct timespec it_value = {.tv_sec = 0, .tv_nsec = 30000000};
    struct itimerspec new_value = {.it_interval = it_value, .it_value = it_value};
    if (timerfd_settime(timer_fd.fd, TFD_TIMER_ABSTIME, &new_value, nullptr) == -1) {
        print_error_msg_and_exit("timerfd_settime failed");
    }
}

void Client::connect_with_game_server(const std::string &server, const std::string &port) {
    std::cout << "Connecting with game server..." << std::endl;

    struct addrinfo hints {};
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    struct addrinfo *server_info;
    if (getaddrinfo(server.c_str(), port.c_str(), &hints, &server_info) != 0) {
        print_error_msg_and_exit("Game server getaddrinfo failed");
    }
    game_serv_fd.fd =
        socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (game_serv_fd.fd < 0) {
        print_error_msg_and_exit("Game server socket creation failed");
    }
    if (connect(game_serv_fd.fd, server_info->ai_addr, server_info->ai_addrlen) < 0) {
        print_error_msg_and_exit("Connecting with game server failed");
    }
    freeaddrinfo(server_info);

    std::cout << "Connected" << std::endl;
}

void Client::connect_with_gui_server(const std::string &server, const std::string &port) {
    std::cout << "Connecting with gui server..." << std::endl;

    struct addrinfo hints {};
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    struct addrinfo *server_info;
    if (getaddrinfo(server.c_str(), port.c_str(), &hints, &server_info) != 0) {
        print_error_msg_and_exit("Gui server getaddrinfo failed");
    }
    gui_serv_fd.fd =
        socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (gui_serv_fd.fd < 0) {
        print_error_msg_and_exit("Gui server socket creation failed");
    }
    int optval = 1;
    if (setsockopt(gui_serv_fd.fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) != 0) {
        print_error_msg_and_exit("Setting TCP_NODELAY failed");
    }
    if (connect(gui_serv_fd.fd, server_info->ai_addr, server_info->ai_addrlen) < 0) {
        print_error_msg_and_exit("Connecting with gui server failed");
    }
    freeaddrinfo(server_info);

    std::cout << "Connected" << std::endl;
}
