#include <iostream>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <errno.h>

int main() {
    // Attach to the system under the name "sensor"
    name_attach_t* attach = name_attach(NULL, "sensor", 0);
    if (attach == NULL) {
        perror("name_attach");
        return EXIT_FAILURE;
    }

    std::cout << "Sensor started. Waiting for messages..." << std::endl;

    // Union can store either a _pulse or your message data
    union {
        struct _pulse pulse;
        int msg;
    } recv_buf;

    int rcvid;

    while (true) {
        rcvid = MsgReceive(attach->chid, &recv_buf, sizeof(recv_buf), NULL);
        if (rcvid == -1) {
            perror("MsgReceive");
            continue;
        }

        if (rcvid == 0) {
            // 🔹 This is a system pulse, not an app message
            // Do nothing (ignore it)
            continue;
        }

        // 🔹 Normal message from app
        std::cout << "Received message: " << recv_buf.msg << std::endl;

        // Optional: simulate sensor data (reply with something)
        int reply = recv_buf.msg + 10;
        MsgReply(rcvid, EOK, &reply, sizeof(reply));
    }

    name_detach(attach, 0);
    return 0;
}
