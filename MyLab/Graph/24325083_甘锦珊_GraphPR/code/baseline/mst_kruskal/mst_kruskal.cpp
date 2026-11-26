#include "mst_kruskal.hpp"

#include <algorithm>
#include <vector>

class DisjointSet
{
public:
    explicit DisjointSet(int size) : parent(size), rank(size, 0)
    {
        for (int i = 0; i < size; ++i) {
            parent[i] = i;
        }
    }

    int Find(int value)
    {
        if (parent[value] != value) {
            parent[value] = Find(parent[value]);
        }
        return parent[value];
    }

    bool Union(int left, int right)
    {
        int left_root = Find(left);
        int right_root = Find(right);
        if (left_root == right_root) {
            return false;
        }

        if (rank[left_root] < rank[right_root]) {
            parent[left_root] = right_root;
        } else if (rank[left_root] > rank[right_root]) {
            parent[right_root] = left_root;
        } else {
            parent[right_root] = left_root;
            ++rank[left_root];
        }
        return true;
    }

private:
    std::vector<int> parent;
    std::vector<int> rank;
};

static bool IsValidEdge(const Edge &edge, int vertex_count)
{
    return edge.From >= 0 && edge.From < vertex_count && edge.To >= 0 && edge.To < vertex_count;
}

int Kruskal(Edge *edges, int edge_count, int vertex_count, Edge *mst_edges, int *total_cost)
{
    if (edges == nullptr || mst_edges == nullptr || total_cost == nullptr || vertex_count <= 0 || edge_count < 0) {
        return 0;
    }

    std::vector<Edge> sorted_edges(edges, edges + edge_count);
    std::sort(sorted_edges.begin(), sorted_edges.end(), [](const Edge &left, const Edge &right) {
        return left.Weight < right.Weight;
    });

    DisjointSet set(vertex_count);
    int selected_count = 0;
    *total_cost = 0;

    for (const Edge &edge : sorted_edges) {
        if (!IsValidEdge(edge, vertex_count)) {
            return 0;
        }

        if (set.Union(edge.From, edge.To)) {
            mst_edges[selected_count] = edge;
            *total_cost += edge.Weight;
            ++selected_count;

            if (selected_count == vertex_count - 1) {
                return 1;
            }
        }
    }

    return selected_count == vertex_count - 1 ? 1 : 0;
}
