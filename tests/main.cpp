#include <netinet/in.h>
#include <cassert>
#include <cstring>
#include "../common/new_game.h"
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
    event_type = ntohl(event_type);

    assert(len == game.get_len());
    assert(event_no == game.get_event_no());
    assert(event_type == game.get_event_type());

    NewGame game_deserialized{serialized_shortened, len, event_no};
    assert(game_deserialized.get_maxx() == 640);
    assert(game_deserialized.get_maxy() == 480);
    assert(game_deserialized.get_players() == players);
    assert(game_deserialized.get_crc32() == game.get_crc32());

    return 0;
}