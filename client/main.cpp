#include "client.h"

int main(int argc, char *argv[]) {
    Client client{argc, argv};
    client.start();
    return 0;
}
