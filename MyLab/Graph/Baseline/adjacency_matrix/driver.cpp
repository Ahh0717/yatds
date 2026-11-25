#include "graph.hpp"

#include <fstream>
#include <iostream>
#include <vector>

static void PrintOrder(const char *label, const int *order, int count)
{
    std::cout << label;
    for (int i = 0; i < count; ++i) {
        std::cout << ' ' << order[i];
    }
    std::cout << '\n';
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <test.in>\n";
        return 1;
    }

    std::ifstream input(argv[1]);
    if (!input) {
        std::cerr << "cannot open input file: " << argv[1] << '\n';
        return 1;
    }

    int case_count = 0;
    if (!(input >> case_count)) {
        return 1;
    }

    for (int case_index = 1; case_index <= case_count; ++case_index) {
        int vertex_count = 0;
        int edge_count = 0;
        int directed = 0;
        int start = 0;
        input >> vertex_count >> edge_count >> directed >> start;

        AdjMatrixGraph graph = CreateAdjMatrixGraph(vertex_count, directed);

        for (int i = 0; i < edge_count; ++i) {
            int from = 0;
            int to = 0;
            input >> from >> to;
            AddAdjMatrixEdge(graph, from, to);
        }

        std::vector<int> order(vertex_count, 0);
        int count = 0;

        std::cout << "CASE " << case_index << '\n';

        AdjMatrixBFS(graph, start, order.data(), &count);
        PrintOrder("BFS", order.data(), count);

        AdjMatrixDFS(graph, start, order.data(), &count);
        PrintOrder("DFS", order.data(), count);

        int ok = AdjMatrixTopologicalSort(graph, order.data(), &count);
        std::cout << "TOPO_OK " << ok << '\n';
        PrintOrder("TOPO", order.data(), ok ? count : 0);

        DisposeAdjMatrixGraph(graph);
    }

    return 0;
}