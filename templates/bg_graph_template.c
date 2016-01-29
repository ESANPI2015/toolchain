/*
 * *** DO NOT MODIFY ***
 */

#include "<name>_graph.h"
#include <math.h>

/*
 * CONST REGION:
 * contains edge weights
 * NOTE: This will be in ROM!
 */
const <type> edgeWeight[<name>_GRAPH_NO_EDGES] =
{
    /*
     * FORMAT:
     * value of type <type>,
     */
<weight0>
};

/*
 * FUNCTION REGION
 */
void <name>_graph_init(<name>_graph_t *graph)
{
    unsigned i;
    for (i = 0; i < <name>_GRAPH_NO_EDGES; ++i)
        graph->edgeValue[i] = 0.0f;
}

int <name>_graph_evaluate (<name>_graph_t *graph, const <type> in[<name>_GRAPH_NO_INPUTS], <type> out[<name>_GRAPH_NO_OUTPUTS])
{
    <type> result;
    <type> merge[3];
    <type> *edgeValue = graph->edgeValue;

<code0>

    return 0;
}
