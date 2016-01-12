#include "sw_to_hw_map.h"
#include "sw_to_hw_map_yaml.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

static bg_error generateSubGraph(sw2hw_map_t *map, const hw_node_t *target, const char* prefix)
{
    bg_error err;
    bg_graph_t *graph;
    char name[bg_MAX_STRING_LENGTH];

    snprintf(name, bg_MAX_STRING_LENGTH, "%s_%u.bg", prefix, target->id);
    fprintf(stderr, "Generate Behaviour Graph for target %u to file \"%s\"\n", target->id, name);
    err = bg_graph_alloc(&graph, name);
    sw2hw_map_create_subgraph(map, target->id, graph);
    err = bg_graph_to_yaml_file(name, graph);
    err = bg_graph_free(graph);
    return err;
}

int main (int argc, char *argv[])
{
    int opt;
    sw2hw_map_t mapping;
    hw_node_t *target;
    priority_list_iterator_t it;
    char mapFileName[SW2HW_MAX_STRING_LENGTH];
    char targetArg[SW2HW_MAX_STRING_LENGTH];
    char prefix[bg_MAX_STRING_LENGTH];
    unsigned int hwId;
    bool allTargets = true;

    printf("Map to Behaviour Graph\n");
    while ((opt = getopt(argc, argv, "ht:")) != -1)
    {
        switch (opt)
        {
            case 't':
                snprintf(targetArg, SW2HW_MAX_STRING_LENGTH, "%s", optarg);
                allTargets = false;
                break;
            case 'h':
            default:
                fprintf(stderr, "Usage: %s [-t target] map prefix-of-output\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    if ((optind >= argc) || (argc < 3))
    {
        fprintf(stderr, "Usage: %s [-t target] map prefix-of-output\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    snprintf(mapFileName, SW2HW_MAX_STRING_LENGTH, "%s", argv[optind]);
    snprintf(prefix, bg_MAX_STRING_LENGTH, "%s", argv[optind+1]);

    bg_initialize();
    sw2hw_map_from_yaml_file(mapFileName, &mapping);
    if (!priority_list_size(mapping.assignments))
    {
        fprintf(stderr, "No assignments found\n");
        exit(EXIT_FAILURE);
    }

    if (allTargets)
    {
        for (target = priority_list_first(mapping.hwGraph->nodes, &it);
                target;
                target = priority_list_next(&it))
            generateSubGraph(&mapping, target, prefix);
    } else {
        if (!sscanf(targetArg, "%u", &hwId))
        {
            for (target = priority_list_first(mapping.hwGraph->nodes, &it);
                    target;
                    target = priority_list_next(&it))
                if (strcmp(target->name, targetArg) == 0)
                    hwId = target->id;
        }
        target = hw_graph_get_node(mapping.hwGraph, hwId);
        if (!target)
        {
            fprintf(stderr, "Could not find target\n");
            exit(EXIT_FAILURE);
        }
        printf("Using target id %u with name %s\n", target->id, target->name);
        generateSubGraph(&mapping, target, prefix);
    }

    sw2hw_map_destroy(&mapping);
    bg_terminate();
    exit(EXIT_SUCCESS);
}
