/*
 * *** DO NOT MODIFY ***
 */
#ifndef _<name>_GRAPH_H
#define _<name>_GRAPH_H

#define <name>_GRAPH_NO_EDGES <edges>
#define <name>_GRAPH_NO_INPUTS <toplvlInputs>
#define <name>_GRAPH_NO_OUTPUTS <toplvlOutputs>

typedef struct {
    <type> edgeValue[<name>_GRAPH_NO_EDGES];
} <name>_graph_t;

void <name>_graph_init(<name>_graph_t *graph);
int <name>_graph_evaluate (<name>_graph_t *graph, const <type> in[<name>_GRAPH_NO_INPUTS], <type> out[<name>_GRAPH_NO_OUTPUTS]);

#endif
