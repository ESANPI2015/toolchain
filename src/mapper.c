#include "sw_to_hw_map.h"
#include "sw_to_hw_map_yaml.h"
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
    char outfileName[SW2HW_MAX_STRING_LENGTH];
    sw2hw_map_t mapping;

    printf("SW to HW Mapper\n");

    while ((opt = getopt(argc, argv, "h")) != -1)
    {
        switch (opt)
        {
            case 'h':
            default:
                fprintf(stderr, "Usage: %s hw_spec sw_spec outfile\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    if ((optind >= argc) || (argc < 4))
    {
        fprintf(stderr, "Usage: %s hw_spec sw_spec outfile\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    snprintf(hwYamlFile, HWG_MAX_STRING_LENGTH, "%s", argv[optind]);
    snprintf(swYamlFile, bg_MAX_STRING_LENGTH, "%s", argv[optind+1]);
    snprintf(outfileName, SW2HW_MAX_STRING_LENGTH, "%s", argv[optind+2]);

    bg_initialize();

    sw2hw_map_init(&mapping, hwYamlFile, swYamlFile);
    sw2hw_map_match(&mapping);
    sw2hw_map_initial(&mapping, defaultCost);
    do {
        while (sw2hw_map_refine(&mapping, defaultCost) == SW2HW_ERR_NONE);
    } while (sw2hw_map_regroup(&mapping, defaultCost) == SW2HW_ERR_NONE);
    sw2hw_map_to_yaml_file(outfileName, &mapping);

    sw2hw_map_destroy(&mapping);
    bg_terminate();
    exit(EXIT_SUCCESS);
}
