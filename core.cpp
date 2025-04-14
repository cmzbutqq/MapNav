#include "core.h"
#include <QRandomGenerator>
#include <algorithm>
#include <iostream>
#include <queue>
#include <set>
#include <vector>

// =====顶点类实现=====
Vertex::Vertex(int id, const QPointF &pos) : id(id), position(pos) {}

void Vertex::addEdge(int edgeId) { connectedEdges.insert(edgeId); }

// =====边类实现=====
Edge::Edge(int id, int from, int to, double len)
    : id(id), fromVertex(from), toVertex(to), length(len) {
    capacity = length * 0.5 + 7;

    int carcount = int(randomInRange(0, capacity + 10));
    currentVehicles = carcount;
    for (int i = 0; i < carcount; i++) {
        double direction = randomInRange(0, 1);
        if (direction > 0.5) {
            vehiclesets.push_back(Vehicle(id, from, to, randomInRange(0, 1)));
        } else {
            vehiclesets.push_back(Vehicle(id, to, from, randomInRange(0, 1)));
        }
    }
}

double Edge::getCurrentTravelTime() {
    const double c = 1.0;
    const double threshold = 0.8;
    double ratio = double(vehiclesets.count()) / double(capacity);

    if (ishappeningaccident()) {
        return randomInRange(3000, 6000);
    }

    if (ratio <= threshold) {
        return c * length;
    }
    return c * length * (1 + std::exp(ratio - threshold));
}

bool Edge::ishappeningaccident() {
    if (vehiclesets.count() > capacity + 20) {
        return (randomInRange(0, 1) > 0.95);
    }
    return 0;
}

// =====网格单元类实现=====
void GridCell::addVertex(int vertexId) { vertices.insert(vertexId); }

// 判断两条线段是否交叉的辅助函数
bool isCrossing(const QPointF &p1, const QPointF &p2, const QPointF &p3,
                const QPointF &p4) {
    auto orientation = [](const QPointF &p, const QPointF &q,
                          const QPointF &r) {
        double val = (q.y() - p.y()) * (r.x() - q.x()) -
                     (q.x() - p.x()) * (r.y() - q.y());
        if (val == 0)
            return 0;             // 共线
        return (val > 0) ? 1 : 2; // 顺时针或逆时针
    };

    int o1 = orientation(p1, p2, p3);
    int o2 = orientation(p1, p2, p4);
    int o3 = orientation(p3, p4, p1);
    int o4 = orientation(p3, p4, p2);

    return (o1 != o2 && o3 != o4);
}

// =====地图类实现=====
Map::Map() {
    Vertexcount = 100;
    gridSize = GRID_DIM;

    // 随机生成顶点
    for (int i = 0; i < Vertexcount; i++) {
        double x = randomInRange(0, 1000);
        double y = randomInRange(0, 1000);
        vertices.push_back(Vertex(i, QPointF(x, y)));
    }

    // 生成最小生成树（MST）
    std::vector<bool> inMST(Vertexcount, false);
    std::priority_queue<std::pair<double, std::pair<int, int>>,
                        std::vector<std::pair<double, std::pair<int, int>>>,
                        std::greater<std::pair<double, std::pair<int, int>>>>
        pq;
    inMST[0] = true;
    /*创建一个优先队列 pq，用于存储待处理的边。*/

    // 初始化优先队列
    for (int i = 1; i < Vertexcount; i++) {
        double len = std::sqrt(
            (vertices[0].position.x() - vertices[i].position.x()) *
                (vertices[0].position.x() - vertices[i].position.x()) +
            (vertices[0].position.y() - vertices[i].position.y()) *
                (vertices[0].position.y() - vertices[i].position.y()));
        pq.push({len, {0, i}});
    }

    Edgecount = 0;
    std::set<std::pair<int, int>> addedEdges;
    std::vector<Edge> mstEdges;

    while (!pq.empty()) {
        // 对于10000个节点的地图来说，这里的循环会生成包含5000,0000个元素的pq,并且要将这五千万个元素一个个排空，崔少旭你确定有跑过自己写的代码吗？
        std::cout<<"pq.size: "<<pq.size()<<std::endl;
        auto top = pq.top();
        pq.pop();
        double len = top.first;
        int from = top.second.first;
        int to =
            top.second
                .second; // 这里的from 是已经在mst中的点了，to
                         // 是待检测是否能加入的点，把这块代码完整看完就直到为什么from一定是mst
                         // 中的点

        if (inMST[to])
            continue;

        inMST[to] = true;
        std::pair<int, int> edge = std::make_pair(
            std::min(from, to),
            std::max(from,
                     to)); // 通过这种方式，无论 from 和 to 的顺序如何，edge
                           // 都会以统一的顺序表示同一条边，
        if (addedEdges.find(edge) ==
            addedEdges.end()) { // 如果 edge 不在 addedEdges 集合中
            mstEdges.push_back(Edge(Edgecount++, from, to, len));
            vertices[from].addEdge(Edgecount - 1);
            vertices[to].addEdge(Edgecount - 1);
            addedEdges.insert(edge);
        }

        for (int i = 0; i < Vertexcount; i++) {
            if (!inMST[i]) {
                double newLen = std::sqrt(
                    (vertices[to].position.x() - vertices[i].position.x()) *
                        (vertices[to].position.x() - vertices[i].position.x()) +
                    (vertices[to].position.y() - vertices[i].position.y()) *
                        (vertices[to].position.y() - vertices[i].position.y()));
                pq.push({newLen, {to, i}});
            }
        }
    }

    // 将 MST 边添加到 edges 中

    // 将 std::vector<Edge> 转换为 QVector<Edge>
    edges = QVector<Edge>(mstEdges.begin(), mstEdges.end());

    // 向 MST 中增加边，同时检查是否交叉
    int additionalEdges = Vertexcount / 2; // 增加一些额外的边
    for (int i = 0; i < additionalEdges; i++) {
        int from, to;
        bool validEdge = false;
        do {
            from = QRandomGenerator::global()->bounded(Vertexcount);
            do {
                to = QRandomGenerator::global()->bounded(Vertexcount);
            } while (from == to);

            std::pair<int, int> edge =
                std::make_pair(std::min(from, to), std::max(from, to));
            if (addedEdges.find(edge) ==
                addedEdges
                    .end()) { // edge不在addededges中，addededges中的边都是小数结点指向大数结点，这样防止边重复
                validEdge = true;
                // 检查新边是否与现有边交叉
                for (const auto &existingEdge : edges) { // 你就一定要遍历所有边吗？
                    if (isCrossing(vertices[from].position,
                                   vertices[to].position,
                                   vertices[existingEdge.fromVertex].position,
                                   vertices[existingEdge.toVertex].position)) {
                        validEdge = false;
                        break;
                    }
                }
            }
        } while (!validEdge); // 这里也是一样的，你不怕死循环？

        double len = std::sqrt(
            (vertices[from].position.x() - vertices[to].position.x()) *
                (vertices[from].position.x() - vertices[to].position.x()) +
            (vertices[from].position.y() - vertices[to].position.y()) *
                (vertices[from].position.y() - vertices[to].position.y()));
        edges.push_back(Edge(Edgecount++, from, to, len));
        vertices[from].addEdge(Edgecount - 1);
        vertices[to].addEdge(Edgecount - 1);
        addedEdges.insert(
            std::make_pair(std::min(from, to), std::max(from, to)));
    }

    // 遍历 edges 中的每一条边，对于每条边，将其终点的 id 插入到起点的
    // connectedVertexs 集合中，同时将起点的 id 插入到终点的 connectedVertexs
    // 集合中。
    for (const auto &edge : edges) {
        vertices[edge.fromVertex].connectedVertexs.insert(edge.toVertex);
        vertices[edge.toVertex].connectedVertexs.insert(edge.fromVertex);
    }
    Edgecount = edges.count();

    printEdges();
}

void Map::printEdges() {
    std::cout << "Total edges: " << Edgecount << std::endl;
    for (const auto &edge : edges) {
        std::cout << "Edge " << edge.id << ": "
                  << "From " << edge.fromVertex << " ("
                  << vertices[edge.fromVertex].position.x() << ", "
                  << vertices[edge.fromVertex].position.y() << ")"
                  << " To " << edge.toVertex << " ("
                  << vertices[edge.toVertex].position.x() << ", "
                  << vertices[edge.toVertex].position.y() << ")"
                  << " Length: " << edge.length
                  << " Capacity: " << edge.capacity
                  << " Current vehicles: " << edge.currentVehicles << std::endl;
    }
}

const Vertex *Map::getVertex(int id) const {
    if (id < 0 || id >= Vertexcount)
        return nullptr;
    return &vertices[id];
}

const Edge *Map::getEdge(int id) const {
    if (id < 0 || id >= Edgecount)
        return nullptr;
    return &edges[id];
}

// =====模拟器类实现=====
Simulator::Simulator(Map *map) : m_map(map) {}

void Simulator::update(double deltaTime) {
    // 先将所有车辆的 processed 标记置为 false
    for (auto &edge : m_map->edges) {
        for (auto &vehicle : edge.vehiclesets) {
            vehicle.processed = false;
        }
    }

    // 遍历每条边
    for (auto &edge : m_map->edges) {
        if (edge.ishappeningaccident()) {
            continue;
        }

        double travelTime = edge.getCurrentTravelTime();

        // 遍历当前边上的每辆车
        auto it = edge.vehiclesets.begin();
        while (it != edge.vehiclesets.end()) {
            Vehicle &vehicle = *it;

            // 若车辆已被处理过，则跳过
            if (vehicle.processed) {
                ++it;
                continue;
            }

            // 标记车辆为已处理
            vehicle.processed = true;

            // 更新车辆的进度
            vehicle.progress += deltaTime / travelTime;

            if (vehicle.progress >= 1.0) {
                // 车辆走完当前路段
                int currentVertex = vehicle.to;

                // 概率：车辆停止
                if (QRandomGenerator::global()->generateDouble() < 0.1) {
                    // 车辆停止
                    it = edge.vehiclesets.erase(it);
                    edge.currentVehicles--;
                    continue;
                }

                // 获取当前顶点连接的边
                Vertex &vertex = m_map->vertices[currentVertex];
                std::vector<int>
                    connectedEdges; // 当前可用的边的向量，不包含发生事故的边
                std::vector<double> probabilities;
                double totalWeight = 0.0;

                for (int edgeId : vertex.connectedEdges) {
                    // 直接访问 edges 向量获取边
                    Edge &nextEdge = m_map->edges[edgeId];
                    if (nextEdge.ishappeningaccident()) {
                        continue;
                    }
                    double weight = 1.0 / (nextEdge.getCurrentTravelTime() +
                                           1e-6); // 反比于通行时间
                    totalWeight += weight;
                    connectedEdges.push_back(edgeId);
                    probabilities.push_back(weight);
                }

                // 归一化概率
                for (double &prob : probabilities) {
                    prob /= totalWeight;
                }

                // 根据概率选择下一条边
                double randomValue =
                    QRandomGenerator::global()->generateDouble();
                double cumulativeProb = 0.0;
                int nextEdgeId = -1;
                for (size_t i = 0; i < connectedEdges.size(); ++i) {
                    cumulativeProb += probabilities[i];
                    if (randomValue < cumulativeProb) {
                        nextEdgeId = connectedEdges[i];
                        break;
                    }
                }

                if (nextEdgeId != -1) {
                    // 直接访问 edges 向量获取边
                    Edge &nextEdge = m_map->edges[nextEdgeId];
                    int nextFrom = currentVertex;
                    int nextTo = (nextEdge.fromVertex == currentVertex)
                                     ? nextEdge.toVertex
                                     : nextEdge.fromVertex;

                    // 将车辆移动到下一条边
                    double newProgress =
                        (vehicle.progress - 1.0) * travelTime /
                        nextEdge.getCurrentTravelTime(); // 计算 progress
                    Vehicle newVehicle(nextEdgeId, nextFrom, nextTo,
                                       newProgress);
                    newVehicle.processed =
                        1; // 这里设为1就可以防止重复更新一辆车
                    nextEdge.vehiclesets.push_back(newVehicle);
                    nextEdge.currentVehicles++;
                    it = edge.vehiclesets.erase(it);
                    edge.currentVehicles--;
                } else {
                    // 如果没有找到下一条边，车辆停止
                    it = edge.vehiclesets.erase(it);
                    edge.currentVehicles--;
                }
            } else {
                ++it;
            }
        }

        // 概率：车辆产生
        if (QRandomGenerator::global()->generateDouble() < 0.02 &&
            !edge.ishappeningaccident()) {
            int from = edge.fromVertex;
            int to = edge.toVertex;
            edge.vehiclesets.push_back(Vehicle(edge.id, from, to, 0.0));
            edge.currentVehicles++;
        }
        if (QRandomGenerator::global()->generateDouble() > 0.98 &&
            !edge.ishappeningaccident()) {
            int from = edge.toVertex;
            int to = edge.fromVertex;
            edge.vehiclesets.push_back(Vehicle(edge.id, from, to, 0.0));
            edge.currentVehicles++;
        }
    }
}
