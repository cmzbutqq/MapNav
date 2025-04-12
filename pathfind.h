#ifndef PATHFIND_H
#define PATHFIND_H

#include "core.h"
#include <vector>

struct PathResult {
    bool success = false;
    double distance = 0.0;
    std::vector<int> path; // 顶点ID序列
    std::vector<int> edges; // 边ID序列
};

class Dijkstra {
public:
    Dijkstra(Map* map, int startVertexId, int endVertexId, bool considerAccident);
    PathResult findPath();

private:
    Map* m_map;
    int m_startVertexId;
    int m_endVertexId;
    bool m_considerAccident;
};

// 考虑路况的Dijkstra算法
class TrafficAwareDijkstra {
public:
    TrafficAwareDijkstra(Map* map, int startVertexId, int endVertexId, bool considerAccident);
    PathResult findPath();

private:
    Map* m_map;
    int m_startVertexId;
    int m_endVertexId;
    bool m_considerAccident;
};

// 经过指定点的最优路线查找（严格按照顺序）
class RouteThroughPoints {
public:
    RouteThroughPoints(Map* map, const std::vector<int>& points, bool considerAccident);
    PathResult findShortestLengthPath();
    PathResult findShortestTimePath();

private:
    Map* m_map;
    std::vector<int> m_points;
    bool m_considerAccident;
};


// 经过指定点的最优路线查找（不严格按照顺序，使用高效算法）
class EfficientRouteThroughPointsUnordered {
public:
    EfficientRouteThroughPointsUnordered(Map* map, const std::vector<int>& points, bool considerAccident);
    PathResult findShortestLengthPath();
    PathResult findShortestTimePath();

private:
    Map* m_map;
    std::vector<int> m_points;
    bool m_considerAccident;
    std::vector<std::vector<double>> calculateDistances(bool considerTime);
};

#endif // PATHFIND_H
