#include "server.h"

int main(int argc, char *argv[]) {
    Server server{argc, argv};
    server.start();
    return 0;
}
