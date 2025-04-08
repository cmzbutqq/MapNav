#include "pathfind.h"
#include <queue>
#include <unordered_map>
#include <limits>
#include <QDebug>

// =====寻路算法基类=====
PathFinder::PathFinder(Map* map) : m_map(map) {}

// =====Dijkstra算法实现=====
class DijkstraFinder : public PathFinder {
public:
    DijkstraFinder(Map* map) : PathFinder(map) {}

    PathResult findPath(int start, int end) override {
        PathResult result;
        if (!m_map || !m_map->getVertex(start) || !m_map->getVertex(end)) {
            result.success = false;
            return result;
        }

        // 优先队列，存储(累计距离, 顶点ID)
        using QueueItem = std::pair<double, int>;
        std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> queue;

        // 记录距离和前驱节点
        std::unordered_map<int, double> distances;
        std::unordered_map<int, int> predecessors;

        // 初始化
        for (int i = 0; i < m_map->getVertexCount(); ++i) {
            distances[i] = std::numeric_limits<double>::max();
        }
        distances[start] = 0.0;
        queue.emplace(0.0, start);

        // 主循环
        while (!queue.empty()) {
            auto [currentDist, current] = queue.top();
            queue.pop();

            if (current == end) break; // 找到终点
            if (currentDist > distances[current]) continue; // 已有更优路径

            const Vertex* vertex = m_map->getVertex(current);
            if (!vertex) continue;

            // 遍历所有邻边
            for (int edgeId : vertex->connectedEdges) {
                const Edge* edge = m_map->getEdge(edgeId);
                if (!edge) continue;

                int neighbor = (edge->fromVertex == current) ? edge->toVertex : edge->fromVertex;
                double newDist = currentDist + edge->length;

                if (newDist < distances[neighbor]) {
                    distances[neighbor] = newDist;
                    predecessors[neighbor] = current;
                    queue.emplace(newDist, neighbor);
                }
            }
        }

        // 检查是否找到路径
        if (distances[end] == std::numeric_limits<double>::max()) {
            result.success = false;
            return result;
        }

        // 回溯路径
        result.path.push_back(end);
        int current = end;
        while (current != start) {
            current = predecessors[current];
            result.path.push_back(current);
        }
        std::reverse(result.path.begin(), result.path.end());

        result.distance = distances[end];
        result.success = true;
        return result;
    }
};

// =====A*算法实现=====
class AStarFinder : public PathFinder {
public:
    AStarFinder(Map* map) : PathFinder(map) {}

    PathResult findPath(int start, int end) override {
        PathResult result;
        // 实现类似Dijkstra，但使用启发式函数
        // 这里省略具体实现...
        return result;
    }
};

// =====寻路工厂=====
std::unique_ptr<PathFinder> PathFinder::create(Algorithm algo, Map* map) {
    switch (algo) {
    case Algorithm::Dijkstra:
        return std::make_unique<DijkstraFinder>(map);
    case Algorithm::AStar:
        return std::make_unique<AStarFinder>(map);
    default:
        return nullptr;
    }
}
