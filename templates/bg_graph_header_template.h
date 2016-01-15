/*
 * *** DO NOT MODIFY ***
 */
#ifndef _<name>_GRAPH_H
#define _<name>_GRAPH_H

#define <name>_GRAPH_NO_EDGES <edges>
#define <name>_GRAPH_NO_INPUTS <toplvlInputs>
#define <name>_GRAPH_NO_OUTPUTS <toplvlOutputs>

/*CALL THIS TO EVALUATE THE BEHAVIOR GRAPH*/
int <name>_graph_evaluate (const <type> in[<name>_GRAPH_NO_INPUTS], <type> out[<name>_GRAPH_NO_OUTPUTS]);

#endif
