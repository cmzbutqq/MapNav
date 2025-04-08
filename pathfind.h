#ifndef PATHFIND_H
#define PATHFIND_H

#include "core.h"
#include <vector>
#include <memory>

struct PathResult {
    bool success = false;
    double distance = 0.0;
    std::vector<int> path; // 顶点ID序列
};

class PathFinder {
public:
    enum class Algorithm {
        Dijkstra,
        AStar
    };

    explicit PathFinder(Map* map);
    virtual ~PathFinder() = default;

    virtual PathResult findPath(int start, int end) = 0;

    static std::unique_ptr<PathFinder> create(Algorithm algo, Map* map);

protected:
    Map* m_map = nullptr;
};

#endif // PATHFIND_H
