/*
 * NOTE: This template is not finished yet because it is target-dependent
 * Long-term vision:
 * * Provide the ndlcomBGDataHandler or ndlcomNodeRegisterBGDataHandler
 * * Provide the device id to be used by target
 */
#include "<name>_graph.h"
#include "ndlcom/Bridge.h"
#include "ndlcom/Node.h"
#include "representations/id.h"
#include "representations/BGData.h"

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
static bool gotUpdate[<name>_GRAPH_NO_INPUTS] = { false };

/*NOTE: These are communication dependent entities*/
static struct NDLComBridge bridge;
static struct NDLComNode node;
static struct NDLComInternalHandler handler;
static struct NDLComExternalInterface interface;

size_t readBuf (void *context, void *buf, const size_t count)
{
    /*NOTE: This is target dependent code!*/
}
void writeBuf (void *context, const void *buf, const size_t count)
{
    /*NOTE: This is target dependent code!*/
}

void on_incoming_packet(void *context, const struct NDLComHeader *header, const void *payload)
{
    struct NDLComNode *node = (struct NDLComNode *)context;
    const unsigned int input, i;
    struct BGData *incoming = (struct BGData *)payload;
    struct BGData response;
    bool trigger = false;

    /*Check packet*/
    if (header->mDataLen != sizeof(struct BGData))
        return;
    /*Find input to be updated*/
    for (input = 0; input < <name>_GRAPH_NO_INPUTS; ++input)
        if (input2srcIdx[input] == incoming->input)
            break;
    /*If not found, return*/
    if (input == <name>_GRAPH_NO_INPUTS)
        return;
    /*Update value*/
    in[input] = payload->value;
    /*Check if we should fire:
     * The idea:
     * * Iff all inputs have seen an update we evaluate and produce new output
     * * If a packet has been lost such that we received a double update, we trigger to
     *   a) prevent dead-locks
     *   b) be always in sync even in case of packet loss*/
    if (gotUpdate[input])
    {
        /*Got double update*/
        trigger = true;
        /*Reset all update flags except ours*/
        for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i)
            gotUpdate[i] = false;
        gotUpdate[input] = true;
    } else {
        /*Got first update*/
        trigger = true;
        gotUpdate[input] = true;
        /*Check if all inputs have seen an update*/
        for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i)
            if (!gotUpdate[i])
                trigger = false;
        /*When we trigger, we reset all update flags*/
        if (trigger)
            for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i)
                gotUpdate[i] = false;
    }

    /*Fire at will :D*/
    if (!trigger)
        return;

    <name>_graph_evaluate(in,out);

    response.mBase.mId = REPRESENTATIONS_REPRESENTATION_ID_BGData;
    /*Send all output values*/
    for (i = 0; i < <name>_GRAPH_NO_OUTPUTS; ++i) {
        response.output = i;
        response.input = output2sinkIdx[i];
        response.value = out[i];
        ndlcomNodeSend(node,output2receiverId[i], &response, sizeof(response));
    }
}

void main(void)
{
    /*INIT*/
    ndlcomBridgeInit(&bridge);
    ndlcomNodeInit(&node, &bridge, <myId>);
    ndlcomInternalHandlerInit(&handler, on_incoming_packet, 0, &node);
    ndlcomNodeRegisterInternalHandler(&node, &handler);
    ndlcomExternalInterfaceInit(&interface, writeBuf, readBuf, 0, FD);

    /*LOOP*/
    while (1) {
        ndlcomBridgeProcess(&bridge);
    }

    /*DEINIT*/
    ndlcomNodeDeinit(&node);
}
