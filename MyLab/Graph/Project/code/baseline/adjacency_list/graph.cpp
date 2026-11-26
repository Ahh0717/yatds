#include "graph.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

struct AdjNode
{
    int Vertex;
    AdjNode *Next;
};

struct AdjListGraphRecord
{
    int VertexCount;
    bool Directed;
    std::vector<AdjNode *> Heads;
    std::vector<AdjNode *> Tails;
};

static void Die(const char *message)
{
    std::cerr << message << '\n';
    std::exit(EXIT_FAILURE);
}

static void ValidateVertex(AdjListGraph graph, int vertex)
{
    if (vertex < 0 || vertex >= graph->VertexCount) {
        Die("Vertex index out of range");
    }
}

static void AddDirectedEdge(AdjListGraph graph, int from, int to)
{
    AdjNode *node = new AdjNode{to, nullptr};

    if (graph->Heads[from] == nullptr) {
        graph->Heads[from] = node;
    } else {
        graph->Tails[from]->Next = node;
    }
    graph->Tails[from] = node;
}

static void DFSRecursive(AdjListGraph graph, int vertex, std::vector<int> &visited, int *order, int *count)
{
    visited[vertex] = 1;
    order[*count] = vertex;
    ++(*count);

    for (AdjNode *node = graph->Heads[vertex]; node != nullptr; node = node->Next) {
        if (!visited[node->Vertex]) {
            DFSRecursive(graph, node->Vertex, visited, order, count);
        }
    }
}

AdjListGraph CreateAdjListGraph(int vertex_count, int directed)
{
    if (vertex_count <= 0) {
        Die("Vertex count must be positive");
    }

    return new AdjListGraphRecord{
        vertex_count,
        directed != 0,
        std::vector<AdjNode *>(vertex_count, nullptr),
        std::vector<AdjNode *>(vertex_count, nullptr),
    };
}

void DisposeAdjListGraph(AdjListGraph graph)
{
    if (graph == nullptr) {
        return;
    }

    for (int i = 0; i < graph->VertexCount; ++i) {
        AdjNode *current = graph->Heads[i];
        while (current != nullptr) {
            AdjNode *next = current->Next;
            delete current;
            current = next;
        }
    }

    delete graph;
}

void AddAdjListEdge(AdjListGraph graph, int from, int to)
{
    if (graph == nullptr) {
        Die("Graph must not be NULL");
    }

    ValidateVertex(graph, from);
    ValidateVertex(graph, to);

    AddDirectedEdge(graph, from, to);
    if (!graph->Directed) {
        AddDirectedEdge(graph, to, from);
    }
}

void PrintAdjListGraph(AdjListGraph graph)
{
    if (graph == nullptr) {
        Die("Graph must not be NULL");
    }

    for (int i = 0; i < graph->VertexCount; ++i) {
        std::cout << i << ':';
        AdjNode *current = graph->Heads[i];
        while (current != nullptr) {
            std::cout << ' ' << current->Vertex;
            current = current->Next;
        }
        std::cout << '\n';
    }
}

void AdjListBFS(AdjListGraph graph, int start, int *order, int *count)
{
    if (graph == nullptr || order == nullptr || count == nullptr) {
        Die("AdjListBFS received NULL argument");
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

        for (AdjNode *node = graph->Heads[vertex]; node != nullptr; node = node->Next) {
            if (!visited[node->Vertex]) {
                visited[node->Vertex] = 1;
                queue[back++] = node->Vertex;
            }
        }
    }
}

void AdjListDFS(AdjListGraph graph, int start, int *order, int *count)
{
    if (graph == nullptr || order == nullptr || count == nullptr) {
        Die("AdjListDFS received NULL argument");
    }

    ValidateVertex(graph, start);

    std::vector<int> visited(graph->VertexCount, 0);
    *count = 0;
    DFSRecursive(graph, start, visited, order, count);
}

int AdjListTopologicalSort(AdjListGraph graph, int *order, int *count)
{
    if (graph == nullptr || order == nullptr || count == nullptr) {
        Die("AdjListTopologicalSort received NULL argument");
    }

    if (!graph->Directed) {
        *count = 0;
        return 0;
    }

    *count = 0;

    std::vector<int> indegree(graph->VertexCount, 0);
    for (int i = 0; i < graph->VertexCount; ++i) {
        for (AdjNode *node = graph->Heads[i]; node != nullptr; node = node->Next) {
            ++indegree[node->Vertex];
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

        for (AdjNode *node = graph->Heads[vertex]; node != nullptr; node = node->Next) {
            if (--indegree[node->Vertex] == 0) {
                queue[back++] = node->Vertex;
            }
        }
    }

    return *count == graph->VertexCount ? 1 : 0;
}
