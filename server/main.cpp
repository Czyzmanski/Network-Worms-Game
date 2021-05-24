#include "server.h"

int main(int argc, const char *argv[]) {
    Server server{argc, argv};
    server.start();
    return 0;
}
