#include "shortest_path.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

struct GraphRecord
{
    int VertexCount;
    bool Directed;
    std::vector<std::vector<int>> Adjacent;
};

static void Die(const char *message)
{
    std::cerr << message << '\n';
    std::exit(EXIT_FAILURE);
}

static void ValidateVertex(Graph graph, int vertex)
{
    if (vertex < 0 || vertex >= graph->VertexCount) {
        Die("Vertex index out of range");
    }
}

Graph CreateShortestPathGraph(int vertex_count, int directed)
{
    if (vertex_count <= 0) {
        Die("Vertex count must be positive");
    }
    return new GraphRecord{vertex_count, directed != 0, std::vector<std::vector<int>>(vertex_count)};
}

void DisposeShortestPathGraph(Graph graph)
{
    delete graph;
}

void AddShortestPathEdge(Graph graph, int from, int to)
{
    if (graph == nullptr) {
        Die("Graph must not be NULL");
    }
    ValidateVertex(graph, from);
    ValidateVertex(graph, to);

    graph->Adjacent[from].push_back(to);
    if (!graph->Directed) {
        graph->Adjacent[to].push_back(from);
    }
}

void ShortestPath(Graph graph, int src, int *dist, int *prev)
{
    if (graph == nullptr || dist == nullptr || prev == nullptr) {
        Die("ShortestPath received NULL argument");
    }
    ValidateVertex(graph, src);

    for (int i = 0; i < graph->VertexCount; ++i) {
        dist[i] = -1;
        prev[i] = -1;
    }

    dist[src] = 0;

    std::vector<int> queue(graph->VertexCount, 0);
    int front = 0;
    int back = 0;
    queue[back++] = src;

    while (front < back) {
        int vertex = queue[front++];
        for (int next : graph->Adjacent[vertex]) {
            if (dist[next] == -1) {
                dist[next] = dist[vertex] + 1;
                prev[next] = vertex;
                queue[back++] = next;
            }
        }
    }
}
