/*
 * Toplvl template:
 * * Based on unreliable, lossy NDLCom
 * * Handles internal inputs/outputs
 * * Uses UDP to connect with a NDLCom Bridge (others could be possible too)
 * * Assumptions: 
 *   * A packet loss is improbable
 *   * Being in phase is more important than having correct values
 *   * Deadlocks have to be avoided
 *
 * NOTE: This template is not finished yet because it is target-dependent
 * Long-term vision:
 * * Provide the ndlcomBGDataHandler or ndlcomNodeRegisterBGDataHandler
 * * Provide the device id to be used by target
 * * Sample and commit stuff has to be exported
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "<name>_graph.h"
#include "ndlcom/Bridge.h"
#include "ndlcom/Node.h"
#include "representations/id.h"
#include "representations/BGData.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_NO_UPDATES 2

static <type> in[MAX_NO_UPDATES][<name>_GRAPH_NO_INPUTS] = { {0.0f} };
static <type> out[<name>_GRAPH_NO_OUTPUTS] = { 0.0f };

static const uint8_t input2senderId[<name>_GRAPH_NO_INPUTS] = {
    <input2srcId0>
};
static const uint8_t input2srcIdx[<name>_GRAPH_NO_INPUTS] = {
    <input2srcIdx0>
};
static const uint8_t output2receiverId[<name>_GRAPH_NO_OUTPUTS] = {
    <output2sinkId0>
};
static const uint8_t output2sinkIdx[<name>_GRAPH_NO_OUTPUTS] = {
    <output2sinkIdx0>
};
/*TODO: Initialize this to 1 when a backedge has been detected as incoming*/
static uint8_t numUpdates[<name>_GRAPH_NO_INPUTS] = { 0 };

/*NOTE: These are communication dependent entities*/
static struct NDLComBridge bridge;
static struct NDLComNode node;
static struct NDLComInternalHandler handler;
static struct NDLComExternalInterface interface;

/*NOTE: This is target dependent code!*/
struct myContext
{
    int fd;
    struct sockaddr_in addr_out;
};

<type> sample(const unsigned int input)
{
    /*NOTE: This is target dependent code!*/
    /*printf("<name>: sampling internal data source %u\n", input);*/
    return 0.0f;
}

void commit(const unsigned int output, const <type> value)
{
    /*NOTE: This is target dependent code!*/
    /*printf("<name>: committing %f to internal data sink %u\n", value, output);*/
}

size_t readBuf (void *context, void *buf, const size_t count)
{
    /*NOTE: This is target dependent code!*/
    ssize_t ret;
    struct myContext *ctx = (struct myContext *)context;
    ret = recvfrom(ctx->fd, buf, count, 0, NULL, NULL);
    if (ret > 0)
        return ret;
    return 0;
}

void writeBuf (void *context, const void *buf, const size_t count)
{
    /*NOTE: This is target dependent code!*/
    ssize_t ret;
    size_t written = 0;
    const char *bufPtr = buf;
    struct myContext *ctx = (struct myContext *)context;
    do {
        ret = sendto(ctx->fd, bufPtr+written, count-written, MSG_NOSIGNAL, (struct sockaddr *)&ctx->addr_out, sizeof(ctx->addr_out));
        if (ret > 0)
            written += ret;
    } while (written != count);
}

void on_incoming_packet(void *context, const struct NDLComHeader *header, const void *payload)
{
    unsigned int input;
    struct BGData *incoming = (struct BGData *)payload;

    /*Check packet*/
    if (header->mDataLen != sizeof(struct BGData))
        return;
    /*Find input to be updated and ignore internal inputs*/
    for (input = 0; input < <name>_GRAPH_NO_INPUTS; ++input) {
        if (input2senderId[input] == <myId>)
            continue;
        if (input2srcIdx[input] == incoming->input)
            break;
    }
    /*If not found, return*/
    if (input == <name>_GRAPH_NO_INPUTS)
        return;

    /*Update value: We have a queue per input in order to handle multiple updates in a timely order at least*/
    if (numUpdates[input] < MAX_NO_UPDATES) {
        in[numUpdates[input]][input] = incoming->value;
        numUpdates[input]++;
    }
}

void resampleInputs()
{
    unsigned int i;

    /*Resample all internal inputs iff they have been consumed before (otherwise we would miss samples)*/
    for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i) {
        if ((input2senderId[i] == <myId>) && (numUpdates[i] == 0)) {
            /*Resample*/
            in[0][i] = sample(i);
            numUpdates[i] = 1;
        }
    }
}

uint8_t canFire()
{
    uint8_t trigger = 1;
    unsigned int i;

    /*Check if we should fire:
     * The idea:
     * * Iff all inputs have seen an update we evaluate and produce new output
     * * Iff we have seen multiple updates, we evaluate as well (using last known values of possibly missing input updates)
     *   a) prevent dead-locks
     *   b) be always in sync even in case of packet loss
     *   c) upsample slower sources with respect to the source with maximum rate
     * * In all other cases, we have to wait */
    for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i) {
        if (numUpdates[i] > MAX_NO_UPDATES/2) {
            /*Multiple updates*/
            break;
        }
        if (numUpdates[i] < 1) {
            /*Missing update*/
            trigger = 0;
            break;
        }
    }

    return trigger;
}

void consumeInputs()
{
    unsigned int i,j;

    /*For all values:
     * * If numUpdates > 1: Remove oldest and update 'queue'. Decrease numUpdates by one
     * * If numUpdates = 1: Decrease numUpdates by one
     * * If numUpdates < 1: Do nothing*/
    for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i) {
        /*Skip missing updates*/
        if (numUpdates[i] < 1)
            continue;
        /*Update 'queue'*/
        for (j = 0; j < numUpdates[i]-1; ++j)
            in[j][i] = in[j+1][i];
        /*Decrease numUpdates*/
        numUpdates[i]--;
    }
}

void produceOutputs()
{
    struct BGData response;
    unsigned int i;

    /*Commit all new internal output values or transmit to other graphs*/
    response.mBase.mId = REPRESENTATIONS_REPRESENTATION_ID_BGData;
    for (i = 0; i < <name>_GRAPH_NO_OUTPUTS; ++i) {
        if (output2receiverId[i] == <myId>) {
            /*Update internal output*/
            commit(i, out[i]);
        } else {
            /*Update external output*/
            response.output = i;
            response.input = output2sinkIdx[i];
            response.value = out[i];
            ndlcomNodeSend(&node,output2receiverId[i], &response, sizeof(response));
        }
    }
}

int main (int argc, char *argv[])
{
    struct myContext context;
    struct sockaddr_in addr_in;
    uint16_t in_port, out_port;
    char ip_addr[16];

    /*NOTE: This is target dependent code!*/
    if (argc < 4) {
        fprintf(stderr, "Usage: %s ip-addr in-port out-port\n", argv[0]);
        return 1;
    }
    snprintf(ip_addr, 16, "%s", argv[1]);
    ip_addr[15] = '\0';
    in_port = atoi(argv[2]);
    out_port = atoi(argv[3]);
    /*Create UDP socket: We cannot block because the BridgeProcess can be overrun with data :(*/
    if ((context.fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "Could not create socket\n");
        return 1;
    }
    memset(&addr_in, 0, sizeof(addr_in));
    memset(&context.addr_out, 0, sizeof(context.addr_out));
    addr_in.sin_family = AF_INET;
    context.addr_out.sin_family = AF_INET;
    addr_in.sin_port = htons(in_port);
    context.addr_out.sin_port = htons(out_port);
    addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
    if (inet_aton(ip_addr, &context.addr_out.sin_addr) == 0) {
        fprintf(stderr, "%s is an invalid IP address\n", ip_addr);
        return 1;
    }
    if (bind(context.fd, (struct sockaddr *)&addr_in, sizeof(addr_in)) < 0) {
        fprintf(stderr, "Could not bind to socket\n");
        return 1;
    }

    /*NOTE: This is communication dependent code*/
    /*INIT*/
    ndlcomBridgeInit(&bridge);
    ndlcomNodeInit(&node, &bridge, <myId>);
    ndlcomInternalHandlerInit(&handler, on_incoming_packet, 0, &node);
    ndlcomNodeRegisterInternalHandler(&node, &handler);
    ndlcomExternalInterfaceInit(&interface, writeBuf, readBuf, 0, &context);
    ndlcomBridgeRegisterExternalInterface(&bridge, &interface);

    /*LOOP*/
    printf("<name>: execution started\n");

    /*First sampling of all internal inputs*/
    resampleInputs();

    while (1) {
        /*Fire at will :D*/
        if (!canFire()) {
            /*Look if there is new data*/
            ndlcomBridgeProcessOnce(&bridge);
            continue;
        }

        /*Evaluate*/
        <name>_graph_evaluate(in[0],out);

        /*Consume the used inputs*/
        consumeInputs();

        /*Update all internal and external outputs*/
        produceOutputs();

        /*Resample all internal inputs*/
        resampleInputs();
    }

    /*DEINIT*/
    ndlcomNodeDeinit(&node);
    return 0;
}
