#include "sw_to_hw_map.h"
#include "sw_to_hw_map_yaml.h"
#include "bg_graph.h"
#include "node_list.h"
#include "node_types/bg_node_subgraph.h"
#include "template_engine.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[])
{
    int opt;
    sw2hw_map_t mapping;
    char mapFileName[SW2HW_MAX_STRING_LENGTH];
    char targetName[SW2HW_MAX_STRING_LENGTH];
    char outFileName[SW2HW_MAX_STRING_LENGTH];
    unsigned int i;
    FILE *dictionary;
    dictEntry dict;
    sw2hw_map_entry_t *entry;
    bg_node_t *node, *source;
    hw_node_t *target, *storedTarget;
    priority_list_iterator_t it;
    bg_node_list_iterator_t it2;
    bool isBackedge;
    unsigned int numInternalInputs = 0;
    unsigned int numExternalInputs = 0;
    unsigned int numInternalOutputs = 0;
    unsigned int numExternalOutputs = 0;
    unsigned int numExternalInterfaces = 0;

    printf("Extract routing information from mapping\n");
    while ((opt = getopt(argc, argv, "h")) != -1)
    {
        switch (opt)
        {
            case 'h':
            default:
                fprintf(stderr, "Usage: %s transformed-map-file target output-dictionary\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    if ((optind >= argc) || (argc < 4))
    {
        fprintf(stderr, "Usage: %s transformed-map-file target output-dictionary\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    snprintf(mapFileName, SW2HW_MAX_STRING_LENGTH, "%s", argv[optind]);
    snprintf(targetName, SW2HW_MAX_STRING_LENGTH, "%s", argv[optind+1]);
    snprintf(outFileName, SW2HW_MAX_STRING_LENGTH, "%s", argv[optind+2]);

    /*Init*/
    bg_initialize();
    sw2hw_map_init(&mapping, NULL, NULL);
    sw2hw_map_from_yaml_file(mapFileName, &mapping);
    if (!priority_list_size(mapping.assignments))
    {
        fprintf(stderr, "No assignments found\n");
        exit(EXIT_FAILURE);
    }

    /*Check if transformation has already been performed*/
    if (!sw2hw_map_is_transformed(&mapping))
    {
        fprintf(stderr, "Mapping has not been transformed yet.\n");
        exit(EXIT_FAILURE);
    }

    /*Determine evaluation order to detect backedges*/
    determine_evaluation_order(mapping.swGraph);

    /*Cycle through all assignments*/
    for (entry = priority_list_first(mapping.assignments, &it);
            entry;
            entry = priority_list_next(&it))
    {
        bg_graph_find_node(mapping.swGraph, entry->swId, &node);
        target = hw_graph_get_node(mapping.hwGraph, entry->hwId);
        if (strcmp(target->name, targetName) != 0)
            continue;
        storedTarget = target;

        dictionary = fopen(outFileName, "a");
        if (!dictionary)
        {
            perror("fopen");
            exit(EXIT_FAILURE);
        }

        /*Handle all inputs*/
        for (i = 0; i < node->input_port_cnt; i++)
        {        
            if (node->input_ports[i]->num_edges != 1)
            {
                /*Internal input*/
                sprintf(dict.token, "<internalInputSinkIdx%u>", numInternalInputs);
                sprintf(dict.repl, "%u,<internalInputSinkIdx%u>", i, numInternalInputs+1);
                writeDictionary(dictionary, &dict);
                numInternalInputs++;
            } else {
                /*External input*/
                sprintf(dict.token, "<externalInputSrcId%u>", numExternalInputs);
                sprintf(dict.repl, "%u,<externalInputSrcId%u>", (unsigned int)node->input_ports[i]->edges[0]->source_node->id, numExternalInputs+1);
                writeDictionary(dictionary, &dict);
                sprintf(dict.token, "<externalInputSrcIdx%u>", numExternalInputs);
                sprintf(dict.repl, "%u,<externalInputSrcIdx%u>", (unsigned int)node->input_ports[i]->edges[0]->source_port_idx, numExternalInputs+1);
                writeDictionary(dictionary, &dict);
                sprintf(dict.token, "<externalInputSinkIdx%u>", numExternalInputs);
                sprintf(dict.repl, "%u,<externalInputSinkIdx%u>", i, numExternalInputs+1);
                writeDictionary(dictionary, &dict);

                isBackedge = true;
                /*Check source node evaluation order*/
                for (source = bg_node_list_first(mapping.swGraph->evaluation_order, &it2);
                        source && (source != node);
                        source = bg_node_list_next(&it2))
                    /*If the source is to be evaluated BEFORE the current node, the edge is NO BACKEDGE*/
                    if (source == node->input_ports[i]->edges[0]->source_node) {
                        isBackedge = false;
                        break;
                    }
                sprintf(dict.token, "<externalInputHasBackedge%u>", numExternalInputs);
                if (isBackedge)
                    sprintf(dict.repl, "%s,<externalInputHasBackedge%u>", "true", numExternalInputs+1);
                else
                    sprintf(dict.repl, "%s,<externalInputHasBackedge%u>", "false", numExternalInputs+1);
                writeDictionary(dictionary, &dict);
                numExternalInputs++;
            }
        }
        /*Finalize with sentinels*/
        sprintf(dict.repl, " ");
        sprintf(dict.token, "<externalInputSrcId%u>", numExternalInputs);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<externalInputSrcIdx%u>", numExternalInputs);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<externalInputSinkIdx%u>", numExternalInputs);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<internalInputSinkIdx%u>", numInternalInputs);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<externalInputHasBackedge%u>", numExternalInputs);
        writeDictionary(dictionary, &dict);

        /*Handle all outputs*/
        for (i = 0; i < node->output_port_cnt; i++)
        {        
            if (node->output_ports[i]->num_edges != 1)
            {
                /*Internal output*/
                sprintf(dict.token, "<internalOutputSrcIdx%u>", numInternalOutputs);
                sprintf(dict.repl, "%u,<internalOutputSrcIdx%u>", i, numInternalOutputs+1);
                writeDictionary(dictionary, &dict);
                numInternalOutputs++;
            } else {
                /*External output*/
                sprintf(dict.token, "<externalOutputSinkId%u>", numExternalOutputs);
                sprintf(dict.repl, "%u,<externalOutputSinkId%u>", (unsigned int)node->output_ports[i]->edges[0]->sink_node->id, numExternalOutputs+1);
                writeDictionary(dictionary, &dict);
                sprintf(dict.token, "<externalOutputSinkIdx%u>", numExternalOutputs);
                sprintf(dict.repl, "%u,<externalOutputSinkIdx%u>", (unsigned int)node->output_ports[i]->edges[0]->sink_port_idx, numExternalOutputs+1);
                writeDictionary(dictionary, &dict);
                sprintf(dict.token, "<externalOutputSrcIdx%u>", numExternalOutputs);
                sprintf(dict.repl, "%u,<externalOutputSrcIdx%u>", i, numExternalOutputs+1);
                writeDictionary(dictionary, &dict);
                numExternalOutputs++;
            }
        }
        /*Finalize with sentinels*/
        sprintf(dict.repl, " ");
        sprintf(dict.token, "<externalOutputSinkId%u>", numExternalOutputs);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<externalOutputSinkIdx%u>", numExternalOutputs);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<externalOutputSrcIdx%u>", numExternalOutputs);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<internalOutputSrcIdx%u>", numInternalOutputs);
        writeDictionary(dictionary, &dict);

        /*Finalize*/
        sprintf(dict.token, "<numInternalInputs>");
        sprintf(dict.repl, "%u", numInternalInputs);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<numExternalInputs>");
        sprintf(dict.repl, "%u", numExternalInputs);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<numInternalOutputs>");
        sprintf(dict.repl, "%u", numInternalOutputs);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<numExternalOutputs>");
        sprintf(dict.repl, "%u", numExternalOutputs);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<myId>");
        sprintf(dict.repl, "%u", (unsigned int)node->id);
        writeDictionary(dictionary, &dict);
    }

    /*Count the number of external interfaces*/
    for (i = 0; i < storedTarget->numPorts; ++i)
    {
        if (storedTarget->ports[i].type != HW_PORT_TYPE_NDLCOM)
            continue;
        numExternalInterfaces++;
    }
    sprintf(dict.token, "<numExternalInterfaces>");
    sprintf(dict.repl, "%u", numExternalInterfaces);
    writeDictionary(dictionary, &dict);
    fclose(dictionary);

    /*Deinit*/
    sw2hw_map_destroy(&mapping);
    bg_terminate();
    exit(EXIT_SUCCESS);
}
