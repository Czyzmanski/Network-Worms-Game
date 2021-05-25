#include <netinet/in.h>
#include <cassert>
#include <cstring>
#include "../common/game_over.h"
#include "../common/new_game.h"
#include "../common/options.h"
#include "../common/pixel.h"
#include "../common/player_eliminated.h"
#include "../common/update_from_player.h"

int main() {
    std::string player_name = "Szymon";
    UpdateFromPlayer update{123456, 1, 69, player_name};
    auto serialized = update.serialize();
    UpdateFromPlayer update_deserialized{serialized};
    assert(update_deserialized.get_session_id() == 123456);
    assert(update_deserialized.get_turn_direction() == 1);
    assert(update_deserialized.get_next_expected_event_no() == 69);
    assert(update_deserialized.get_player_name() == "Szymon");

    //////////////////////////////////////////////////////////////////////

    players_t players{"Szymon", "Michał", "Adam", "Wiktoria"};
    NewGame game{234234, 640, 480, players};
    serialized = game.serialize();
    assert(serialized.size() == game.get_serialized_size());

    data_t serialized_shortened;
    for (size_t i = 9; i < serialized.size(); i++) {
        serialized_shortened.push_back(serialized[i]);
    }

    auto buf = serialized.data();

    uint32_t len;
    memcpy(&len, buf, 4);
    len = ntohl(len);

    uint32_t event_no;
    memcpy(&event_no, buf + 4, 4);
    event_no = ntohl(event_no);

    uint8_t event_type;
    memcpy(&event_type, buf + 8, 1);

    assert(len == game.get_len());
    assert(event_no == game.get_event_no());
    assert(event_type == game.get_event_type());

    NewGame game_deserialized{serialized_shortened, len, event_no};
    assert(game_deserialized.get_maxx() == 640);
    assert(game_deserialized.get_maxy() == 480);
    assert(game_deserialized.get_len() == len);
    assert(game_deserialized.get_event_no() == event_no);
    assert(game_deserialized.get_event_type() == event_type);
    assert(game_deserialized.get_players() == players);
    assert(game_deserialized.get_crc32() == game.get_crc32());
    assert(game_deserialized.text_repr() ==
           "NEW_GAME 640 480 Szymon Michał Adam Wiktoria");

    //////////////////////////////////////////////////////////////////////
    player_by_number_t player_by_number;
    player_by_number.insert({69, "Szymon"});

    Pixel pixel{234234, 69, 323, 432};
    serialized = pixel.serialize();
    assert(serialized.size() == pixel.get_serialized_size());

    data_t pixel_serialized_shortened;
    for (size_t i = 9; i < serialized.size(); i++) {
        pixel_serialized_shortened.push_back(serialized[i]);
    }

    buf = serialized.data();

    memcpy(&len, buf, 4);
    len = ntohl(len);

    memcpy(&event_no, buf + 4, 4);
    event_no = ntohl(event_no);

    memcpy(&event_type, buf + 8, 1);

    assert(len == pixel.get_len());
    assert(event_no == pixel.get_event_no());
    assert(event_type == pixel.get_event_type());

    Pixel pixel_deserialized{pixel_serialized_shortened, len, event_no,
                             player_by_number};
    assert(pixel_deserialized.get_x() == 323);
    assert(pixel_deserialized.get_y() == 432);
    assert(pixel_deserialized.get_len() == len);
    assert(pixel_deserialized.get_event_no() == event_no);
    assert(pixel_deserialized.get_event_type() == event_type);
    assert(pixel_deserialized.get_player_number() == 69);
    assert(pixel_deserialized.get_player_name() == player_by_number[69]);
    assert(pixel_deserialized.get_crc32() == pixel.get_crc32());
    assert(pixel_deserialized.text_repr() == "PIXEL 323 432 Szymon");

    ////////////////////////////////////////////////////////////////////

    PlayerEliminated player_eliminated{234234, 69};
    serialized = player_eliminated.serialize();
    assert(serialized.size() == player_eliminated.get_serialized_size());

    data_t player_eliminated_serialized_shortened;
    for (size_t i = 9; i < serialized.size(); i++) {
        player_eliminated_serialized_shortened.push_back(serialized[i]);
    }

    buf = serialized.data();

    memcpy(&len, buf, 4);
    len = ntohl(len);

    memcpy(&event_no, buf + 4, 4);
    event_no = ntohl(event_no);

    memcpy(&event_type, buf + 8, 1);

    assert(len == player_eliminated.get_len());
    assert(event_no == player_eliminated.get_event_no());
    assert(event_type == player_eliminated.get_event_type());

    PlayerEliminated player_eliminated_deserialized{
        player_eliminated_serialized_shortened, len, event_no,
        player_by_number};
    assert(player_eliminated_deserialized.get_len() == len);
    assert(player_eliminated_deserialized.get_event_no() == event_no);
    assert(player_eliminated_deserialized.get_event_type() == event_type);
    assert(player_eliminated_deserialized.get_player_number() == 69);
    assert(player_eliminated_deserialized.get_player_name() ==
           player_by_number[69]);
    assert(player_eliminated_deserialized.get_crc32() ==
           player_eliminated.get_crc32());
    assert(player_eliminated_deserialized.text_repr() ==
           "PLAYER_ELIMINATED Szymon");

    ////////////////////////////////////////////////////////////////////

    GameOver game_over{234234};
    serialized = game_over.serialize();
    assert(serialized.size() == game_over.get_serialized_size());

    data_t game_over_serialized_shortened;
    for (size_t i = 9; i < serialized.size(); i++) {
        game_over_serialized_shortened.push_back(serialized[i]);
    }

    buf = serialized.data();

    memcpy(&len, buf, 4);
    len = ntohl(len);

    memcpy(&event_no, buf + 4, 4);
    event_no = ntohl(event_no);

    memcpy(&event_type, buf + 8, 1);

    assert(len == game_over.get_len());
    assert(event_no == game_over.get_event_no());
    assert(event_type == game_over.get_event_type());

    GameOver game_over_deserialized{game_over_serialized_shortened, len,
                                    event_no};
    assert(game_over_deserialized.get_len() == len);
    assert(game_over_deserialized.get_event_no() == event_no);
    assert(game_over_deserialized.get_event_type() == event_type);
    assert(game_over_deserialized.get_crc32() == game_over.get_crc32());
    assert(game_over_deserialized.text_repr() == "GAME_OVER");

    ////////////////////////////////////////////////////////////////////////

    // ./screen-worms-client game_server [-n player_name] [-p n] [-i gui_server] [-r n]

    int argc = 5;
    std::string binary_file = "screen-worms-client";
    std::string n = "-n";
    player_name = "Szymon";
    std::string p = "-p";
    std::string port = "342234";
    char *const argv[] = {&binary_file[0], &n[0], &player_name[0], &p[0], &port[0]};
    options_t options = parse_options(argc, argv);
    assert(options.at("player_name") == "Szymon");
    assert(options.at("game_server_port") == "342234");
    assert(options.at("gui_server") == "localhost");
    assert(options.at("gui_server_port") == "20210");

    return 0;
}