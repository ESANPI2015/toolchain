#ifndef _<name>_COMM_H
#define _<name>_COMM_H

#include "<name>_graph.h"
#include "ndlcom/Node.h"

#define <name>_DEVICE_ID <myId>

typedef <type> (*<name>_getInternalInputFunc)(const unsigned int);
typedef void (*<name>_setInternalOutputFunc)(const unsigned int, const <type>);

typedef struct {
    /*Behaviour Graph*/
    <type> in[<name>_GRAPH_NO_INPUTS];
    <type> out[<name>_GRAPH_NO_OUTPUTS];
    <name>_graph_t graph;
    /*NDLCOM*/
    uint8_t numUpdates[<name>_GRAPH_NO_INPUTS];
    struct NDLComInternalHandler handler;
    struct NDLComNode *node;
    /*Internal I/O functions*/
    <name>_getInternalInputFunc sample;
    <name>_setInternalOutputFunc commit;
} <name>_comm_t;

/*This function registers handlers to the passed node to enable BG processing*/
uint8_t <name>_init(<name>_comm_t *comm, struct NDLComNode *node, const <name>_getInternalInputFunc sample, const <name>_setInternalOutputFunc commit);

/*This function has to be called whenever the behaviour graph is allowed to be evaluated*/
uint8_t <name>_process(<name>_comm_t *comm);

#endif
