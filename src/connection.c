//
// Created by yoshinori on 15/11/28.
//

#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <limits.h>
#include <errno.h>
#include "connection.h"
#include "myclib/myclib.h"

enum {
    CLOSED = 0,
    READINGCMD,
    READINGDATA

};

#define INIT_BUF_SIZE 1024
#define CMDLINELIMIT 1023
#define RESPONSE_CMDLINELIMIT CMDLINELIMIT + 32
#define Connection_Close(c) {c->state = CLOSED; close(c->sock);}

static void Connection_ResetCmd(Connection *self, long processed) {
    // we need to handle the case where '\r\n<right command>'. currently we wait at recv forever?
    self->cmdlen -= processed;
    my_memmove(self->cmdbuf, self->cmdbuf + processed, self->cmdlen);
    self->state = READINGCMD;
}

static void Connection_SendError(Connection *self) {
    char *err = "ERROR\r\n";
    send(self->sock, err, my_strlen(err), 0);
}

void Connection_ParseAndRunCmd(Connection *self) {

    // loop until we consume all the commands we got.
    while(1) {
        if (self->state == READINGCMD) {
            char *newLinePtr = my_memchr(self->cmdbuf, '\n', self->cmdlen);
            if (newLinePtr == NULL) {
                if (self->cmdlen > CMDLINELIMIT) {
                    syslog(LOG_WARNING, "Invalid command. closing connection..");
                    Connection_Close(self);
                }
                // have not received full cmd line yet. returning.
                return;
            }
            int newLinePosition = (int) (newLinePtr - self->cmdbuf);
            if (newLinePosition <= 4 || *(newLinePtr - 1) != '\r') {
                // wrong command. ignore this line.
                Connection_ResetCmd(self, newLinePosition + 1);
                Connection_SendError(self);
                return;
            }

            if (strncmp(self->cmdbuf, "get ", 4) == 0) {
                DictionaryItem *item = KVS_GetCmd(self->kvs, (self->cmdbuf + 4), newLinePosition - 5);
                if (item) {
                    self->sendiov[0].iov_len = my_snprintf(self->sendcmdbuf, RESPONSE_CMDLINELIMIT, "VALUE %s 0 %lld\r\n", item->key, item->valuelen);
                    self->sendiov[0].iov_base = self->sendcmdbuf;
                    self->sendiov[1].iov_len = item->valuelen; // TODO valuelen is long long int and iov_len is int. is this ok?
                    self->sendiov[1].iov_base = item->value;
                    self->sendiov[2].iov_len = 7;
                    self->sendiov[2].iov_base = "\r\nEND\r\n";
                    struct msghdr msg;
                    msg.msg_namelen=0;
                    msg.msg_iov=self->sendiov;
                    msg.msg_iovlen=3;
                    msg.msg_controllen=0;
                    msg.msg_flags=0;
                    int r = sendmsg(self->sock, &msg, 0);
                    KVS_ReturnItem(self->kvs, item);
                    if (r < 0){
                        syslog(LOG_ERR, "Failed to send back the data. ");
                        Connection_Close(self);
                        return;
                    }
                } else if (send(self->sock, "END\r\n", 5, 0) < 0) {
                    syslog(LOG_ERR, "Failed to send back the data. ");
                    Connection_Close(self);
                    return;
                }
                Connection_ResetCmd(self, newLinePosition + 1);
            } else if (strncmp(self->cmdbuf, "set ", 4) == 0) {
                // parse "set <key> <flags> <exptime> <bytes>"
                char *ptr = self->cmdbuf + 4;
                char *last = newLinePtr - 1;
                while (*ptr == ' ' && ptr < last) {
                    ptr++;
                }
                self->key = ptr;
                while (*ptr != ' ' && ptr < last) {
                    ptr++;
                }
                self->keylen = (unsigned int) (ptr - self->key);
                char *next;
                long flags = my_strtol(ptr, &next, 10);
                if (ptr == next || flags == LONG_MAX || flags == LONG_MIN) {
                    // wrong command. ignore this line.
                    Connection_ResetCmd(self, newLinePosition + 1);
                    Connection_SendError(self);
                    return;
                }
                ptr = next;
                self->expires = (unsigned long)my_strtol(ptr, &next, 10);
                if (ptr == next || self->expires == LONG_MAX || self->expires == LONG_MIN) {
                    // wrong command. ignore this line.
                    Connection_ResetCmd(self, newLinePosition + 1);
                    Connection_SendError(self);
                    return;
                }
                ptr = next;
                self->bytes = (unsigned long)my_strtol(ptr, &next, 10);
                if (ptr == next || self->bytes == LONG_MAX || self->bytes == LONG_MIN) {
                    // wrong command. ignore this line.
                    Connection_ResetCmd(self, newLinePosition + 1);
                    Connection_SendError(self);
                    return;
                }
                self->data = newLinePtr + 1;
                self->state = READINGDATA;

            } else if (strncmp(self->cmdbuf, "delete ", 7) == 0) {
                // parse "delete <key> [noreply]"
                char *ptr = self->cmdbuf + 7;
                char *last = newLinePtr - 1;
                while (*ptr == ' ' && ptr < last) {
                    ptr++;
                }
                self->key = ptr;
                while (*ptr != ' ' && ptr < last) {
                    ptr++;
                }
                self->keylen = (unsigned int) (ptr - self->key);
                while (*ptr == ' ' && ptr < last) {
                    ptr++;
                }
                bool removed = KVS_RemoveCmd(self->kvs, self->key, self->keylen);
                if (ptr + 7 > last || strncmp(ptr, "noreply", 7) != 0) {
                    char *rep;
                    int replen;
                    if (removed) {
                        rep = "DELETED\r\n";
                        replen = 9;
                    } else {
                        rep = "NOT_FOUND\r\n";
                        replen = 11;
                    }
                    if (send(self->sock, rep, replen, 0) < 0) {
                        syslog(LOG_ERR, "Failed to send back the data. ");
                        Connection_Close(self);
                        return;
                    }
                }
                Connection_ResetCmd(self, newLinePosition + 1);
                return;
            } else {
                // wrong command. ignore this line.
                Connection_ResetCmd(self, newLinePosition + 1);
                Connection_SendError(self);
                return;
            }
        }
        if (self->state == READINGDATA) {
            long cmdsize = self->bytes + (self->data - self->cmdbuf) + 2;
            if (self->cmdlen >= cmdsize) {
                if (self->cmdbuf[cmdsize - 1] != '\n' && self->cmdbuf[cmdsize - 2] != '\r') {
                    // wrong and difficult to recover.
                    syslog(LOG_WARNING, "Detected invalid command. closing the connection...");
                    Connection_Close(self);
                    return;
                }
                if (!KVS_SetCmd(self->kvs, self->key, self->keylen, self->data, self->bytes, self->expires)) {
                    syslog(LOG_WARNING, "Failed to store the data");
                    char *err = "SERVER_ERROR out of memory error\r\n";
                    send(self->sock, err, my_strlen(err), 0);
                    Connection_Close(self);
                    return;
                }
                if (send(self->sock, "STORED\r\n", 8, 0) < 0) {
                    syslog(LOG_ERR, "Failed to send back the data. ");
                    Connection_Close(self);
                    return;
                }
                if (!KVS_EvictIfNeeeded(self->kvs)) {
                    syslog(LOG_ERR, "Failed to evict some data.");
                }

                Connection_ResetCmd(self, cmdsize);
            } else {
                return;
            }
        }
    }
}


int Connection_Handler(void *arg) {
    Connection *self = (Connection*)arg;
    int nrd = recv(self->sock, self->cmdbuf + self->cmdlen, self->cmdbufsize - self->cmdlen, 0);
    if (nrd <= 0) {
        if (nrd == 0) {
            syslog(LOG_DEBUG, "Connection was closed gracefully by the client ");
            Connection_Close(self);
        } else {
            syslog(LOG_ERR, "Failed to receive data. %d", errno);
            Connection_Close(self);
        }
    } else {
        self->cmdlen += nrd;
        Connection_ParseAndRunCmd(self);
    }
    return HANDLER_OK;
}

Connection *Connection_New(KVS* kvs, int connsock) {
    Connection *self = my_malloc(sizeof(Connection));
    if (!self) {
        syslog(LOG_ERR, "Failed to create connection");
        return NULL;
    }
    self->sock = connsock;
    self->state = READINGCMD;
    self->kvs = kvs;
    self->cmdbuf = my_malloc(INIT_BUF_SIZE);
    self->cmdbufsize = INIT_BUF_SIZE;
    self->cmdlen = 0;
    self->sendcmdbuf = my_malloc(RESPONSE_CMDLINELIMIT);
    self->eh.func = Connection_Handler;
    self->eh.arg = self;
    return self;
}

void Connection_Delete(Connection *self) {
    if (self->state != CLOSED) {
        Connection_Close(self);
    }
    if (self->cmdbuf) {
        my_free(self->cmdbuf);
    }
    if (self->sendcmdbuf) {
        my_free(self->sendcmdbuf);
    }
    my_free(self);
}


bool Connection_IsClosed(Connection *self) {
    return self->state == CLOSED;
}