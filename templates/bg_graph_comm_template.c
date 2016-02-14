#include "<name>_comm.h"
#include "representations/id.h"
#include "representations/BGData.h"

/*Internal Input Definitions*/
static const uint8_t internalInputSinkIdx[<name>_NO_INTERNAL_INPUTS+1] = {
    <internalInputSinkIdx0>
        0
};
/*External Input Definitions*/
static const uint8_t externalInputSenderId[<name>_NO_EXTERNAL_INPUTS+1] = {
    <externalInputSrcId0>
        0
};
static const uint8_t externalInputSrcIdx[<name>_NO_EXTERNAL_INPUTS+1] = { /*Not needed*/
    <externalInputSrcIdx0>
        0
};
static const uint8_t externalInputSinkIdx[<name>_NO_EXTERNAL_INPUTS+1] = {
    <externalInputSinkIdx0>
        0
};
static const bool externalInputHasBackedge[<name>_NO_EXTERNAL_INPUTS+1] = {
    <externalInputHasBackedge0>
        false
};

/*Internal Output Definitions*/
static const uint8_t internalOutputSrcIdx[<name>_NO_INTERNAL_OUTPUTS+1] = {
    <internalOutputSrcIdx0>
        0
};
/*External Output Definitions*/
static const uint8_t externalOutputReceiverId[<name>_NO_EXTERNAL_OUTPUTS+1] = {
    <externalOutputSinkId0>
        0
};
static const uint8_t externalOutputSrcIdx[<name>_NO_EXTERNAL_OUTPUTS+1] = {
    <externalOutputSrcIdx0>
        0
};
static const uint8_t externalOutputSinkIdx[<name>_NO_EXTERNAL_OUTPUTS+1] = {
    <externalOutputSinkIdx0>
        0
};

#if (<name>_NO_EXTERNAL_INPUTS > 0) || (<name>_NO_EXTERNAL_OUTPUTS > 0)
static void <name>_ndlcomBGDataHandler(void *context, const struct NDLComHeader *header, const void *payload)
{
    <name>_comm_t *comm = (<name>_comm_t *)context;
    unsigned int input;
    struct BGData *incoming = (struct BGData *)payload;

    /*Check packet*/
    if (header->mDataLen != sizeof(struct BGData))
        return;
    /*Find input to be updated*/
    for (input = 0; input < <name>_NO_EXTERNAL_INPUTS; ++input) {
        if (externalInputSinkIdx[input] == incoming->input)
            break;
    }
    /*If not found, return*/
    if (input == <name>_NO_EXTERNAL_INPUTS)
        return;

    /*Update internal value and set flag*/
    comm->in[externalInputSinkIdx[input]] = incoming->value;
    comm->gotUpdate[input] = true;
}
#endif

/*This function registers handlers to the passed node to enable BG processing*/
uint8_t <name>_init(<name>_comm_t *comm, struct NDLComNode *node, const <name>_getInternalInputFunc sample, const <name>_setInternalOutputFunc commit)
{
    unsigned int i;
    /*Reset inputs*/
    for (i = 0; i < <name>_GRAPH_NO_INPUTS; ++i)
        comm->in[i] = 0;
    /*Set initial number of updates*/
    for (i = 0; i < <name>_NO_EXTERNAL_INPUTS; ++i)
        comm->gotUpdate[i] = externalInputHasBackedge[i];
    /*Initialize graph*/
    <name>_graph_init(&comm->graph);
    /*Register handlers iff given*/
    comm->sample = NULL;
    comm->commit = NULL;
#if <name>_NO_INTERNAL_INPUTS > 0
    if (sample)
        comm->sample = sample;
    else
        return 0;
#endif
#if <name>_NO_INTERNAL_OUTPUTS > 0
    if (commit)
        comm->commit = commit;
    else
        return 0;
#endif
    /*Setup ndlcom node iff given*/
    comm->node = NULL;
#if (<name>_NO_EXTERNAL_INPUTS > 0) || (<name>_NO_EXTERNAL_OUTPUTS > 0)
    if (node) {
        comm->node = node;
        ndlcomInternalHandlerInit(&comm->handler, <name>_ndlcomBGDataHandler, 0, comm);
        ndlcomNodeRegisterInternalHandler(comm->node, &comm->handler);
        ndlcomNodeSetOwnSenderId(comm->node, <name>_DEVICE_ID);
    } else
        return 0;
#endif
    return 1;
}

/*This function has to be called whenever the behaviour graph is allowed to be evaluated*/
uint8_t <name>_process(<name>_comm_t *comm)
{
    uint8_t trigger = 1;
    unsigned int i;
    struct BGData response;

    response.mBase.mId = REPRESENTATIONS_REPRESENTATION_ID_BGData;

    /*Read all external inputs*/
    for (i = 0; i < <name>_NO_EXTERNAL_INPUTS; ++i) {
        /*Missing update: cannot fire*/
        if (!comm->gotUpdate[i]) {
            trigger = 0;
            break;
        }
    }
    if (!trigger)
        return trigger;

    /*Read all internal inputs*/
    for (i = 0; i < <name>_NO_INTERNAL_INPUTS; ++i) {
        comm->in[internalInputSinkIdx[i]] = comm->sample(i);
    }

    /*Evaluate behaviour graph*/
    <name>_graph_evaluate(&comm->graph,comm->in,comm->out);

    /*Reset all flags*/
    for (i = 0; i < <name>_NO_EXTERNAL_INPUTS; ++i) {
        comm->gotUpdate[i] = false;
    }

    /*Commit all internal outputs*/
    for (i = 0; i < <name>_NO_INTERNAL_OUTPUTS; ++i) {
        comm->commit(i, comm->out[internalOutputSrcIdx[i]]);
    }

    /*Transmit all external outputs*/
    for (i = 0; i < <name>_NO_EXTERNAL_OUTPUTS; ++i) {
        /*Transmit new value*/
        response.input = externalOutputSinkIdx[i];
        response.value = comm->out[externalOutputSrcIdx[i]];
        ndlcomNodeSend(comm->node,externalOutputReceiverId[i], &response, sizeof(response));
    }

    return trigger;
}
