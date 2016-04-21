#ifndef _PROPOSER_H_
#define _PROPOSER_H_
#include <event2/event.h>
#include <event2/bufferevent.h>
#include "config.h"
#include "message.h"

typedef struct CoordinatorCtx {
    struct event_base *base;
    struct sockaddr_in *acceptor_addr;
    Config conf;
    int cur_inst;
    int listen_port;
    evutil_socket_t sock;
} CoordinatorCtx;

int start_coordinator(Config *conf);

#endif
