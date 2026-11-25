#include "mst_prim.hpp"

#include <fstream>
#include <iostream>
#include <vector>

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
        input >> vertex_count >> edge_count;

        PrimGraph graph = CreatePrimGraph(vertex_count);

        for (int i = 0; i < edge_count; ++i) {
            int from = 0;
            int to = 0;
            int weight = 0;
            input >> from >> to >> weight;
            AddPrimEdge(graph, from, to, weight);
        }

        std::vector<Edge> mst_edges(vertex_count > 1 ? vertex_count - 1 : 1);
        int total_cost = 0;
        int ok = Prim(graph, mst_edges.data(), &total_cost);

        std::cout << "CASE " << case_index << '\n';
        std::cout << "OK " << ok << '\n';
        std::cout << "TOTAL " << (ok ? total_cost : 0) << '\n';
        std::cout << "EDGE_COUNT " << (ok ? vertex_count - 1 : 0) << '\n';
        if (ok) {
            for (int i = 0; i < vertex_count - 1; ++i) {
                std::cout << mst_edges[i].From << ' ' << mst_edges[i].To << ' ' << mst_edges[i].Weight << '\n';
            }
        }

        DisposePrimGraph(graph);
    }

    return 0;
}