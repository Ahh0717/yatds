#include "dijkstra.hpp"

#include <fstream>
#include <iostream>
#include <vector>

static void PrintArray(const char *label, const int *values, int count)
{
    std::cout << label;
    for (int i = 0; i < count; ++i) {
        std::cout << ' ' << values[i];
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
        int source = 0;
        input >> vertex_count >> edge_count >> directed >> source;

        WGraph graph = CreateDijkstraGraph(vertex_count, directed);

        for (int i = 0; i < edge_count; ++i) {
            int from = 0;
            int to = 0;
            int weight = 0;
            input >> from >> to >> weight;
            AddDijkstraEdge(graph, from, to, weight);
        }

        std::vector<int> dist(vertex_count, -1);
        std::vector<int> prev(vertex_count, -1);

        Dijkstra(graph, source, dist.data(), prev.data());

        std::cout << "CASE " << case_index << '\n';
        PrintArray("DIST", dist.data(), vertex_count);
        PrintArray("PREV", prev.data(), vertex_count);

        DisposeDijkstraGraph(graph);
    }

    return 0;
}