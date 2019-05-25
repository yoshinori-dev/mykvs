//
// Created by yoshinori on 16/01/02.
//

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/syslog.h>
#include <unistd.h>

#include "myclib/myclib.h"
#include "utils.h"

int SendShutdownCmd(const char *path) {
    struct sockaddr_un addr;
    my_memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    my_memcpy(addr.sun_path, path, my_strlen(path));
    int ctrlSock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ctrlSock < 0) {
        syslog(LOG_ERR, "Failed to create the control socket.. can not shut down...");
        return -1;
    }
    if (connect(ctrlSock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0) {
        syslog(LOG_ERR, "Can not connect to the control socket.. can not shut down...");
        return -1;
    }
    if (send(ctrlSock, "SHUTDOWN", 8, 0) < 8) {
        // Shall I consider the case where it sends only 7 bytes or less?
        syslog(LOG_ERR, "can not send SHUTDOWN command..");
        return -1;
    }
    close(ctrlSock);
}