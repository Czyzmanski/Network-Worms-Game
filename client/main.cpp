#include "client.h"

int main(int argc, const char *argv[]) {
    Client client{argc, argv};
    client.start();
    return 0;
}
