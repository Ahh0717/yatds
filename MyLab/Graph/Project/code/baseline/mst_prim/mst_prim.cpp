#include "mst_prim.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

struct WEdge
{
    int To;
    int Weight;
};

struct PrimGraphRecord
{
    int VertexCount;
    std::vector<std::vector<WEdge>> Adjacent;
};

static void Die(const char *message)
{
    std::cerr << message << '\n';
    std::exit(EXIT_FAILURE);
}

static void ValidateVertex(PrimGraph graph, int vertex)
{
    if (vertex < 0 || vertex >= graph->VertexCount) {
        Die("Vertex index out of range");
    }
}

PrimGraph CreatePrimGraph(int vertex_count)
{
    if (vertex_count <= 0) {
        Die("Vertex count must be positive");
    }
    return new PrimGraphRecord{vertex_count, std::vector<std::vector<WEdge>>(vertex_count)};
}

void DisposePrimGraph(PrimGraph graph)
{
    delete graph;
}

void AddPrimEdge(PrimGraph graph, int from, int to, int weight)
{
    if (graph == nullptr) {
        Die("Graph must not be NULL");
    }
    ValidateVertex(graph, from);
    ValidateVertex(graph, to);

    graph->Adjacent[from].push_back(WEdge{to, weight});
    graph->Adjacent[to].push_back(WEdge{from, weight});
}

int Prim(PrimGraph graph, Edge *mst_edges, int *total_cost)
{
    if (graph == nullptr || mst_edges == nullptr || total_cost == nullptr) {
        return 0;
    }

    *total_cost = 0;

    const int infinity = -1;
    std::vector<int> selected(graph->VertexCount, 0);
    std::vector<int> best_weight(graph->VertexCount, infinity);
    std::vector<int> parent(graph->VertexCount, -1);
    best_weight[0] = 0;

    int edge_count = 0;

    for (int round = 0; round < graph->VertexCount; ++round) {
        int vertex = -1;
        for (int i = 0; i < graph->VertexCount; ++i) {
            if (!selected[i] && best_weight[i] != infinity &&
                (vertex == -1 || best_weight[i] < best_weight[vertex])) {
                vertex = i;
            }
        }

        if (vertex == -1) {
            return 0;
        }

        selected[vertex] = 1;

        if (parent[vertex] != -1) {
            mst_edges[edge_count] = Edge{parent[vertex], vertex, best_weight[vertex]};
            *total_cost += best_weight[vertex];
            ++edge_count;
        }

        for (const WEdge &edge : graph->Adjacent[vertex]) {
            if (!selected[edge.To] &&
                (best_weight[edge.To] == infinity || edge.Weight < best_weight[edge.To])) {
                best_weight[edge.To] = edge.Weight;
                parent[edge.To] = vertex;
            }
        }
    }

    return edge_count == graph->VertexCount - 1 ? 1 : 0;
}
