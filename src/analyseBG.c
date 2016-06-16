#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "bagel.h"
#include "node_types/bg_node_subgraph.h"
#include "node_list.h"

void printUsage(const char *name)
{
    fprintf(stderr, "Usage: %s behavior-graph\n", name);
}

struct statistics {
    /*NODE STUFF*/
    unsigned int inputs;
    unsigned int outputs;
    unsigned int pipes;
    unsigned int dividers;
    unsigned int sqrts;
    unsigned int absolutes;
    unsigned int cosinus;
    unsigned int sinus;
    unsigned int acosinus;
    unsigned int asinus;
    unsigned int tangens;
    unsigned int atangens;
    unsigned int bin_atan;
    unsigned int exponential;
    unsigned int logarithm;
    unsigned int modulo;
    unsigned int power;
    unsigned int greaterthan;
    unsigned int equalto;
    unsigned int subgraphs;
    unsigned int tanhyperbolicus;
    unsigned int sigmoids;
    unsigned int externals;
    unsigned int nodes;
    /*NESTING*/
    unsigned int maxLevel;
    /*MERGES*/
    unsigned int sums;
    unsigned int products;
    unsigned int weightedsums;
    unsigned int minima;
    unsigned int maxima;
    unsigned int norms;
    unsigned int means;
    unsigned int medians;
    unsigned int merges;
    /*EDGES*/
    unsigned int edges;
    unsigned int real_edges;
};

void printStats(const struct statistics *stats)
{
    printf("Node \t Occurence\n");
    printf("----\n");
    printf("%-30s %4u\n", "Inputs:", stats->inputs);
    printf("%-30s %4u\n", "Outputs:", stats->outputs);
    printf("%-30s %4u\n", "Nodes:", stats->nodes);
    printf("%-30s %4u\n", " Pipes:", stats->pipes);
    printf("%-30s %4u\n", " Divides:", stats->dividers);
    printf("%-30s %4u\n", " Square roots:", stats->sqrts);
    printf("%-30s %4u\n", " Absolutes:", stats->absolutes);
    printf("%-30s %4u\n", " Cosines:", stats->cosinus);
    printf("%-30s %4u\n", " Sines:", stats->sinus);
    printf("%-30s %4u\n", " Tangens:", stats->tangens);
    printf("%-30s %4u\n", " Tangens hyperb.:", stats->tanhyperbolicus);
    printf("%-30s %4u\n", " Arcus cosines:", stats->acosinus);
    printf("%-30s %4u\n", " Arcus sines:", stats->asinus);
    printf("%-30s %4u\n", " Arcus tangens:", stats->atangens);
    printf("%-30s %4u\n", " Binary arcus tangens:", stats->bin_atan);
    printf("%-30s %4u\n", " Exponentials:", stats->exponential);
    printf("%-30s %4u\n", " Sigmoids:", stats->sigmoids);
    printf("%-30s %4u\n", " Logarithms:", stats->logarithm);
    printf("%-30s %4u\n", " Powers:", stats->power);
    printf("%-30s %4u\n", " Moduli:", stats->modulo);
    printf("%-30s %4u\n", " >0:", stats->greaterthan);
    printf("%-30s %4u\n", " ==0:", stats->equalto);
    printf("%-30s %4u\n", " User defined:", stats->externals);
    printf("%-30s %4u\n", " Subgraphs:", stats->subgraphs);
    printf("Merge \t Occurence\n");
    printf("----\n");
    printf("%-30s %4u\n", "Merges:", stats->merges);
    printf("%-30s %4u\n", " Sums:", stats->sums);
    printf("%-30s %4u\n", " Weighted sums:", stats->weightedsums);
    printf("%-30s %4u\n", " Products:", stats->products);
    printf("%-30s %4u\n", " Minima:", stats->minima);
    printf("%-30s %4u\n", " Maxima:", stats->maxima);
    printf("%-30s %4u\n", " Norms:", stats->norms);
    printf("%-30s %4u\n", " Means:", stats->means);
    printf("%-30s %4u\n", " Medians:", stats->medians);
    printf("Edge \t Occurence\n");
    printf("----\n");
    printf("%-30s %4u\n", "Edges:", stats->edges);
    printf("%-30s %4u\n", " Full:", stats->real_edges);
    printf("Name \t Value\n");
    printf("----\n");
    printf("%-30s %4u\n", "Max. nesting level:", stats->maxLevel);
}

void evaluateRecursively(struct statistics *stats, bg_graph_t *g, const unsigned int level)
{
    bg_node_t *current_node;
    bg_node_list_iterator_t it;
    unsigned int i,j,k;

    /*Evaluate level*/
    stats->maxLevel = (level > stats->maxLevel) ? level : stats->maxLevel;

    /*Count inputs*/
    for (current_node = bg_node_list_first(g->input_nodes, &it);
            current_node;
            current_node = bg_node_list_next(&it))
    {
        stats->inputs++;
    }

    /*Count hidden nodes and their type*/
    for (current_node = bg_node_list_first(g->hidden_nodes, &it);
            current_node;
            current_node = bg_node_list_next(&it))
    {
        /*Evaluate merges TODO: Put this in a function!*/
        for (i = 0; (i < current_node->input_port_cnt); ++i)
        {
            /*Check how many edges flow into the merge*/
            j = current_node->input_ports[i]->num_edges;
            stats->edges += j;
            for (k = 0; k < j; ++k)
            {
                if (fabsf(current_node->input_ports[i]->edges[k]->weight) != 1.0f)
                    stats->real_edges++;
            }

            /*Do not count unconnected inputs*/
            if (j < 1)
                continue;

            stats->merges++;
            switch (current_node->input_ports[i]->merge->id)
            {
                case bg_MERGE_TYPE_SUM:
                    stats->sums++;
                    break;
                case bg_MERGE_TYPE_PRODUCT:
                    stats->products++;
                    break;
                case bg_MERGE_TYPE_MAX:
                    stats->maxima++;
                    break;
                case bg_MERGE_TYPE_MIN:
                    stats->minima++;
                    break;
                case bg_MERGE_TYPE_NORM:
                    stats->norms++;
                    break;
                case bg_MERGE_TYPE_MEAN:
                    stats->means++;
                    break;
                case bg_MERGE_TYPE_WEIGHTED_SUM:
                    stats->weightedsums++;
                    break;
                case bg_MERGE_TYPE_MEDIAN:
                    stats->medians++;
                    break;
                default:
                    fprintf(stderr, "Unknown merge on input %u at node %u\n", i, (unsigned int)current_node->id);
                    exit(EXIT_FAILURE);
            }
        }

        stats->nodes++;
        switch(current_node->type->id)
        {
            case bg_NODE_TYPE_INPUT:
                fprintf(stderr, "FATAL: Found input in hidden nodes\n");
                exit(EXIT_FAILURE);
            case bg_NODE_TYPE_OUTPUT:
                fprintf(stderr, "FATAL: Found output in hidden nodes\n");
                exit(EXIT_FAILURE);
            case bg_NODE_TYPE_PIPE:
                stats->pipes++;
                break;
            case bg_NODE_TYPE_DIVIDE:
                stats->dividers++;
                break;
            case bg_NODE_TYPE_SQRT:
                stats->sqrts++;
                break;
            case bg_NODE_TYPE_ABS:
                stats->absolutes++;
                break;
            case bg_NODE_TYPE_COS:
                stats->cosinus++;
                break;
            case bg_NODE_TYPE_ACOS:
                stats->acosinus++;
                break;
            case bg_NODE_TYPE_SIN:
                stats->sinus++;
                break;
            case bg_NODE_TYPE_ASIN:
                stats->asinus++;
                break;
            case bg_NODE_TYPE_TAN:
                stats->tangens++;
                break;
            case bg_NODE_TYPE_ATAN:
                stats->atangens++;
                break;
            case bg_NODE_TYPE_EXP:
                stats->exponential++;
                break;
            case bg_NODE_TYPE_LOG:
                stats->logarithm++;
                break;
            case bg_NODE_TYPE_MOD:
                stats->modulo++;
                break;
            case bg_NODE_TYPE_ATAN2:
                stats->bin_atan++;
                break;
            case bg_NODE_TYPE_POW:
                stats->power++;
                break;
            case bg_NODE_TYPE_GREATER_THAN_0:
                stats->greaterthan++;
                break;
            case bg_NODE_TYPE_EQUAL_TO_0:
                stats->equalto++;
                break;
            case bg_NODE_TYPE_SUBGRAPH:
                stats->subgraphs++;
                /*Call recursively*/
                evaluateRecursively(stats, ((subgraph_data_t*)current_node->_priv_data)->subgraph, level+1);
                break;
            case bg_NODE_TYPE_TANH:
                stats->tanhyperbolicus++;
                break;
            case bg_NODE_TYPE_FSIGMOID:
                stats->sigmoids++;
                break;
            case bg_NODE_TYPE_EXTERN:
                stats->externals++;
                break;
            default:
                fprintf(stderr, "Unknown node %u\n", (unsigned int)current_node->id);
                exit(EXIT_FAILURE);
        }
    }

    /*Count outputs*/
    for (current_node = bg_node_list_first(g->output_nodes, &it);
            current_node;
            current_node = bg_node_list_next(&it))
    {
        /*Evaluate merges TODO: Put this in a function!*/
        for (i = 0; (i < current_node->input_port_cnt); ++i)
        {
            /*Check how many edges flow into the merge*/
            j = current_node->input_ports[i]->num_edges;
            stats->edges += j;
            for (k = 0; k < j; ++k)
            {
                if (fabsf(current_node->input_ports[i]->edges[k]->weight) != 1.0f)
                    stats->real_edges++;
            }

            /*Do not count unconnected inputs*/
            if (j < 1)
                continue;

            stats->merges++;
            switch (current_node->input_ports[i]->merge->id)
            {
                case bg_MERGE_TYPE_SUM:
                    stats->sums++;
                    break;
                case bg_MERGE_TYPE_PRODUCT:
                    stats->products++;
                    break;
                case bg_MERGE_TYPE_MAX:
                    stats->maxima++;
                    break;
                case bg_MERGE_TYPE_MIN:
                    stats->minima++;
                    break;
                case bg_MERGE_TYPE_NORM:
                    stats->norms++;
                    break;
                case bg_MERGE_TYPE_MEAN:
                    stats->means++;
                    break;
                case bg_MERGE_TYPE_WEIGHTED_SUM:
                    stats->weightedsums++;
                    break;
                case bg_MERGE_TYPE_MEDIAN:
                    stats->medians++;
                    break;
                default:
                    fprintf(stderr, "Unknown merge on input %u at node %u\n", i, (unsigned int)current_node->id);
                    exit(EXIT_FAILURE);
            }
        }

        stats->outputs++;
    }
}

int main (int argc, char *argv[])
{
    int opt;
    char yamlfile[256];
    bg_graph_t *g = NULL;
    struct statistics stats;


    printf("Behavior Graph Analysis\n");

    while ((opt = getopt(argc, argv, "h")) != -1)
    {
        switch (opt)
        {
            case 'h':
                printUsage(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            default:
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    if ((optind >= argc) || (argc < 2))
    {
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    snprintf(yamlfile, 256, "%s", argv[optind]);

    bg_initialize();
    bg_graph_alloc(&g, "");
    bg_graph_from_yaml_file(yamlfile, g);

    memset(&stats, 0, sizeof(struct statistics));
    evaluateRecursively(&stats, g, 0);
    printStats(&stats);

    bg_graph_free(g);
    bg_terminate();

    exit(EXIT_SUCCESS);
}
