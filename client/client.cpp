#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/timerfd.h>

#include <poll.h>
#include <sys/time.h>
#include <iostream>

#include "../common/event.h"
#include "../common/options.h"
#include "../common/update_from_player.h"
#include "client.h"

Client::Client(int argc, const char *argv[]) {
    if (argc == 1) {
        // TODO: print error with correct usage format
        exit(EXIT_FAILURE);
    }
    options_t options = parse_options(argc - 2, argv + 2, ":n:p::i::r::");

    struct timeval tv {};
    gettimeofday(&tv, nullptr);
    session_id = tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
    turn_direction = 0;
    next_expected_event_no = 0;
    player_name = options.at("player_name");

    timer_fd.events = POLLIN;
    gui_serv_fd.events = POLLIN;
    game_serv_fd.events = POLLIN;

    connect_with_gui_server(options.at("gui_server"),
                            options.at("gui_server_port"));
    init_game_server_sockfd(argv[1], options.at("game_server_port"));
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
                    read_data_from_game_server();
                    send_data_to_gui_server();
                }
            }
        }
    }
}

void Client::send_data_to_game_server() {
    UpdateFromPlayer update{session_id, turn_direction, next_expected_event_no,
                            player_name};
    data_t serialized = update.serialize();
    int rv = sendto(game_serv_fd.fd, serialized.data(), serialized.size(), 0,
                    &game_serv_addr, game_serv_addr_len);
    if (rv != 0) {
        perror("sending to game server failed");
        exit(EXIT_FAILURE);
    }
}

void Client::read_data_from_game_server() {}

void Client::send_data_to_gui_server() {}

void Client::read_data_from_gui_server() {}

void Client::init_timer() {
    if ((timer_fd.fd = timerfd_create(CLOCK_MONOTONIC, 0)) == -1) {
        std::cerr << "timerfd_create failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    struct timespec it_value = {.tv_sec = 0, .tv_nsec = 30000};
    struct itimerspec new_value = {.it_interval = it_value,
                                   .it_value = it_value};
    if (timerfd_settime(timer_fd.fd, TFD_TIMER_ABSTIME, &new_value, nullptr) ==
        -1) {
        std::cerr << "timerfd_settime failed" << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Client::init_game_server_sockfd(const std::string &server,
                                     const std::string &port) {
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
        if ((game_serv_fd.fd = socket(node->ai_family, node->ai_socktype,
                                      node->ai_protocol)) != -1) {
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

void Client::connect_with_gui_server(const std::string &server,
                                     const std::string &port) {
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
        if ((gui_serv_fd.fd = socket(node->ai_family, node->ai_socktype,
                                     node->ai_protocol)) != -1) {
            if (connect(gui_serv_fd.fd, node->ai_addr, node->ai_addrlen) !=
                -1) {
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
    if (setsockopt(gui_serv_fd.fd, IPPROTO_TCP, TCP_NODELAY, &optval,
                   sizeof(optval)) != 0) {
        perror("setting TCP_NODELAY failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "connecting with gui server..." << std::endl;

    freeaddrinfo(server_info);
}
