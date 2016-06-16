#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "ndlcom/Bridge.h"
#include "ndlcom/Node.h"
#include "ndlcom/ExternalInterfaceParseUri.hpp"
#include "representations/id.h"
#include "representations/Isp.h"

static struct option long_options[] = {
    {"help",     no_argument,       0, 'h'},
    {"switch",   no_argument,       0, 'x'},
    {"upload",   no_argument,       0, 'u'},
    {"verify",   no_argument,       0, 'v'},
    {"download", no_argument,       0, 'd'},
    {"node_id",  required_argument, 0, 'n'},
    {"address",  required_argument, 0, 'a'},
    {"size",     required_argument, 0, 's'},
    {"uri",      required_argument, 0, 'i'},
    {"my_id",    required_argument, 0, 'm'},
    {0, 0, 0, 0}
};

static char filename[256];
static NDLComId myId = 0x01;
static char uri[256];

enum ispAction {
    ISP_ACTION_NONE,
    ISP_ACTION_UPLOAD,
    ISP_ACTION_DOWNLOAD,
    ISP_ACTION_VERIFY,
    ISP_ACTION_SWITCH
};

enum ispState {
    ISP_STATE_IDLE,
    ISP_STATE_ERASING,
    ISP_STATE_UPLOADING,
    ISP_STATE_DOWNLOADING,
    ISP_STATE_VERIFIING,
    ISP_STATE_ERROR
};

struct ispContext {
    enum ispState state;
    FILE *fp;
    struct NDLComNode *node;
    NDLComId targetId;
    unsigned int startAddr;
    unsigned int offset;
    unsigned int length;
};

enum ispAction parse_args(struct ispContext *context, int argc, char **argv);
void print_help(const char* name);
void ispHandler(void *context, const struct NDLComHeader *header, const void *payload, const void *origin);
void ispCmdHandler(struct ispContext *ctx, const struct NDLComHeader *header, const struct representations::IspCommand *cmd);
void ispDataHandler(struct ispContext *ctx, const struct NDLComHeader *header, const struct representations::IspData *data);
void ispSendCmd(struct ispContext *context, const uint8_t cmd, const uint32_t addr, const uint32_t len);
int  ispSendData(struct ispContext *context);

long fileSize(FILE *fp)
{
    long value;
    fseek(fp, 0, SEEK_END);
    value = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return value;
}

int main(int argc, char **argv)
{
    struct NDLComBridge bridge;
    struct NDLComNode node;
    struct NDLComInternalHandler handler;
    struct ispContext context;
    enum ispAction action = ISP_ACTION_NONE;
    unsigned int percentage = 0;;
    unsigned int lastPercentage = 0;;

    context.state = ISP_STATE_IDLE;
    context.offset = 0;
    context.fp = NULL;
    context.node = &node;
    context.length = 0;

    if ((action = parse_args(&context, argc, argv)) == ISP_ACTION_NONE) return -1;

    // I. Setup ndlcom stuff
    ndlcomBridgeInit(&bridge);
    ndlcomNodeInit(&node, &bridge, myId);
    ndlcom::ParseUriAndCreateExternalInterface(std::cerr, bridge, uri);
    ndlcomInternalHandlerInit(&handler, ispHandler, 0, &context);
    ndlcomNodeRegisterInternalHandler(&node, &handler);

    // II. Prepare actions
    switch (action)
    {
        case ISP_ACTION_SWITCH:
            printf("Switching between images at device %u\n", context.targetId);
            ispSendCmd(&context, ISP_CMD_EXECUTE, context.startAddr, context.length);
            ndlcomBridgeProcessOnce(&bridge);
            return 0;
        case ISP_ACTION_UPLOAD:
            printf("Uploading '%s' to device %u: ", filename, context.targetId);
            // Open file for reading
            context.fp = fopen(filename, "r");
            if (!context.fp)
            {
                fprintf(stderr, "Could not open file '%s'\n", filename);
                return -1;
            }
            // Check size
            if (context.length < 1)
                context.length = fileSize(context.fp);
            // Send upload command
            ispSendCmd(&context, ISP_CMD_UPLOAD, context.startAddr, context.length);
            // Update state machine
            context.state = ISP_STATE_ERASING;
            break;
        case ISP_ACTION_DOWNLOAD:
            printf("Downloading to '%s' from device %u: ", filename, context.targetId);
            // Open file for writing
            context.fp = fopen(filename, "w");
            if (!context.fp)
            {
                fprintf(stderr, "Could not open file '%s'\n", filename);
                return -1;
            }
            // Send first download command
            ispSendCmd(&context, ISP_CMD_DOWNLOAD, context.startAddr, ISP_DATA_TRANSMISSION_BLOCK_SIZE);
            // Update state machine
            context.state = ISP_STATE_DOWNLOADING;
            break;
        case ISP_ACTION_VERIFY:
        default:
            printf("Verifiing '%s' and content at device %u: ", filename, context.targetId);
            // Open file for reading
            context.fp = fopen(filename, "r");
            if (!context.fp)
            {
                fprintf(stderr, "Could not open file '%s'\n", filename);
                return -1;
            }
            // Check size
            if (context.length < 1)
                context.length = fileSize(context.fp);
            // Send first download command
            ispSendCmd(&context, ISP_CMD_DOWNLOAD, context.startAddr, ISP_DATA_TRANSMISSION_BLOCK_SIZE);
            // Update state machine
            context.state = ISP_STATE_VERIFIING;
            break;
    }

    // III. Main loop for handling ndlcom packets
    while ((context.state != ISP_STATE_IDLE) && (context.state != ISP_STATE_ERROR))
    {
        // Every percent we print a '.'
        percentage = context.offset * 100 / context.length;
        if (percentage != lastPercentage)
        {
            printf(".");
            lastPercentage = percentage;
        }
        ndlcomBridgeProcessOnce(&bridge);
    }

    // IV. Check if we have been successful
    switch (context.state)
    {
        case ISP_STATE_IDLE:
            printf(" DONE\n");
            return 0;
        case ISP_STATE_ERROR:
            switch (action)
            {
                case ISP_ACTION_VERIFY:
                    fprintf(stderr, " Verification failed at offset 0x%x\n", context.offset);
                    break;
                default:
                    fprintf(stderr, " In state error but dont know why ...\n");
                    break;
            }
            break;
        default:
            break;
    }

    return -1;
}

void ispSendCmd(struct ispContext *context, const uint8_t cmd, const uint32_t addr, const uint32_t len)
{
    struct representations::IspCommand command;
    command.mBase.mId = REPRESENTATIONS_REPRESENTATION_ID_IspCommand;
    command.mCommand = cmd;
    command.mAddress = addr;
    command.mLength = len;

    ndlcomNodeSend(context->node, context->targetId, &command, sizeof(command));
}

int ispSendData(struct ispContext *context)
{
    struct representations::IspData data;
    const int n = (context->length - context->offset > ISP_DATA_TRANSMISSION_BLOCK_SIZE)?ISP_DATA_TRANSMISSION_BLOCK_SIZE:context->length - context->offset;
    int bytes_read = 0;
    data.mBase.mId = REPRESENTATIONS_REPRESENTATION_ID_IspData;
    data.mAddress = context->startAddr + context->offset;

    // NOTE: n specifies how many bytes we WANT to send, bytes_read tells us how many we CAN send
    if (n>0) {
        if ((bytes_read = fread(data.mData, 1, n, context->fp)) > 0)
            ndlcomNodeSend(context->node, context->targetId, &data, sizeof(data));
    }
    return bytes_read;
}

void ispCmdHandler(struct ispContext *ctx, const struct NDLComHeader *header, const struct representations::IspCommand *cmd)
{
    // When we get an ACK and are in state UPLOADING, we read content from file and transmit it as DATA packet
    switch (cmd->mCommand)
    {
        case ISP_CMD_ACK:
            // Got an ACK, so we can proceed
            switch (ctx->state)
            {
                case ISP_STATE_UPLOADING:
                    // Update offset
                    ctx->offset += ISP_DATA_TRANSMISSION_BLOCK_SIZE;
                case ISP_STATE_ERASING:
                    // Check if we had transmitted data
                    if (ispSendData(ctx) > 0)
                    {
                        ctx->state = ISP_STATE_UPLOADING;
                    } else {
                        // Ready :)
                        ctx->state = ISP_STATE_IDLE;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void ispDataHandler(struct ispContext *ctx, const struct NDLComHeader *header, const struct representations::IspData *data)
{
    // When we get a data packet AND are in state DOWNLOADING, we write content to file and transmit a new DOWNLOAD command
    const int n = (ctx->length - ctx->offset > ISP_DATA_TRANSMISSION_BLOCK_SIZE)?ISP_DATA_TRANSMISSION_BLOCK_SIZE:ctx->length - ctx->offset;
    int i;
    uint8_t buffer[ISP_DATA_TRANSMISSION_BLOCK_SIZE];

    switch (ctx->state)
    {
        case ISP_STATE_VERIFIING:
            // Check if addresses match
            if (ctx->startAddr+ctx->offset != data->mAddress)
            {
                ctx->state = ISP_STATE_ERROR;
                break;
            }
            // Read from file and compare with received data
            fread(buffer, 1, n, ctx->fp);
            for (i = 0; i < n; ++i)
            {
                if (data->mData[i] != buffer[i])
                {
                    ctx->state = ISP_STATE_ERROR;
                    break;
                }
                // Update offset (useful for backtracking)
                ctx->offset++;
            }

            // Proceed if comparison was successful
            if (ctx->state != ISP_STATE_ERROR)
            {
                // Check if we still have to read data
                if (ctx->offset >= ctx->length)
                {
                    // Ready :)
                    ctx->state = ISP_STATE_IDLE;
                } else {
                    // Download next packet
                    ispSendCmd(ctx, ISP_CMD_DOWNLOAD, ctx->startAddr + ctx->offset, ISP_DATA_TRANSMISSION_BLOCK_SIZE);
                    ctx->state = ISP_STATE_DOWNLOADING;
                }
            }
            break;
        case ISP_STATE_DOWNLOADING:
            // Check if addresses match
            if (ctx->startAddr+ctx->offset != data->mAddress)
            {
                ctx->state = ISP_STATE_ERROR;
                break;
            }
            // Write data to file
            fwrite(data->mData, 1, n, ctx->fp);
            // Update offset
            ctx->offset += n;
            // Check if we still have to read data
            if (ctx->offset >= ctx->length)
            {
                // Ready :)
                ctx->state = ISP_STATE_IDLE;
            } else {
                // Download next packet
                ispSendCmd(ctx, ISP_CMD_DOWNLOAD, ctx->startAddr + ctx->offset, ISP_DATA_TRANSMISSION_BLOCK_SIZE);
                ctx->state = ISP_STATE_DOWNLOADING;
            }
            break;
        default:
            break;
    }
}

void ispHandler(void *context, const struct NDLComHeader *header, const void *payload, const void *origin)
{
    // Handle incoming isp stuff
    const struct representations::Representation *repr = (const struct representations::Representation *)payload;
    struct ispContext *ctx = (struct ispContext *)context;

    if (header->mSenderId != ctx->targetId)
        return;

    switch (repr->mId)
    {
        case REPRESENTATIONS_REPRESENTATION_ID_IspCommand:
            // Call command handler
            ispCmdHandler(ctx, header, (const struct representations::IspCommand *)repr);
            break;
        case REPRESENTATIONS_REPRESENTATION_ID_IspData:
            // Call data handler
            ispDataHandler(ctx, header, (const struct representations::IspData *)repr);
            break;
        default:
            break;
    }
}

enum ispAction parse_args(struct ispContext *context, int argc, char **argv)
{
    enum ispAction action = ISP_ACTION_NONE;
    int c;
     
    while (1) {
        /* getopt_long stores the option index here. */
        int option_index = 0;
     
        c = getopt_long (argc, argv, "abc:d:f:",
                         long_options, &option_index);
     
        /* Detect the end of the options. */
        if (c == -1)
            break;
     
        switch (c) {
     
        case 'h': // help
            print_help(argv[0]);
            exit(-1);
     
        case 'u': // upload
            action = ISP_ACTION_UPLOAD;
            break;

        case 'v':
            action = ISP_ACTION_VERIFY;
            break;

        case 'd':
            action = ISP_ACTION_DOWNLOAD;
            break;

        case 'x':
            action = ISP_ACTION_SWITCH;
            break;

        case 'i':
            snprintf(uri, 256, "%s", optarg);
            break;

        case 'm':
            myId = atoi(optarg);
            break;

        case 'n':
            context->targetId = atoi(optarg);
            break;

        case 'a':
            sscanf(optarg,"0x%x",&(context->startAddr));
            break;

        case 's':
            context->length = atoi(optarg);
            break;
     
        default:
            break;
        }
    }
     
    // copy filename argument
    if (optind < argc)
    {
        strncpy(filename,argv[optind],256);
    }
    else if (action != ISP_ACTION_SWITCH)
    {
        print_help(argv[0]);
        exit(-1);
    }

    // Check size
    if ((context->length < 1) && (action == ISP_ACTION_DOWNLOAD))
    {
        fprintf(stderr, "Size has to be greater than zero\n");
        exit(-1);
    }

    return action;
}

void print_help(const char* name)
{
    printf("Usage: %s [options]\n", name);
    printf("Options:\n");
    printf("  --help           Display this information\n");
    printf("  --switch         Switches between FIRMWARE and BOOTLOADER (only some devices)\n");
    printf("  --node_id=<id>   Node id of the device to program\n");
    printf("  --address=<addr> address (hex) to write bin-file to (default 0x0)\n");
    printf("  --size=<size>    Size of the data to download (default 0)\n");
    printf("  --uri=<uri>      An URI to the interface for data transmission and reception\n");
    printf("  --my_id=<id>     An id to be used for ISP (default 0x01)\n");
    printf("\nThe following commands need a binary file argument\n");
    printf("  --upload         Upload a bin-file\n");
    printf("  --verify         Verify a bin-file (default)\n");
    printf("  --download       Download data and store it to a file (--size=<size> required)\n");
}

