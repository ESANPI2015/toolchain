#include "sw_to_hw_map.h"
#include "priority_list.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int defaultCost(sw2hw_map_t *map, unsigned int hwId, unsigned int swId)
{
    return (costByNeighbours(map, hwId, swId) + costByAssignments(map, hwId, swId));
}

int main (int argc, char *argv[])
{
    int opt;
    char hwYamlFile[HWG_MAX_STRING_LENGTH];
    char swYamlFile[bg_MAX_STRING_LENGTH];
    char name[bg_MAX_STRING_LENGTH];
    sw2hw_map_t mapping;
    sw2hw_map_entry_t *entry;
    priority_list_iterator_t it;
    hw_node_t *target;
    bg_graph_t *graph;

    printf("SW to HW Mapper\n");

    while ((opt = getopt(argc, argv, "h")) != -1)
    {
        switch (opt)
        {
            case 'h':
            default:
                fprintf(stderr, "Usage: %s hw_spec sw_spec\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    if ((optind >= argc) || (argc < 3))
    {
        fprintf(stderr, "Usage: %s hw_spec sw_spec\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    snprintf(hwYamlFile, HWG_MAX_STRING_LENGTH, "%s", argv[optind]);
    snprintf(swYamlFile, bg_MAX_STRING_LENGTH, "%s", argv[optind+1]);

    bg_initialize();

    sw2hw_map_init(&mapping, hwYamlFile, swYamlFile);
    sw2hw_map_match(&mapping);
    sw2hw_map_initial(&mapping, defaultCost);
    do {
        while (sw2hw_map_refine(&mapping, defaultCost) == SW2HW_ERR_NONE);
    } while (sw2hw_map_regroup(&mapping, defaultCost) == SW2HW_ERR_NONE);

    for (entry = priority_list_first(mapping.assignments, &it);
            entry;
            entry = priority_list_next(&it))
    {
        fprintf(stderr, "SW ID %u -> TARGET %u GLOBAL COST: %d\n", entry->swId, entry->hwId, priority_list_get_priority(&it));
    }

    /*For each target create a subgraph*/
    for (target = priority_list_first(mapping.hwGraph->nodes, &it);
            target;
            target = priority_list_next(&it))
    {
        fprintf(stderr, "Generate Behaviour Graph for target %u\n", target->id);
        snprintf(name, bg_MAX_STRING_LENGTH, "deploy_to_%u.yml", target->id);
        bg_graph_alloc(&graph, name);
        sw2hw_map_create_subgraph(&mapping, target->id, graph);
        bg_graph_to_yaml_file(name, graph);
        bg_graph_free(graph);
    }

    sw2hw_map_destroy(&mapping);

    bg_terminate();
    exit(EXIT_SUCCESS);
}
