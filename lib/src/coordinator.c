#include <stdio.h>
#include <event2/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <string.h>
#include <strings.h>
#include <inttypes.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "message.h"
#include "netpaxos_utils.h"
#include "config.h"
#include "coordinator.h"

#define BUF_SIZE 1500

void propose_value(CoordinatorCtx *ctx, void *arg, int size, struct sockaddr_in client);

CoordinatorCtx *proposer_ctx_new(Config conf) {
    CoordinatorCtx *ctx = malloc(sizeof(CoordinatorCtx));
    ctx->conf = conf;
    ctx->cur_inst = 0;
    // TODO: Pass port number as in configuration
    ctx->listen_port = 12345;
    ctx->buf = malloc(BUF_SIZE);
    ctx->msg = malloc(sizeof(Message)); 
    bzero(ctx->msg, sizeof(Message));
    return ctx;
}


void on_value(evutil_socket_t fd, short what, void *arg)
{
    CoordinatorCtx *ctx = (CoordinatorCtx *) arg;
    if (what&EV_READ) {
        struct sockaddr_in remote;
        socklen_t remote_len = sizeof(remote);
        char recvbuf[BUF_SIZE]; 
        int n = recvfrom(fd, recvbuf, BUF_SIZE, 0, (struct sockaddr *) &remote, &remote_len);
        if (n < 0) {
          perror("ERROR in recvfrom");
          return;
        }
        printf("on value: %s: %d length, addr_length: %d\n", recvbuf, n, remote_len);
        propose_value(ctx, recvbuf, n, remote);
    }
}


void propose_value(CoordinatorCtx *ctx, void *arg, int size, struct sockaddr_in client)
{
    char *v = (char*) arg;
    socklen_t serverlen = sizeof(*ctx->learner_addr);
    Message msg;
    initialize_message(&msg, ctx->conf.paxos_msgtype);
    msg.client = client;
    if (ctx->cur_inst >= ctx->conf.maxinst) {
        return;
    }
    msg.inst = ctx->cur_inst;
    strncpy(msg.paxosval, v, size);

    pack(ctx->msg, &msg);
    if (sendto(ctx->sock, ctx->msg, sizeof(Message), 0, 
            (struct sockaddr*) ctx->learner_addr, serverlen) < 0) {
        perror("ERROR in sendto");
        return;
    }
    ctx->cur_inst++;
}

int start_coordinator(Config *conf, int port) {
    CoordinatorCtx *ctx = proposer_ctx_new(*conf);
    ctx->base = event_base_new();
    struct hostent *server;
    int serverlen;
    ctx->learner_addr = malloc( sizeof (struct sockaddr_in) );

    // socket to send Paxos messages to learners
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("cannot create socket");
        return EXIT_FAILURE;
    }
    ctx->sock = sock;

    server = gethostbyname(conf->learner_addr);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host as %s\n", conf->learner_addr);
        return EXIT_FAILURE;
    }

    /* build the server's Internet address */
    bzero((char *) ctx->learner_addr, sizeof(struct sockaddr_in));
    ctx->learner_addr->sin_family = AF_INET;
    bcopy((char *)server->h_addr,
      (char *)&(ctx->learner_addr->sin_addr.s_addr), server->h_length);
    ctx->learner_addr->sin_port = htons(conf->learner_port);

    struct event *ev_recv;
    int listen_socket = create_server_socket(port);
    addMembership(conf->proposer_addr, listen_socket);
    ev_recv = event_new(ctx->base, listen_socket, EV_READ|EV_PERSIST, on_value, ctx);

    event_add(ev_recv, NULL);

    event_base_dispatch(ctx->base);
    // close(client_sock);
    close(sock);
    return EXIT_SUCCESS;
}

