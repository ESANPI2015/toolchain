/*
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

static <type> in[<name>_GRAPH_NO_INPUTS] = { 0.0f };
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
static uint8_t numUpdates[<name>_GRAPH_NO_INPUTS] = { 0 };

/*NOTE: These are communication dependent entities*/
static struct NDLComBridge bridge;
static struct NDLComNode node;
static struct NDLComInternalHandler handler;
static struct NDLComExternalInterface interface;

struct myContext
{
    int fd;
    struct sockaddr_in addr_out;
};

<type> sample(const unsigned int input)
{
    /*NOTE: This is target dependent code!*/
    /*IMPLEMENT ME*/
    printf("<name>: sampling internal data source %u\n", input);
    return 0.0f;
}

void commit(const unsigned int output, const <type> value)
{
    /*NOTE: This is target dependent code!*/
    /*IMPLEMENT ME*/
    printf("<name>: committing %f to internal data sink %u\n", value, output);
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
    for (input = 0; input < <name>_GRAPH_NO_INPUTS; ++input)
    {
        if (input2senderId[input] == <myId>)
            continue;
        if (input2srcIdx[input] == incoming->input)
            break;
    }
    /*If not found, return*/
    if (input == <name>_GRAPH_NO_INPUTS)
        return;
    /*Update value*/
    in[input] = incoming->value;
    numUpdates[input]++;
}

int main (int argc, char *argv[])
{
    struct BGData response;
    struct myContext context;
    struct sockaddr_in addr_in;
    uint16_t in_port, out_port;
    unsigned int i, trigger;
    if (argc < 3) {
        fprintf(stderr, "Usage: %s in-port out-port\n", argv[0]);
        return 1;
    }
    in_port = atoi(argv[1]);
    out_port = atoi(argv[2]);
    /*Create UDP socket: We cannot block because the BridgeProcess can be overrun with data :(*/
    context.fd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&addr_in, 0, sizeof(addr_in));
    memset(&context.addr_out, 0, sizeof(context.addr_out));
    addr_in.sin_family = AF_INET;
    context.addr_out.sin_family = AF_INET;
    addr_in.sin_port = htons(in_port);
    context.addr_out.sin_port = htons(out_port);
    addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_aton("127.0.0.1", &context.addr_out.sin_addr);
    bind(context.fd, (struct sockaddr *)&addr_in, sizeof(addr_in));

    /*INIT*/
    ndlcomBridgeInit(&bridge);
    ndlcomNodeInit(&node, &bridge, <myId>);
    ndlcomInternalHandlerInit(&handler, on_incoming_packet, 0, &node);
    ndlcomNodeRegisterInternalHandler(&node, &handler);
    ndlcomExternalInterfaceInit(&interface, writeBuf, readBuf, 0, &context);
    ndlcomBridgeRegisterExternalInterface(&bridge, &interface);
    response.mBase.mId = REPRESENTATIONS_REPRESENTATION_ID_BGData;

    /*LOOP*/
    printf("<name>: execution started\n");
    while (1) {
        /*Sample all internal inputs and mark them as available*/
        for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i) {
            if (input2senderId[i] == <myId>) {
                in[i] = sample(i);
                numUpdates[i] = 1;
            }
        }

        /*Check if we should fire:
         * The idea:
         * * Iff all inputs have seen an update we evaluate and produce new output
         * * If a packet has been lost such that we received a double update, we trigger to
         *   a) prevent dead-locks
         *   b) be always in sync even in case of packet loss
         * * In all other cases, we have to wait */
        trigger = 1;
        for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i) {
            if (numUpdates[i] > 1) {
                /*Double update*/
                break;
            }
            if (numUpdates[i] < 1) {
                /*Missing update*/
                trigger = 0;
                break;
            }
        }

        /*Fire at will :D*/
        if (!trigger) {
            /*Look if there is data: USE THE FAIR METHOD!*/
            ndlcomBridgeProcessFair(&bridge);
            /*If we had been blocked we want to resample the internal inputs*/
            continue;
        }

        /*Decrease number of updates if not zero*/
        for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i) {
            if (numUpdates[i] > 0) {
                numUpdates[i]--;
            }
        }

        /*Evaluate*/
        <name>_graph_evaluate(in,out);

        /*Commit all new internal output values or transmit to other graphs*/
        for (i = 0; i < <name>_GRAPH_NO_OUTPUTS; ++i)
            if (output2receiverId[i] == <myId>) {
                commit(i, out[i]);
            } else {
                response.output = i;
                response.input = output2sinkIdx[i];
                response.value = out[i];
                ndlcomNodeSend(&node,output2receiverId[i], &response, sizeof(response));
            }
    }

    /*DEINIT*/
    ndlcomNodeDeinit(&node);
    return 0;
}
