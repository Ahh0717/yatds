#include "graph.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

struct AdjMatrixGraphRecord
{
    int VertexCount;
    bool Directed;
    std::vector<int> Matrix;
};

#define CELL(g, from, to) ((g)->Matrix[(from) * (g)->VertexCount + (to)])

static void Die(const char *message)
{
    std::cerr << message << '\n';
    std::exit(EXIT_FAILURE);
}

static void ValidateVertex(AdjMatrixGraph graph, int vertex)
{
    if (vertex < 0 || vertex >= graph->VertexCount) {
        Die("Vertex index out of range");
    }
}

AdjMatrixGraph CreateAdjMatrixGraph(int vertex_count, int directed)
{
    if (vertex_count <= 0) {
        Die("Vertex count must be positive");
    }

    return new AdjMatrixGraphRecord{
        vertex_count,
        directed != 0,
        std::vector<int>(vertex_count * vertex_count, 0),
    };
}

void DisposeAdjMatrixGraph(AdjMatrixGraph graph)
{
    delete graph;
}

void AddAdjMatrixEdge(AdjMatrixGraph graph, int from, int to)
{
    if (graph == nullptr) {
        Die("Graph must not be NULL");
    }
    ValidateVertex(graph, from);
    ValidateVertex(graph, to);

    CELL(graph, from, to) = 1;
    if (!graph->Directed) {
        CELL(graph, to, from) = 1;
    }
}

void PrintAdjMatrixGraph(AdjMatrixGraph graph)
{
    if (graph == nullptr) {
        Die("Graph must not be NULL");
    }

    for (int i = 0; i < graph->VertexCount; ++i) {
        std::cout << i << ':';
        for (int j = 0; j < graph->VertexCount; ++j) {
            if (CELL(graph, i, j)) {
                std::cout << ' ' << j;
            }
        }
        std::cout << '\n';
    }
}

void AdjMatrixBFS(AdjMatrixGraph graph, int start, int *order, int *count)
{
    if (graph == nullptr || order == nullptr || count == nullptr) {
        Die("AdjMatrixBFS received NULL argument");
    }
    ValidateVertex(graph, start);

    std::vector<int> visited(graph->VertexCount, 0);
    std::vector<int> queue(graph->VertexCount, 0);
    int front = 0;
    int back = 0;

    *count = 0;
    visited[start] = 1;
    queue[back++] = start;

    while (front < back) {
        int vertex = queue[front++];
        order[*count] = vertex;
        ++(*count);

        for (int j = 0; j < graph->VertexCount; ++j) {
            if (CELL(graph, vertex, j) && !visited[j]) {
                visited[j] = 1;
                queue[back++] = j;
            }
        }
    }
}

static void DFSRecursive(AdjMatrixGraph graph, int vertex, std::vector<int> &visited, int *order, int *count)
{
    visited[vertex] = 1;
    order[*count] = vertex;
    ++(*count);

    for (int j = 0; j < graph->VertexCount; ++j) {
        if (CELL(graph, vertex, j) && !visited[j]) {
            DFSRecursive(graph, j, visited, order, count);
        }
    }
}

void AdjMatrixDFS(AdjMatrixGraph graph, int start, int *order, int *count)
{
    if (graph == nullptr || order == nullptr || count == nullptr) {
        Die("AdjMatrixDFS received NULL argument");
    }
    ValidateVertex(graph, start);

    std::vector<int> visited(graph->VertexCount, 0);
    *count = 0;
    DFSRecursive(graph, start, visited, order, count);
}

int AdjMatrixTopologicalSort(AdjMatrixGraph graph, int *order, int *count)
{
    if (graph == nullptr || order == nullptr || count == nullptr) {
        Die("AdjMatrixTopologicalSort received NULL argument");
    }

    if (!graph->Directed) {
        *count = 0;
        return 0;
    }

    *count = 0;

    std::vector<int> indegree(graph->VertexCount, 0);
    for (int i = 0; i < graph->VertexCount; ++i) {
        for (int j = 0; j < graph->VertexCount; ++j) {
            if (CELL(graph, i, j)) {
                ++indegree[j];
            }
        }
    }

    std::vector<int> queue(graph->VertexCount, 0);
    int front = 0;
    int back = 0;
    for (int i = 0; i < graph->VertexCount; ++i) {
        if (indegree[i] == 0) {
            queue[back++] = i;
        }
    }

    while (front < back) {
        int vertex = queue[front++];
        order[*count] = vertex;
        ++(*count);

        for (int j = 0; j < graph->VertexCount; ++j) {
            if (CELL(graph, vertex, j) && --indegree[j] == 0) {
                queue[back++] = j;
            }
        }
    }

    return *count == graph->VertexCount ? 1 : 0;
}
