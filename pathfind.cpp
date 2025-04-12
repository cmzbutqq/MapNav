#include "pathfind.h"
#include <limits>
#include <queue>
#include <unordered_map>

Dijkstra::Dijkstra(Map *map, int startVertexId, int endVertexId,
                   bool considerAccident)
    : m_map(map), m_startVertexId(startVertexId), m_endVertexId(endVertexId),
      m_considerAccident(considerAccident) {}

PathResult Dijkstra::findPath() {
    PathResult result;
    if (!m_map || m_startVertexId < 0 ||
        m_startVertexId >= m_map->vertices.size() || m_endVertexId < 0 ||
        m_endVertexId >= m_map->vertices.size()) {
        result.success = false;
        return result;
    }

    // 优先队列，存储(累计距离, 顶点ID)
    using QueueItem = std::pair<double, int>; // using别名声明，定义了一个类型别名QueueItem，是std::pair<double,
                                              // int>的别名
    std::priority_queue<QueueItem, std::vector<QueueItem>,
                        std::greater<QueueItem>>
        queue;
    /*
    这行代码定义了一个优先队列 queue，下面详细解释其模板参数：
    QueueItem：这是优先队列中存储的元素类型，
    std::vector<QueueItem>：这是优先队列底层使用的容器类型。std::priority_queue
    是一个容器适配器，它需要一个底层容器来存储元素，这里选择了
    std::vector。std::vector
    是一个动态数组，具有随机访问的特性，适合用于优先队列的底层存储。
    std::greater<QueueItem>：这是一个比较函数对象，用于定义优先队列中元素的优先级顺序。std::greater<QueueItem>
    表示使用 “大于”
    比较，即优先队列会按照元素的第一个元素（累计距离）从小到大的顺序进行排序。这样，优先队列的顶部元素（queue.top()）总是累计距离最小的元素。
    */

    // 记录距离、前驱节点和前驱边
    std::unordered_map<int, double>
        distances; // 用于记录从起始顶点到图中各个顶点的当前最短距离,键（int
                   // 类型）：表示图中顶点的 ID。值（double
                   // 类型）：表示从起始顶点到该顶点的当前最短距离。
    std::unordered_map<int, int>
        predecessors; // 记录图中每个顶点在最短路径上的前驱顶点,键（int
                      // 类型）：表示图中顶点的 ID。值（int
                      // 类型）：表示该顶点在最短路径上的前驱顶点的 ID。
    std::unordered_map<int, int>
        edgePredecessors; // 记录图中每个顶点在最短路径上到达该顶点所经过的边的
                          // ID,键（int 类型）：表示图中顶点的 ID。值（int
                          // 类型）：表示到达该顶点所经过的边的 ID

    // 初始化
    for (int i = 0; i < m_map->vertices.size(); ++i) {
        distances[i] = std::numeric_limits<double>::max();
    }
    distances[m_startVertexId] = 0.0;
    queue.emplace(0.0, m_startVertexId);

    // 主循环
    while (!queue.empty()) {
        auto [currentDist, current] = queue.top();
        queue.pop();

        if (current == m_endVertexId)
            break; // 找到终点
        if (currentDist > distances[current])
            continue; // 已有更优路径

        const Vertex &vertex = m_map->vertices[current];

        // 遍历当前结点的所有邻边
        for (int edgeId : vertex.connectedEdges) {
            Edge &edge = m_map->edges[edgeId];

            if (m_considerAccident && edge.ishappeningaccident()) {
                continue; // 考虑事故且该边发生事故，跳过
            }

            int neighbor = (edge.fromVertex == current)
                               ? edge.toVertex
                               : edge.fromVertex; // 找到这条边的另外一个结点
            double newDist = currentDist + edge.length;

            if (newDist < distances[neighbor]) {
                distances[neighbor] = newDist;
                predecessors[neighbor] = current;
                edgePredecessors[neighbor] = edgeId;
                queue.emplace(newDist, neighbor);
            }
        }
    }

    // 检查是否找到路径
    if (distances[m_endVertexId] == std::numeric_limits<double>::max()) {
        result.success = false;
        return result;
    }

    // 回溯路径
    result.path.push_back(
        m_endVertexId); // 通过终点和前面定义的三个变量来找到路径
    int current = m_endVertexId;
    while (current != m_startVertexId) {
        int edgeId = edgePredecessors[current];
        result.edges.push_back(edgeId);
        current = predecessors[current];
        result.path.push_back(current);
    }
    std::reverse(result.path.begin(), result.path.end()); // 将顺序倒置
    std::reverse(result.edges.begin(), result.edges.end());

    result.distance = distances[m_endVertexId];
    result.success = true;
    return result;
}

// 考虑路况的Dijkstra类的构造函数，TrafficAwareDijkstra 类的 findPath 方法和
// Dijkstra 算法的核心逻辑基本一致，主要区别在于距离度量上，TrafficAwareDijkstra
// 把边的通行时间作为 “边长”
// Dijkstra 类：在考虑事故的情况下，若边发生事故，则跳过该边。
// TrafficAwareDijkstra
// 类：在考虑事故情况下，计算通行时间时，产生事故的边也会计算并判断。因为我在Edge中定义的getCurrentTravelTime函数中，即使边发生了事故，也会返回时间
TrafficAwareDijkstra::TrafficAwareDijkstra(Map *map, int startVertexId,
                                           int endVertexId,
                                           bool considerAccident)
    : m_map(map), m_startVertexId(startVertexId), m_endVertexId(endVertexId),
      m_considerAccident(considerAccident) {}

// 新类的寻路方法
PathResult TrafficAwareDijkstra::findPath() {
    PathResult result;
    if (!m_map || m_startVertexId < 0 ||
        m_startVertexId >= m_map->vertices.size() || m_endVertexId < 0 ||
        m_endVertexId >= m_map->vertices.size()) {
        result.success = false;
        return result;
    }

    // 优先队列，存储(累计时间, 顶点ID)
    using QueueItem = std::pair<double, int>;
    std::priority_queue<QueueItem, std::vector<QueueItem>,
                        std::greater<QueueItem>>
        queue;

    // 记录时间、前驱节点和前驱边
    std::unordered_map<int, double> times;
    std::unordered_map<int, int> predecessors;
    std::unordered_map<int, int> edgePredecessors;

    // 初始化
    for (int i = 0; i < m_map->vertices.size(); ++i) {
        times[i] = std::numeric_limits<double>::max();
    }
    times[m_startVertexId] = 0.0;
    queue.emplace(0.0, m_startVertexId);

    // 主循环
    while (!queue.empty()) {
        auto [currentTime, current] = queue.top();
        queue.pop();

        if (current == m_endVertexId)
            break; // 找到终点
        if (currentTime > times[current])
            continue; // 已有更优路径

        const Vertex &vertex = m_map->vertices[current];

        // 遍历当前结点的所有邻边
        for (int edgeId : vertex.connectedEdges) {
            Edge &edge = m_map->edges[edgeId];

            if (m_considerAccident && edge.ishappeningaccident()) {
                continue; // 考虑事故且该边发生事故，跳过
            }

            int neighbor =
                (edge.fromVertex == current) ? edge.toVertex : edge.fromVertex;
            double travelTime = edge.getCurrentTravelTime();
            double newTime = currentTime + travelTime;

            if (newTime < times[neighbor]) {
                times[neighbor] = newTime;
                predecessors[neighbor] = current;
                edgePredecessors[neighbor] = edgeId;
                queue.emplace(newTime, neighbor);
            }
        }
    }

    // 检查是否找到路径
    if (times[m_endVertexId] == std::numeric_limits<double>::max()) {
        result.success = false;
        return result;
    }

    // 回溯路径
    result.path.push_back(m_endVertexId);
    int current = m_endVertexId;
    while (current != m_startVertexId) {
        int edgeId = edgePredecessors[current];
        result.edges.push_back(edgeId);
        current = predecessors[current];
        result.path.push_back(current);
    }
    std::reverse(result.path.begin(), result.path.end());
    std::reverse(result.edges.begin(), result.edges.end());

    result.distance = times[m_endVertexId];
    result.success = true;
    return result;
}

// RouteThroughPoints类的实现
/*
findShortestLengthPath 方法使用 Dijkstra 类计算最短长度的路线。
findShortestTimePath 方法使用 TrafficAwareDijkstra 类计算最短时间的路线。
在 findShortestLengthPath 和 findShortestTimePath
方法中，将相邻两点之间的子路径拼接起来，形成最终的路径。
 */
RouteThroughPoints::RouteThroughPoints(Map *map, const std::vector<int> &points,
                                       bool considerAccident)
    : m_map(map), m_points(points), m_considerAccident(considerAccident) {}

PathResult RouteThroughPoints::findShortestLengthPath() {
    PathResult finalResult;
    finalResult.success = true;

    for (size_t i = 0; i < m_points.size() - 1; ++i) {
        Dijkstra dijkstra(m_map, m_points[i], m_points[i + 1],
                          m_considerAccident);
        PathResult subResult = dijkstra.findPath();

        if (!subResult.success) {
            finalResult.success = false;
            return finalResult;
        }

        if (i > 0) {
            subResult.path.erase(subResult.path.begin()); // 避免重复添加起点
        }

        finalResult.path.insert(finalResult.path.end(), subResult.path.begin(),
                                subResult.path.end());
        finalResult.edges.insert(finalResult.edges.end(),
                                 subResult.edges.begin(),
                                 subResult.edges.end());
        finalResult.distance += subResult.distance;
    }

    return finalResult;
}

PathResult RouteThroughPoints::findShortestTimePath() {
    PathResult finalResult;
    finalResult.success = true;

    for (size_t i = 0; i < m_points.size() - 1; ++i) {
        TrafficAwareDijkstra dijkstra(m_map, m_points[i], m_points[i + 1],
                                      m_considerAccident);
        PathResult subResult = dijkstra.findPath();

        if (!subResult.success) {
            finalResult.success = false;
            return finalResult;
        }

        if (i > 0) {
            subResult.path.erase(subResult.path.begin()); // 避免重复添加起点
        }

        finalResult.path.insert(finalResult.path.end(), subResult.path.begin(),
                                subResult.path.end());
        finalResult.edges.insert(finalResult.edges.end(),
                                 subResult.edges.begin(),
                                 subResult.edges.end());
        finalResult.distance += subResult.distance;
    }

    return finalResult;
}

// EfficientRouteThroughPointsUnordered的实现，动态规划 +
// 状态压缩，这个方法在必经点较少的时候表现较为出色，并且符合现实生活情况，复杂度：O(k²·2^k)（k为必经点数量）
EfficientRouteThroughPointsUnordered::EfficientRouteThroughPointsUnordered(
    Map *map, const std::vector<int> &points, bool considerAccident)
    : m_map(map), m_points(points), m_considerAccident(considerAccident) {}

/*
 alculateDistances 函数的主要功能是计算 m_points
中任意两点之间的最短距离，并将这些距离存储在一个二维向量中返回。 根据
considerTime 参数的值，函数会选择使用 Dijkstra 算法或 TrafficAwareDijkstra
算法来计算最短距离。
 */
std::vector<std::vector<double>>
EfficientRouteThroughPointsUnordered::calculateDistances(bool considerTime) {
    int n = m_points.size();
    std::vector<std::vector<double>> distances(
        n, std::vector<double>(n, std::numeric_limits<double>::max()));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                distances[i][j] = 0;
            } else {
                if (considerTime) {
                    TrafficAwareDijkstra dijkstra(
                        m_map, m_points[i], m_points[j], m_considerAccident);
                    PathResult result = dijkstra.findPath();
                    if (result.success) {
                        distances[i][j] = result.distance;
                    }
                } else {
                    Dijkstra dijkstra(m_map, m_points[i], m_points[j],
                                      m_considerAccident);
                    PathResult result = dijkstra.findPath();
                    if (result.success) {
                        distances[i][j] = result.distance;
                    }
                }
            }
        }
    }

    return distances;
}

// findShortestLengthPath
// 函数的主要功能是在不考虑点的顺序的情况下，找到经过指定的所有点的最短长度路径。
PathResult EfficientRouteThroughPointsUnordered::findShortestLengthPath() {
    int n = m_points.size();
    std::vector<std::vector<double>> distances = calculateDistances(false);

    // 状态压缩动态规划,  dp 数组用于存储状态和最短距离。dp[mask][i]
    // 表示已经经过的点的状态为 mask，且当前位于点 i
    // 时的最短路径长度。初始值设为无穷大。
    // prev 数组用于记录路径的前驱信息。prev[mask][i] 表示在状态 mask 下到达点 i
    // 的前一个点的索引。初始值设为 -1。
    std::vector<std::vector<double>> dp(
        1 << n, std::vector<double>(n, std::numeric_limits<double>::max()));
    std::vector<std::vector<int>> prev(1 << n, std::vector<int>(n, -1));

    // 初始化
    for (int i = 0; i < n; ++i) {
        dp[1 << i][i] = 0;
    }

    // 动态规划过程
    for (int mask = 1; mask < (1 << n); ++mask) {
        for (int i = 0; i < n; ++i) {
            if (mask & (1 << i)) {
                for (int j = 0; j < n; ++j) {
                    if (!(mask & (1 << j)) &&
                        distances[i][j] != std::numeric_limits<double>::max()) {
                        int newMask = mask | (1 << j);
                        if (dp[mask][i] + distances[i][j] < dp[newMask][j]) {
                            dp[newMask][j] = dp[mask][i] + distances[i][j];
                            prev[newMask][j] = i;
                        }
                    }
                }
            }
        }
    }

    // 找到最短路径的终点
    double minDistance = std::numeric_limits<double>::max();
    int end = -1;
    for (int i = 0; i < n; ++i) {
        if (dp[(1 << n) - 1][i] < minDistance) {
            minDistance = dp[(1 << n) - 1][i];
            end = i;
        }
    }

    // 回溯路径
    PathResult result;
    if (end != -1) {
        result.success = true;
        result.distance = minDistance;
        int mask = (1 << n) - 1;
        int current = end;
        std::vector<int> path;
        while (current != -1) {
            path.push_back(m_points[current]);
            int prevMask = mask ^ (1 << current);
            current = prev[mask][current];
            mask = prevMask;
        }
        std::reverse(path.begin(), path.end());

        // 拼接子路径
        for (size_t i = 0; i < path.size() - 1; ++i) {
            Dijkstra dijkstra(m_map, path[i], path[i + 1], m_considerAccident);
            PathResult subResult = dijkstra.findPath();
            if (i > 0) {
                subResult.path.erase(
                    subResult.path.begin()); // 避免重复添加起点
            }
            result.path.insert(result.path.end(), subResult.path.begin(),
                               subResult.path.end());
            result.edges.insert(result.edges.end(), subResult.edges.begin(),
                                subResult.edges.end());
        }
    }

    return result;
}

PathResult EfficientRouteThroughPointsUnordered::findShortestTimePath() {
    int n = m_points.size();
    std::vector<std::vector<double>> distances = calculateDistances(true);

    // 状态压缩动态规划
    std::vector<std::vector<double>> dp(
        1 << n, std::vector<double>(n, std::numeric_limits<double>::max()));
    std::vector<std::vector<int>> prev(1 << n, std::vector<int>(n, -1));

    // 初始化
    for (int i = 0; i < n; ++i) {
        dp[1 << i][i] = 0;
    }

    // 动态规划过程
    for (int mask = 1; mask < (1 << n); ++mask) {
        for (int i = 0; i < n; ++i) {
            if (mask & (1 << i)) {
                for (int j = 0; j < n; ++j) {
                    if (!(mask & (1 << j)) &&
                        distances[i][j] != std::numeric_limits<double>::max()) {
                        int newMask = mask | (1 << j);
                        if (dp[mask][i] + distances[i][j] < dp[newMask][j]) {
                            dp[newMask][j] = dp[mask][i] + distances[i][j];
                            prev[newMask][j] = i;
                        }
                    }
                }
            }
        }
    }

    // 找到最短路径的终点
    double minTime = std::numeric_limits<double>::max();
    int end = -1;
    for (int i = 0; i < n; ++i) {
        if (dp[(1 << n) - 1][i] < minTime) {
            minTime = dp[(1 << n) - 1][i];
            end = i;
        }
    }

    // 回溯路径
    PathResult result;
    if (end != -1) {
        result.success = true;
        result.distance = minTime;
        int mask = (1 << n) - 1;
        int current = end;
        std::vector<int> path;
        while (current != -1) {
            path.push_back(m_points[current]);
            int prevMask = mask ^ (1 << current);
            current = prev[mask][current];
            mask = prevMask;
        }
        std::reverse(path.begin(), path.end());

        // 拼接子路径
        for (size_t i = 0; i < path.size() - 1; ++i) {
            TrafficAwareDijkstra dijkstra(m_map, path[i], path[i + 1],
                                          m_considerAccident);
            PathResult subResult = dijkstra.findPath();
            if (i > 0) {
                subResult.path.erase(
                    subResult.path.begin()); // 避免重复添加起点
            }
            result.path.insert(result.path.end(), subResult.path.begin(),
                               subResult.path.end());
            result.edges.insert(result.edges.end(), subResult.edges.begin(),
                                subResult.edges.end());
        }
    }

    return result;
}
