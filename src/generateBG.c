#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "bg_generate_c.h"
#include "bg_generate_vhdl.h"

enum flavours
{
    FLAVOUR_C = 0,
    FLAVOUR_VHDL,
    FLAVOURS
};

const char flavourStr[FLAVOURS][10] =
{
    "C",
    "VHDL"
};

bg_error generate(FILE *out, bg_graph_t *graph, const char *name, const enum flavours flavour)
{
    bg_error err = bg_SUCCESS;
    bg_generator_c_t c_gen;
    bg_generator_vhdl_t vhdl_gen;

    switch (flavour)
    {
        case FLAVOUR_C:
            bg_generator_c_init(&c_gen, out, name);
            switch (bg_graph_generate_c(&c_gen, graph, 0))
            {
                case bg_ERR_WRONG_TYPE:
                    fprintf(stderr, "Error: Wrong type at node %u\n", c_gen.nodes);
                    return bg_ERR_WRONG_TYPE;
                case bg_ERR_NOT_IMPLEMENTED:
                    fprintf(stderr, "Error: Merge or node function at node %u not implemented\n", c_gen.nodes);
                    return bg_ERR_NOT_IMPLEMENTED;
                default:
                    break;
            }
            bg_generator_c_finalize(&c_gen);

            printf("--- SUMMARY ---\n");
            printf("Nodes: %u\n", c_gen.nodes);
            printf("Edges: %u\n", c_gen.edges);
            printf("Inputs: %u\n", c_gen.toplvl_inputs);
            printf("Outputs: %u\n", c_gen.toplvl_outputs);

            break;
        case FLAVOUR_VHDL:
            bg_generator_vhdl_init(&vhdl_gen, out, name);
            switch (bg_graph_generate_vhdl(&vhdl_gen, graph, 0))
            {
                case bg_ERR_WRONG_TYPE:
                    fprintf(stderr, "Error: Wrong type at node %u\n", vhdl_gen.nodes);
                    return bg_ERR_WRONG_TYPE;
                case bg_ERR_NOT_IMPLEMENTED:
                    fprintf(stderr, "Error: Merge or node function at node %u not implemented\n", vhdl_gen.nodes);
                    return bg_ERR_NOT_IMPLEMENTED;
                default:
                    break;
            }
            bg_generator_vhdl_finalize(&vhdl_gen);

            printf("--- SUMMARY ---\n");
            printf("Nodes: %u\n", vhdl_gen.nodes);
            printf("Edges: %u\n", vhdl_gen.edges);
            printf("Inputs: %u\n", vhdl_gen.toplvl_inputs);
            printf("Outputs: %u\n", vhdl_gen.toplvl_outputs);
            printf("Sources: %u\n", vhdl_gen.sources);
            printf("Sinks: %u\n", vhdl_gen.sinks);
            printf("Copies: %u\n", vhdl_gen.copies);
            printf("Merges: %u\n", vhdl_gen.merges);
            printf("Unaries: %u\n", vhdl_gen.unaryNodes);
            printf("Binaries: %u\n", vhdl_gen.binaryNodes);
            printf("Ternaries: %u\n", vhdl_gen.ternaryNodes);

            break;
        default:
            fprintf(stderr, "Generator not implemented yet\n");
            return bg_ERR_NOT_IMPLEMENTED;
    }


    return err;
}

int main (int argc, char *argv[])
{
    int opt;
    FILE *out;
    char yamlfile[256];
    char name[256];
    bg_graph_t *g = NULL;
    enum flavours flavour = FLAVOUR_C;
    unsigned int i;

    printf("Behavior Graph to Dictionary Generator\n");

    name[0] = '\0';
    while ((opt = getopt(argc, argv, "hn:l:")) != -1)
    {
        switch (opt)
        {
            case 'l':
                for (i = 0; i < FLAVOURS; ++i)
                {
                    if (strcmp(optarg, flavourStr[i]) == 0)
                    {
                        flavour = i;
                        break;
                    }
                }
                if (i == FLAVOURS)
                {
                    fprintf(stderr, "Unknown flavour %s. Use C or VHDL\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'n':
                snprintf(name, 256, "%s", optarg);
                break;
            case 'h':
            default:
                fprintf(stderr, "Usage: %s [-n name] [-l flavour] yaml-file dictionary-file\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    if ((optind >= argc) || (argc < 3))
    {
        fprintf(stderr, "Usage: %s [-n name] [-l flavour] yaml-file dictionary-file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Using flavour: \"%s\"\n", flavourStr[flavour]);

    snprintf(yamlfile, 256, "%s", argv[optind]);
    out = fopen(argv[optind+1], "a");
    if (!out)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    bg_initialize();
    bg_graph_alloc(&g, name);
    bg_graph_from_yaml_file(yamlfile, g);
    if ((strlen(g->name) > 0) && (strlen(name) < 1))
        strncpy(name, g->name, bg_MAX_STRING_LENGTH);
    if (strlen(name) < 1)
        sprintf(name, "noname");
    printf("Using graph name: \"%s\"\n", name);
    generate(out, g, name, flavour);
    bg_graph_free(g);
    bg_terminate();

    fclose(out);
    exit(EXIT_SUCCESS);
}
