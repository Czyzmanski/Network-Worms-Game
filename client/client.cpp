#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/time.h>

#include "../common/crc32.h"
#include "../common/new_game.h"
#include "../common/options.h"
#include "../common/pixel.h"
#include "../common/player_eliminated.h"
#include "../common/update_from_player.h"
#include "client.h"

Client::Client(int argc, char *const argv[]) {
    if (argc == 1) {
        std::cerr << "Usage: ./screen-worms-client game_server [-n player_name] [-p "
                     "n] [-i gui_server] [-r n]"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    options_t options = parse_options(argc - 1, argv + 1);

    struct timeval tv {};
    gettimeofday(&tv, nullptr);
    session_id = tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
    turn_direction = 0;
    next_expected_event_no = 0;
    player_name = options.at("player_name");
    new_game_exp = true;
    curr_game_id = 0;

    timer_fd.events = POLLIN;
    gui_serv_fd.events = POLLIN;
    game_serv_fd.events = POLLIN;

    connect_with_gui_server(options.at("gui_server"), options.at("gui_server_port"));
    init_game_server_sockfd(argv[1], options.at("game_server_port"));

    turn_direction_by_key_event.insert({"LEFT_KEY_DOWN", 2});
    turn_direction_by_key_event.insert({"LEFT_KEY_UP", 0});
    turn_direction_by_key_event.insert({"RIGHT_KEY_DOWN", 1});
    turn_direction_by_key_event.insert({"RIGHT_KEY_UP", 0});
}

void Client::start() {
    init_timer();
    struct pollfd fds[FDS_COUNT] = {timer_fd, gui_serv_fd, game_serv_fd};
    while (true) {
        int poll_count = poll(fds, FDS_COUNT, -1);
        if (poll_count == -1) {
            perror("poll failed");
            exit(EXIT_FAILURE);
        }

        for (auto &fd : fds) {
            if (fd.revents & POLLIN) {
                if (fd.fd == timer_fd.fd) {
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
    int rv = sendto(game_serv_fd.fd, serialized.data(), serialized.size(), 0, &game_serv_addr,
                    game_serv_addr_len);
    if (rv != 0) {
        perror("sending to game server failed");
        exit(EXIT_FAILURE);
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
            perror("sending data to gui server failed");
            exit(EXIT_FAILURE);
        }
        pos += sent;
        buf_len -= sent;
    } while (sent > 0);
}

void Client::pass_data_between_game_server_and_gui_server() {
    size_t buf_len = 550;
    uint8_t buf[buf_len];
    ssize_t read =
        recvfrom(game_serv_fd.fd, buf, buf_len, 0, &game_serv_addr, &game_serv_addr_len);
    if (read < 0) {
        perror("error while reading from game server");
        exit(EXIT_FAILURE);
    }

    if (read <= sizeof(uint32_t)) {
        return;
    }

    size_t pos = 0;
    uint32_t game_id;
    memcpy(&game_id, buf, sizeof(game_id));
    game_id = ntohl(game_id);
    pos += sizeof(game_id);

    if (events.empty()) {
        new_game_exp = true;
        curr_game_id = game_id;
        next_expected_event_no = 0;
        events.insert(game_id);
    } else if (game_id != curr_game_id) {
        if (events.find(game_id) == events.end()) {
            new_game_exp = true;
            curr_game_id = game_id;
            next_expected_event_no = 0;
            events.insert(game_id);
        } else {
            // Datagram from a previous game.
            return;
        }
    }

    while (pos < read) {
        uint32_t record_start = pos;
        uint32_t len;
        if (pos + sizeof(len) >= read) {
            break;
        }
        memcpy(&len, buf + pos, sizeof(len));
        len = ntohl(len);
        pos += sizeof(len);
        if (pos + len + sizeof(uint32_t) > read) {
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
                }
            } else {
                // Unrecognized event type.
                break;
            }
        }
        pos += len + sizeof(uint32_t);
    }
}

void Client::read_data_from_gui_server() {
    data_t data;
    ssize_t read;
    do {
        size_t buf_len = 1024;
        uint8_t buf[buf_len];
        read = recv(gui_serv_fd.fd, buf, buf_len, MSG_DONTWAIT);
        if (read < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("error while reading from gui server");
                exit(EXIT_FAILURE);
            }
        } else {
            for (size_t i = 0; i < read; i++) {
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
                turn_direction = turn_direction_by_key_event[key_event];
            }

            key_event.clear();
        } else {
            key_event += (char)b;
        }
    }
}

void Client::init_timer() {
    if ((timer_fd.fd = timerfd_create(CLOCK_MONOTONIC, 0)) == -1) {
        std::cerr << "timerfd_create failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    struct timespec it_value = {.tv_sec = 0, .tv_nsec = 30000};
    struct itimerspec new_value = {.it_interval = it_value, .it_value = it_value};
    if (timerfd_settime(timer_fd.fd, TFD_TIMER_ABSTIME, &new_value, nullptr) == -1) {
        std::cerr << "timerfd_settime failed" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Client::init_game_server_sockfd(const std::string &server, const std::string &port) {
    struct addrinfo hints {};
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;

    struct addrinfo *server_info;
    if (getaddrinfo(server.c_str(), port.c_str(), &hints, &server_info) != 0) {
        perror("gui server getaddrinfo failed");
        exit(EXIT_FAILURE);
    }

    struct addrinfo *node = server_info;
    while (node != nullptr) {
        if ((game_serv_fd.fd =
                 socket(node->ai_family, node->ai_socktype, node->ai_protocol)) != -1) {
            break;
        }
        perror("game server socket creation failed");
        node = node->ai_next;
    }

    if (node == nullptr) {
        fprintf(stderr, "failed to create any game server socket\n");
        exit(EXIT_FAILURE);
    }

    game_serv_addr = *node->ai_addr;
    game_serv_addr_len = node->ai_addrlen;

    freeaddrinfo(server_info);
}

void Client::connect_with_gui_server(const std::string &server, const std::string &port) {
    struct addrinfo hints {};
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *server_info;
    if (getaddrinfo(server.c_str(), port.c_str(), &hints, &server_info) != 0) {
        perror("gui server getaddrinfo failed");
        exit(EXIT_FAILURE);
    }

    struct addrinfo *node = server_info;
    while (node != nullptr) {
        gui_serv_fd.fd =
            socket(node->ai_family, node->ai_socktype | SOCK_NONBLOCK, node->ai_protocol);
        if (gui_serv_fd.fd != -1) {
            if (connect(gui_serv_fd.fd, node->ai_addr, node->ai_addrlen) != -1) {
                break;
            }
            perror("connecting with gui server failed");
        } else {
            perror("gui server socket creation failed");
        }
        node = node->ai_next;
    }

    if (node == nullptr) {
        std::cerr << "failed to connect with gui server" << std::endl;
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if (setsockopt(gui_serv_fd.fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) != 0) {
        perror("setting TCP_NODELAY failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "connecting with gui server..." << std::endl;

    freeaddrinfo(server_info);
}
