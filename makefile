.PHONY: all clean

all: screen-worms-client

main.o: client/main.cpp client/client.h
	g++ -Wall -Wextra -O2 -std=c++17 client/main.cpp -c

client.o: client/client.cpp client/client.h common/serv_event.h common/crc32.h common/event.h common/new_game.h common/options.h common/pixel.h common/player_eliminated.h common/update_from_player.h common/utils.h
	g++ -Wall -Wextra -O2 -std=c++17 client/client.cpp -c

crc32.o: common/crc32.cpp common/crc32.h
	g++ -Wall -Wextra -O2 -std=c++17 common/crc32.cpp -c

game_over.o: common/game_over.cpp common/game_over.h common/crc32.h
	g++ -Wall -Wextra -O2 -std=c++17 common/game_over.cpp -c

new_game.o: common/new_game.cpp common/new_game.h common/crc32.h
	g++ -Wall -Wextra -O2 -std=c++17 common/new_game.cpp -c

options.o: common/options.cpp common/options.h
	g++ -Wall -Wextra -O2 -std=c++17 common/options.cpp -c

pixel.o: common/pixel.cpp common/pixel.h common/crc32.h common/utils.h
	g++ -Wall -Wextra -O2 -std=c++17 common/pixel.cpp -c

player_eliminated.o: common/player_eliminated.cpp common/player_eliminated.h common/crc32.h common/utils.h
	g++ -Wall -Wextra -O2 -std=c++17 common/player_eliminated.cpp -c

update_from_player.o: common/update_from_player.cpp common/update_from_player.h
	g++ -Wall -Wextra -O2 -std=c++17 common/update_from_player.cpp -c

utils.o: common/utils.cpp common/utils.h
	g++ -Wall -Wextra -O2 -std=c++17 common/utils.cpp -c

screen-worms-client: client.o crc32.o game_over.o new_game.o options.o pixel.o player_eliminated.o update_from_player.o utils.o main.o
	g++ -Wall -Wextra -O2 -std=c++17 client.o crc32.o game_over.o new_game.o options.o pixel.o player_eliminated.o update_from_player.o utils.o main.o -o screen-worms-client

clean:
	rm -f client.o crc32.o game_over.o new_game.o options.o pixel.o player_eliminated.o update_from_player.o main.o screen-worms-client
