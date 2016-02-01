#include "<name>_comm.h"
#include "ndlcom/Bridge.h"
#include "ndlcom/Node.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define <name>_NO_EXTERNAL_INTERFACES <numExternalInterfaces>

float <name>_readInternalInput(const unsigned int i)
{
    /*Implement me*/
    return 0.0f;
}

void <name>_writeInternalOutput(const unsigned int i, const <type> value)
{
    /*Implement me*/
}

struct <name>_context
{
    unsigned int interface;
    /*Add everything here, the read and write methods need to know*/
};

size_t <name>_readExternalInput (void *context, void *buf, const size_t count)
{
    struct <name>_context *ctx = (struct <name>_context *)context;
    switch (ctx->interface)
    {
        /*Implement me*/
        default:
            fprintf(stderr, "<name>_readExternalInput: Unknown interface\n");
            return 0;
    }
}

void <name>_writeExternalOutput(void *context, const void *buf, const size_t count)
{
    struct <name>_context *ctx = (struct <name>_context *)context;
    switch (ctx->interface)
    {
        /*Implement me*/
        default:
            fprintf(stderr, "<name>_writeExternalOutput: Unknown interface\n");
            return;
    }
}

int main ()
{
    <name>_comm_t comm;
    struct NDLComNode node;
    struct NDLComBridge bridge;
    struct NDLComExternalInterface interfaces[<name>_NO_EXTERNAL_INTERFACES];
    struct <name>_context contexts[<name>_NO_EXTERNAL_INTERFACES];
    unsigned int i;

    /*Initialize ndlcom*/
    ndlcomBridgeInit(&bridge);
    ndlcomNodeInit(&node, &bridge, <name>_DEVICE_ID);
    for (i = 0; i < <name>_NO_EXTERNAL_INTERFACES; ++i) {
        contexts[i].interface = i;
        ndlcomExternalInterfaceInit(&interfaces[i], <name>_writeExternalOutput, <name>_readExternalInput, 0, &contexts[i]);
        ndlcomBridgeRegisterExternalInterface(&bridge, &interfaces[i]);
    }

    /*Initialize the communication layer and the behaviour graph*/
    <name>_init(&comm, &node, <name>_readInternalInput, <name>_writeInternalOutput);

    /*Keep calling the graph and the ndlcom processes*/
    while (1)
    {
        <name>_process(&comm);
        ndlcomBridgeProcessOnce(&bridge);
    }

    return 0;
}
