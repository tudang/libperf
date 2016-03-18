#ifndef _PROPOSER_H_
#define _PROPOSER_H_
#include <event2/event.h>
#include "config.h"

typedef struct ProposerCtx {
    struct event_base *base;
    struct sockaddr_in *serveraddr;
    int verbose;
    int mps;
    int cur_inst;
    int max_inst;
    int acked_packets;
    int enable_paxos;
    int outstanding;
    int64_t avg_lat;
    int *values;
    char *buffer;
} ProposerCtx;

int start_proposer();
ProposerCtx *proposer_ctx_new(Config *conf);
void proposer_ctx_destroy(ProposerCtx *st);
void send_value(evutil_socket_t fd, short what, void *arg);
#endif
