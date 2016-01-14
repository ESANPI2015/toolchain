#include "sw_to_hw_map.h"
#include "sw_to_hw_map_yaml.h"
#include "bg_graph.h"
#include "node_types/bg_node_subgraph.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main (int argc, char *argv[])
{
    pid_t pid;
    int opt;
    char mapFile[SW2HW_MAX_STRING_LENGTH];
    char flavourArg[256];
    char subgraphName[bg_MAX_STRING_LENGTH];
    char subName[256];
    char dictName[256];
    char srcName[256];
    char templateName[256];
    sw2hw_map_t mapping;
    sw2hw_map_entry_t *entry;
    bg_graph_t *sub;
    bg_node_t *node;
    hw_node_t *target;
    priority_list_iterator_t it;

    printf("Generate deployments from SW2HW mapping\n");

    while ((opt = getopt(argc, argv, "h")) != -1)
    {
        switch (opt)
        {
            case 'h':
            default:
                fprintf(stderr, "Usage: %s transformed-map-file\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    if ((optind >= argc) || (argc < 2))
    {
        fprintf(stderr, "Usage: %s transformed-map-file\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    snprintf(mapFile, HWG_MAX_STRING_LENGTH, "%s", argv[optind]);

    /*Initialization*/
    bg_initialize();
    sw2hw_map_init(&mapping, NULL, NULL);
    sw2hw_map_from_yaml_file(mapFile, &mapping);

    if (!sw2hw_map_is_transformed(&mapping))
    {
        fprintf(stderr, "Mapping has not been transformed yet.\n");
        exit(EXIT_FAILURE);
    }

    for (entry = priority_list_first(mapping.assignments, &it);
            entry;
            entry = priority_list_next(&it))
    {
        target = hw_graph_get_node(mapping.hwGraph, entry->hwId);
        bg_graph_find_node(mapping.swGraph, entry->swId, &node);
        sub = ((subgraph_data_t *)node->_priv_data)->subgraph;
        snprintf(subName, bg_MAX_STRING_LENGTH, "%s", sub->name);
        sscanf(subName, "%[^.].", subgraphName);
        sprintf(dictName, "%s.dict", subgraphName);
        if (target->type == HW_NODE_TYPE_FPGA)
            sprintf(flavourArg, "VHDL");
        else
            sprintf(flavourArg, "C");
        if (!(pid = fork())) {
            execlp("./bg2dict", "bg2dict", "-n", subgraphName, "-l", flavourArg, subName, dictName, (char *)NULL);
        }
        if (target->type == HW_NODE_TYPE_FPGA)
        {
            sprintf(templateName, "templates/bg_graph_config_template.vhd");
            sprintf(srcName, "%s_config.vhd", subgraphName);
        } else {
            sprintf(templateName, "templates/bg_graph_header_template.h");
            sprintf(srcName, "%s.h", subgraphName);
        }
        waitpid(pid,NULL,0);
        if (!(pid = fork())) {
            execlp("./fill-template", "fill-template", templateName, dictName, srcName, (char *)NULL);
        }
        if (target->type == HW_NODE_TYPE_FPGA)
        {
            sprintf(templateName, "templates/bg_graph_template.vhd");
            sprintf(srcName, "%s.vhd", subgraphName);
        } else {
            sprintf(templateName, "templates/bg_graph_source_template.c");
            sprintf(srcName, "%s.c", subgraphName);
        }
        waitpid(pid,NULL,0);
        if (!fork()) {
            execlp("./fill-template", "fill-template", templateName, dictName, srcName, (char *)NULL);
        }
    }

    while (wait(NULL) != -1) ;

    /*Deinit*/
    sw2hw_map_destroy(&mapping);
    bg_terminate();
    exit(EXIT_SUCCESS);
}
