#include "sw_to_hw_map.h"
#include "sw_to_hw_map_yaml.h"
#include "node_list.h"
#include "node_types/bg_node_subgraph.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[])
{
    int opt;
    sw2hw_map_t mapping, transformed;
    bg_graph_t *sub;
    bg_node_t *node;
    bg_node_list_iterator_t it;
    char mapFileName[SW2HW_MAX_STRING_LENGTH];
    char outFileName[SW2HW_MAX_STRING_LENGTH];
    char prefix[bg_MAX_STRING_LENGTH];
    char fileName[bg_MAX_STRING_LENGTH];

    printf("Mapping to Behaviour Graph(s)\n");
    while ((opt = getopt(argc, argv, "hp:")) != -1)
    {
        switch (opt)
        {
            case 'p':
                snprintf(prefix, bg_MAX_STRING_LENGTH, "%s", optarg);
                break;
            case 'h':
            default:
                fprintf(stderr, "Usage: %s [-p prefix-of-subgraphs] map-file transformed-map-file\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    if ((optind >= argc) || (argc < 3))
    {
        fprintf(stderr, "Usage: %s [-p prefix-of-subgraphs] map-file transformed-map-file\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    snprintf(mapFileName, SW2HW_MAX_STRING_LENGTH, "%s", argv[optind]);
    snprintf(outFileName, SW2HW_MAX_STRING_LENGTH, "%s", argv[optind+1]);

    /*Init*/
    bg_initialize();
    sw2hw_map_init(&mapping, NULL, NULL);
    sw2hw_map_init(&transformed, NULL, NULL);
    sw2hw_map_from_yaml_file(mapFileName, &mapping);
    if (!priority_list_size(mapping.assignments))
    {
        fprintf(stderr, "No assignments found\n");
        exit(EXIT_FAILURE);
    }

    /*Transformation*/
    sw2hw_map_transform(&transformed, &mapping);
    /*Storing*/
    for (node = bg_node_list_first(transformed.swGraph->hidden_nodes, &it);
            node;
            node = bg_node_list_next(&it))
    {
        sub = ((subgraph_data_t *)node->_priv_data)->subgraph;
        /*Ensure that file and node name are the same ... otherwise we loose correspondence*/
        snprintf(fileName, bg_MAX_STRING_LENGTH, "%s_%s.bg", prefix, node->name);
        snprintf((char *)sub->name, bg_MAX_STRING_LENGTH, "%s", fileName);
        bg_graph_to_yaml_file(fileName, ((subgraph_data_t *)node->_priv_data)->subgraph);
    }
    sw2hw_map_to_yaml_file(outFileName, &transformed);

    /*Deinit*/
    sw2hw_map_destroy(&mapping);
    sw2hw_map_destroy(&transformed);
    bg_terminate();
    exit(EXIT_SUCCESS);
}
