/**
Dijkstra algorithm for the movement.
@author Vadim Coselev
*/

#ifndef MATRICES_DIJKSTRA_DEFINE
#define MATRICES_DIJKSTRA_DEFINE

#include <stdint.h>
#include <vector>
#define INFINITY 9999
#define MAX 22

std::vector<int16_t> dijkstra(int16_t G[MAX][MAX],uint16_t n,uint16_t startnode,uint16_t nodo_final);
std::vector<int16_t> dijkstra_vector_function(uint16_t nodo_inicial, int16_t angulo_inicial, uint16_t nodo_final, int16_t MATRIZ_ADYACENTE[MAX][MAX], int16_t MATRIZ_GIRO[MAX][MAX]);

#endif