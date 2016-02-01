#include "<name>_comm.h"
#include "representations/id.h"
#include "representations/BGData.h"
#include <stdbool.h>

/*Internal Input Definitions*/
static const uint8_t internalInputSinkIdx[<name>_NO_INTERNAL_INPUTS+1] = {
    <internalInputSinkIdx0>
};
/*External Input Definitions*/
static const uint8_t externalInputSenderId[<name>_NO_EXTERNAL_INPUTS+1] = {
    <externalInputSrcId0>
};
static const uint8_t externalInputSrcIdx[<name>_NO_EXTERNAL_INPUTS+1] = { /*Not needed*/
    <externalInputSrcIdx0>
};
static const uint8_t externalInputSinkIdx[<name>_NO_EXTERNAL_INPUTS+1] = {
    <externalInputSinkIdx0>
};
static const bool externalInputHasBackedge[<name>_NO_EXTERNAL_INPUTS+1] = {
    <externalInputHasBackedge0>
};

/*Internal Output Definitions*/
static const uint8_t internalOutputSrcIdx[<name>_NO_INTERNAL_OUTPUTS+1] = {
    <internalOutputSrcIdx0>
};
/*External Output Definitions*/
static const uint8_t externalOutputReceiverId[<name>_NO_EXTERNAL_OUTPUTS+1] = {
    <externalOutputSinkId0>
};
static const uint8_t externalOutputSrcIdx[<name>_NO_EXTERNAL_OUTPUTS+1] = {
    <externalOutputSrcIdx0>
};
static const uint8_t externalOutputSinkIdx[<name>_NO_EXTERNAL_OUTPUTS+1] = {
    <externalOutputSinkIdx0>
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

    /*Update internal value iff it hasn't been updated yet*/
    if (comm->numUpdates[input] < <name>_MAX_NO_UPDATES) {
        comm->in[comm->numUpdates[input]][externalInputSinkIdx[input]] = incoming->value;
        comm->numUpdates[input]++;
    } else {
        /*If we come up here, we have data loss!!!*/
    }
}
#endif

/*This function registers handlers to the passed node to enable BG processing*/
uint8_t <name>_init(<name>_comm_t *comm, struct NDLComNode *node, const <name>_getInternalInputFunc sample, const <name>_setInternalOutputFunc commit)
{
    unsigned int i,j;
    /*Reset inputs*/
    for (i = 0; i < <name>_MAX_NO_UPDATES; ++i)
        for (j = 0; j < <name>_GRAPH_NO_INPUTS; ++j)
            comm->in[i][j] = 0;
    /*Set initial number of updates*/
    for (i = 0; i < <name>_NO_EXTERNAL_INPUTS; ++i)
        comm->numUpdates[i] = (externalInputHasBackedge[i]) ? 1 : 0;
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
    unsigned int i, j;
    struct BGData response;

    response.mBase.mId = REPRESENTATIONS_REPRESENTATION_ID_BGData;

    /*Check if we should fire:
     * The idea:
     * * Iff all inputs have seen an update we evaluate and produce new output
     * * Iff we have seen multiple updates, we evaluate immediately to (using last known values of possibly missing input updates)
     *   a) prevent dead-locks
     *   b) be always in sync even in case of packet loss
     *   c) upsample slower sources with respect to the source with maximum rate
     * * In all other cases, we have to wait */

    /*Read all external inputs*/
    for (i = 0; i < <name>_NO_EXTERNAL_INPUTS; ++i) {
        /*Trigger immediately iff
         * a) sample rates of incoming values mismatch (upsampling)
         * b) a input value has been lost during transmission*/
        if (comm->numUpdates[i] > <name>_MAX_NO_UPDATES/2) {
            trigger = 1;
            break;
        }
        /*Missing update
         * a) reset trigger but keep checking other inputs*/
        if (comm->numUpdates[i] < 1)
            trigger = 0;
    }
    if (!trigger)
        return trigger;

    /*Read all internal inputs*/
    for (i = 0; i < <name>_NO_INTERNAL_INPUTS; ++i) {
        comm->in[0][internalInputSinkIdx[i]] = comm->sample(i);
    }

    /*Evaluate behaviour graph*/
    <name>_graph_evaluate(&comm->graph,comm->in[0],comm->out);

    /*Update queues (NOT THREAD SAFE!)*/
    for (i = 0; i < <name>_NO_EXTERNAL_INPUTS; ++i) {
        /*Skip missing updates*/
        if (comm->numUpdates[i] < 1)
            continue;
        /*Update 'queue'*/
        for (j = 0; j < comm->numUpdates[i]-1; ++j)
            comm->in[j][i] = comm->in[j+1][i];
        /*Decrease numUpdates*/
        comm->numUpdates[i]--;
    }

    /*Commit all internal outputs*/
    for (i = 0; i < <name>_NO_INTERNAL_OUTPUTS; ++i) {
        comm->commit(i, comm->out[internalOutputSrcIdx[i]]);
    }

    /*Transmit all external outputs*/
    for (i = 0; i < <name>_NO_EXTERNAL_OUTPUTS; ++i) {
        /*Transmit new value*/
        response.output = externalOutputSrcIdx[i];
        response.input = externalOutputSinkIdx[i];
        response.value = comm->out[externalOutputSrcIdx[i]];
        ndlcomNodeSend(comm->node,externalOutputReceiverId[i], &response, sizeof(response));
    }

    return trigger;
}
