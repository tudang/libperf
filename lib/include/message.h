#ifndef _MESSAGE_H_
#define _MESSAGE_H_
#include <stdint.h>
#include <time.h>

#define PAXOS_VALUE_SIZE 32

#define VLEN 10
#define BUFSIZE 200
#define TIMEOUT 1

enum paxos_type {
    phase0,
    phase1a,
    phase1b,
    phase2a,
    phase2b
};

typedef struct Message {
    uint32_t inst;
    uint16_t rnd;
    uint16_t vrnd;
    uint16_t acptid;
    uint16_t msgtype;
    // Fixed value size: 32 Bytes
    char paxosval[PAXOS_VALUE_SIZE];
    struct sockaddr_in client;
} Message;


void print_message(Message *m);
void pack(Message *src);
void unpack(Message *src);
void initialize_message(Message *m, int msgtype);
#endif