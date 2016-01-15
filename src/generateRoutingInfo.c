#include "sw_to_hw_map.h"
#include "sw_to_hw_map_yaml.h"
#include "bg_graph.h"
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
    char temp[SW2HW_MAX_STRING_LENGTH];
    unsigned int input, output;
    FILE *dictionary;
    dictEntry dict;
    sw2hw_map_entry_t *entry;
    bg_node_t *node;
    hw_node_t *target;
    priority_list_iterator_t it;

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

    /*Cycle through all assignments*/
    for (entry = priority_list_first(mapping.assignments, &it);
            entry;
            entry = priority_list_next(&it))
    {
        bg_graph_find_node(mapping.swGraph, entry->swId, &node);
        target = hw_graph_get_node(mapping.hwGraph, entry->hwId);
        if (strcmp(target->name, targetName) != 0)
            continue;

        dictionary = fopen(outFileName, "a");
        if (!dictionary)
        {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        for (input = 0; input < node->input_port_cnt; input++)
        {        
            sprintf(temp, ",\n");
            if (input == node->input_port_cnt - 1)
                sprintf(temp, " ");
            if (node->input_ports[input]->num_edges != 1)
            {
                sprintf(dict.token, "<input2srcId%u>", input);
                sprintf(dict.repl, "<myId>%s<input2srcId%u>", temp, input+1);
                writeDictionary(dictionary, &dict);
                sprintf(dict.token, "<input2srcIdx%u>", input);
                sprintf(dict.repl, "%u%s<input2srcIdx%u>", input, temp, input+1);
                writeDictionary(dictionary, &dict);
            } else {
                sprintf(dict.token, "<input2srcId%u>", input);
                sprintf(dict.repl, "%u%s<input2srcId%u>", (unsigned int)node->input_ports[input]->edges[0]->source_node->id, temp, input+1);
                writeDictionary(dictionary, &dict);
                sprintf(dict.token, "<input2srcIdx%u>", input);
                sprintf(dict.repl, "%u%s<input2srcIdx%u>", (unsigned int)node->input_ports[input]->edges[0]->source_port_idx, temp, input+1);
                writeDictionary(dictionary, &dict);
            }
        }
        sprintf(dict.repl, " ");
        sprintf(dict.token, "<input2srcId%u>", input);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<input2srcIdx%u>", input);
        writeDictionary(dictionary, &dict);
        for (output = 0; output < node->output_port_cnt; output++)
        {        
            sprintf(temp, ",\n");
            if (output == node->output_port_cnt - 1)
                sprintf(temp, " ");
            if (node->output_ports[output]->num_edges != 1)
            {
                sprintf(dict.token, "<output2sinkId%u>", output);
                sprintf(dict.repl, "<myId>%s<output2sinkId%u>", temp, output+1);
                writeDictionary(dictionary, &dict);
                sprintf(dict.token, "<output2sinkIdx%u>", output);
                sprintf(dict.repl, "%u%s<output2sinkIdx%u>", output, temp, output+1);
                writeDictionary(dictionary, &dict);
            } else {
                sprintf(dict.token, "<output2sinkId%u>", output);
                sprintf(dict.repl, "%u%s<output2sinkId%u>", (unsigned int)node->output_ports[output]->edges[0]->sink_node->id, temp, output+1);
                writeDictionary(dictionary, &dict);
                sprintf(dict.token, "<output2sinkIdx%u>", output);
                sprintf(dict.repl, "%u%s<output2sinkIdx%u>", (unsigned int)node->output_ports[output]->edges[0]->sink_port_idx, temp, output+1);
                writeDictionary(dictionary, &dict);
            }
        }
        sprintf(dict.repl, " ");
        sprintf(dict.token, "<output2sinkId%u>", output);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<output2sinkIdx%u>", output);
        writeDictionary(dictionary, &dict);
        sprintf(dict.token, "<myId>");
        sprintf(dict.repl, "%u", (unsigned int)node->id);
        writeDictionary(dictionary, &dict);
        fclose(dictionary);
    }

    /*Deinit*/
    sw2hw_map_destroy(&mapping);
    bg_terminate();
    exit(EXIT_SUCCESS);
}
