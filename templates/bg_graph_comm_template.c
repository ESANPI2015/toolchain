#include "<name>_comm.h"
#include "representations/id.h"
#include "representations/BGData.h"
/*#include "ndlcom/Types.h"*/

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

static <type> default_sample(const unsigned int input)
{
    return 0.0f;
}

static void default_commit(const unsigned int output, const <type> value)
{
}

static void <name>_ndlcomBGDataHandler(void *context, const struct NDLComHeader *header, const void *payload)
{
    <name>_comm_t *comm = (<name>_comm_t *)context;
    unsigned int input;
    struct BGData *incoming = (struct BGData *)payload;

    /*Check packet*/
    if (header->mDataLen != sizeof(struct BGData))
        return;
    /*Find input to be updated and ignore internal inputs*/
    for (input = 0; input < <name>_GRAPH_NO_INPUTS; ++input) {
        if (input2senderId[input] == <name>_DEVICE_ID)
            continue;
        if (input2srcIdx[input] == incoming->input)
            break;
    }
    /*If not found, return*/
    if (input == <name>_GRAPH_NO_INPUTS)
        return;

    /*Update internal values iff it hasn't been updated yet*/
    if (!comm->numUpdates[input]) {
        comm->in[input] = incoming->value;
        comm->numUpdates[input]++;
    }
}

/*This function registers handlers to the passed node to enable BG processing*/
uint8_t <name>_init(<name>_comm_t *comm, struct NDLComNode *node, const <name>_getInternalInputFunc sample, const <name>_setInternalOutputFunc commit)
{
    unsigned int i;
    /*Reset inputs and number of updates*/
    for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i)
    {
        comm->in[i] = 0;
        /*TODO: Initialize this to 1 when a backedge has been detected as incoming*/
        comm->numUpdates[i] = 0;
    }
    /*Initialize graph*/
    <name>_graph_init(&comm->graph);
    /*Register handlers iff given*/
    comm->sample = default_sample;
    comm->commit = default_commit;
    if (sample)
        comm->sample = sample;
    if (commit)
        comm->commit = commit;
    /*Setup ndlcom node iff given*/
    comm->node = NULL;
    if (node) {
        comm->node = node;
        ndlcomInternalHandlerInit(&comm->handler, <name>_ndlcomBGDataHandler, 0, comm);
        ndlcomNodeRegisterInternalHandler(comm->node, &comm->handler);
        ndlcomNodeSetOwnSenderId(comm->node, <name>_DEVICE_ID);
    }
    return 1;
}

/*This function has to be called whenever the behaviour graph is allowed to be evaluated*/
uint8_t <name>_process(<name>_comm_t *comm)
{
    uint8_t trigger = 1;
    unsigned int i;
    struct BGData response;

    /*TODO: Add checks and initializations*/
    response.mBase.mId = REPRESENTATIONS_REPRESENTATION_ID_BGData;

    /*Read all external inputs*/
    for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i) {
        /*Skip internal inputs*/
        if (input2senderId[i] == <name>_DEVICE_ID)
            continue;
        /*Missing update*/
        if (comm->numUpdates[i] < 1) {
            trigger = 0;
            break;
        }
    }
    if (!trigger)
        return trigger;

    /*Read all internal inputs*/
    for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i) {
        if (input2senderId[i] == <name>_DEVICE_ID)
            comm->in[i] = comm->sample(i);
    }

    /*Evaluate behaviour graph*/
    <name>_graph_evaluate(&comm->graph,comm->in,comm->out);

    /*Commit all internal outputs*/
    for (i = 0; i < <name>_GRAPH_NO_OUTPUTS; ++i) {
        if (output2receiverId[i] == <name>_DEVICE_ID)
            comm->commit(i, comm->out[i]);
    }

    /*Transmit all external outputs*/
    for (i = 0; i < <name>_GRAPH_NO_OUTPUTS; ++i) {
        /*Skip internal outputs*/
        if (output2receiverId[i] == <name>_DEVICE_ID)
            continue;
        /*Transmit new value*/
        response.output = i;
        response.input = output2sinkIdx[i];
        response.value = comm->out[i];
        ndlcomNodeSend(comm->node,output2receiverId[i], &response, sizeof(response));
    }

    return trigger;
}
