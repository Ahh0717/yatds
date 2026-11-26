#include "dijkstra.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

struct WEdge
{
    int To;
    int Weight;
};

struct WGraphRecord
{
    int VertexCount;
    bool Directed;
    std::vector<std::vector<WEdge>> Adjacent;
};

static void Die(const char *message)
{
    std::cerr << message << '\n';
    std::exit(EXIT_FAILURE);
}

static void ValidateVertex(WGraph graph, int vertex)
{
    if (vertex < 0 || vertex >= graph->VertexCount) {
        Die("Vertex index out of range");
    }
}

WGraph CreateDijkstraGraph(int vertex_count, int directed)
{
    if (vertex_count <= 0) {
        Die("Vertex count must be positive");
    }
    return new WGraphRecord{vertex_count, directed != 0, std::vector<std::vector<WEdge>>(vertex_count)};
}

void DisposeDijkstraGraph(WGraph graph)
{
    delete graph;
}

void AddDijkstraEdge(WGraph graph, int from, int to, int weight)
{
    if (graph == nullptr) {
        Die("Graph must not be NULL");
    }
    if (weight < 0) {
        Die("Dijkstra requires non-negative edge weights");
    }
    ValidateVertex(graph, from);
    ValidateVertex(graph, to);

    graph->Adjacent[from].push_back(WEdge{to, weight});
    if (!graph->Directed) {
        graph->Adjacent[to].push_back(WEdge{from, weight});
    }
}

void Dijkstra(WGraph graph, int src, int *dist, int *prev)
{
    if (graph == nullptr || dist == nullptr || prev == nullptr) {
        Die("Dijkstra received NULL argument");
    }
    ValidateVertex(graph, src);

    for (int i = 0; i < graph->VertexCount; ++i) {
        dist[i] = -1;
        prev[i] = -1;
    }
    dist[src] = 0;

    const int infinity = -1;
    std::vector<int> visited(graph->VertexCount, 0);
    std::vector<int> best(graph->VertexCount, infinity);
    best[src] = 0;

    for (int round = 0; round < graph->VertexCount; ++round) {
        int vertex = -1;
        for (int i = 0; i < graph->VertexCount; ++i) {
            if (!visited[i] && best[i] != infinity && (vertex == -1 || best[i] < best[vertex])) {
                vertex = i;
            }
        }
        if (vertex == -1) {
            break;
        }

        visited[vertex] = 1;
        for (const WEdge &edge : graph->Adjacent[vertex]) {
            int candidate = best[vertex] + edge.Weight;
            if (best[edge.To] == infinity || candidate < best[edge.To]) {
                best[edge.To] = candidate;
                prev[edge.To] = vertex;
            }
        }
    }

    for (int i = 0; i < graph->VertexCount; ++i) {
        if (best[i] != infinity) {
            dist[i] = best[i];
        }
    }
}
