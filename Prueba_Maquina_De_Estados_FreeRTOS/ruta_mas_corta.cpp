/**
Dijkstra algorithm for the movement.
@author Vadim Coselev
*/

#include <vector>

#include "ruta_mas_corta.h"

std::vector<int16_t> dijkstra_vector_function(uint16_t nodo_inicial, int16_t angulo_inicial, uint16_t nodo_final, int16_t MATRIZ_ADYACENTE[MAX][MAX], int16_t MATRIZ_GIRO[MAX][MAX])
{
int16_t i,j;
uint16_t n = MAX;
std::vector<int16_t> shortest_path_node;
std::vector<int16_t> movimientos;
std::vector<int16_t>::iterator it;

shortest_path_node = dijkstra(MATRIZ_ADYACENTE,n,nodo_inicial,nodo_final);
uint8_t size = shortest_path_node.size();
std::vector<int16_t> giros;
int16_t theta_n = angulo_inicial;
movimientos.push_back(shortest_path_node[0]);

for(i = 0;i<=size-2;i++){
	int16_t theta_n_n_1 = MATRIZ_GIRO[shortest_path_node[i]][shortest_path_node[i+1]];
	int16_t var_theta = theta_n_n_1-theta_n;
	if(var_theta == 270){
		var_theta = -90;
	}
	else if(var_theta == -270){
		var_theta = 90;
	}
	movimientos.push_back(var_theta);
	movimientos.push_back(shortest_path_node[i+1]);
	theta_n = theta_n_n_1;
}
return movimientos;
}




std::vector<int16_t> dijkstra(int16_t G[MAX][MAX],uint16_t n,uint16_t startnode,uint16_t nodo_final)
{
int16_t cost[MAX][MAX],distance[MAX],pred[MAX];
int16_t visited[MAX],count,mindistance,nextnode,i,j;

std::vector<int16_t> shortest_path_node;
shortest_path_node.clear();

std::vector<int16_t>::iterator it;

//pred[] stores the predecessor of each node
//count gives the number of nodes seen so far
//create the cost matrix
for(i=0;i<n;i++)
	for(j=0;j<n;j++)
		if(G[i][j]==0)
			cost[i][j]=INFINITY;
		else
			cost[i][j]=G[i][j];
//initialize pred[],distance[] and visited[]
for(i=0;i<n;i++)
{
	distance[i]=cost[startnode][i];
	pred[i]=startnode;
	visited[i]=0;
}
distance[startnode]=0;
visited[startnode]=1;
count=1;
while(count<n-1)
{
	mindistance=INFINITY;
	//nextnode gives the node at minimum distance
	for(i=0;i<n;i++)
		if(distance[i]<mindistance&&!visited[i])
		{
			mindistance=distance[i];
			nextnode=i;
		}
	//check if a better path exists through nextnode
	visited[nextnode]=1;
	for(i=0;i<n;i++)
	if(!visited[i])
	if(mindistance+cost[nextnode][i]<distance[i])
	{
	distance[i]=mindistance+cost[nextnode][i];
	pred[i]=nextnode;
	}
	count++;
}

//print the path and distance of each node
	if(i!=startnode){
		
			j=nodo_final;
			
			it = shortest_path_node.begin();
			shortest_path_node.insert(it,j);
			do
		{
			j=pred[j];
			it = shortest_path_node.begin();
			shortest_path_node.insert(it,j);
		}while(j!=startnode);
    //shortest_path_node.insert(shortest_path_node.begin(),distance[nodo_final]-1);
	}
return shortest_path_node;
}
