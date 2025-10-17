#include <iostream>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <unistd.h>

int main() {
    sleep(2); // Wait for sensor to start

    int coid = name_open("sensor", 0);
    if (coid == -1) {
        perror("name_open");
        return -1;
    }

    std::cout << "Connected to sensor!\n";

    int msg = 42;
    int reply;

    for (int i = 0; i < 3; ++i) {
        if (MsgSend(coid, &msg, sizeof(msg), &reply, sizeof(reply)) == -1) {
            perror("MsgSend");
        } else {
            std::cout << "Message " << (i + 1) << " sent. Sensor replied with: " << reply << std::endl;
        }
        sleep(1);
    }

    name_close(coid);
    return 0;
}
