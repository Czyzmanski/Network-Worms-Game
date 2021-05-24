#include <netinet/in.h>
#include <cassert>
#include <cstring>
#include "../common/new_game.h"
#include "../common/pixel.h"
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
    assert(game_deserialized.text_repr() == "NEW_GAME 640 480 Szymon Michał Adam Wiktoria");

    //////////////////////////////////////////////////////////////////////

    Pixel pixel{234234, 69, player_name, 323, 432};
    serialized = pixel.serialize();
    assert(serialized.size() == pixel.get_serialized_size());

    data_t pixel_serialized_shortened;
    for (size_t i = 10; i < serialized.size(); i++) {
        pixel_serialized_shortened.push_back(serialized[i]);
    }

    buf = serialized.data();

    memcpy(&len, buf, 4);
    len = ntohl(len);

    memcpy(&event_no, buf + 4, 4);
    event_no = ntohl(event_no);

    memcpy(&event_type, buf + 8, 1);

    uint8_t player_number;
    memcpy(&player_number, buf + 9, 1);

    assert(len == pixel.get_len());
    assert(event_no == pixel.get_event_no());
    assert(event_type == pixel.get_event_type());
    assert(player_number == pixel.get_player_number());
    assert(player_name == pixel.get_player_name());

    Pixel pixel_deserialized{pixel_serialized_shortened, len, event_no, player_number, player_name};
    assert(pixel_deserialized.get_x() == 323);
    assert(pixel_deserialized.get_y() == 432);
    assert(pixel_deserialized.get_len() == len);
    assert(pixel_deserialized.get_event_no() == event_no);
    assert(pixel_deserialized.get_event_type() == event_type);
    assert(pixel_deserialized.get_player_number() == player_number);
    assert(pixel_deserialized.get_player_name() == player_name);
    assert(pixel_deserialized.get_crc32() == pixel.get_crc32());
    assert(pixel_deserialized.text_repr() == "PIXEL 323 432 Szymon");

    return 0;
}