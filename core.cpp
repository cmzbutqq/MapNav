#include "core.h"
#include <QRandomGenerator>
#include <algorithm>
#include <vector>
#include <set>

// 辅助函数：判断点 p 是否在由 a, b, c 构成的三角形的外接圆内
bool isPointInCircumcircle(const QPointF& p, const QPointF& a, const QPointF& b, const QPointF& c) {
    double ax = a.x() - p.x();
    double ay = a.y() - p.y();
    double bx = b.x() - p.x();
    double by = b.y() - p.y();
    double cx = c.x() - p.x();
    double cy = c.y() - p.y();

    double det = ax * (by * (cx * cx + cy * cy) - cy * (bx * bx + by * by)) -
                 ay * (bx * (cx * cx + cy * cy) - cx * (bx * bx + by * by)) +
                 (ax * by - ay * bx) * (cx * bx + cy * by);

    return det > 0;
}

// =====辅助结构体：三角形=====
struct Triangle {
    int a, b, c;

    Triangle(int a, int b, int c) : a(a), b(b), c(c) {}

    // 添加 operator== 重载
    bool operator==(const Triangle& other) const {
        return a == other.a && b == other.b && c == other.c;
    }
};


// =====顶点类实现=====
Vertex::Vertex(int id, const QPointF& pos) : id(id), position(pos) {}

void Vertex::addEdge(int edgeId) {
    connectedEdges.insert(edgeId);
}

// =====边类实现=====
Edge::Edge(int id, int from, int to, double len) :
    id(id), fromVertex(from), toVertex(to), length(len)
{
    capacity = length * 0.5 + 7;

    int carcount = int(randomInRange(0, capacity + 10));
    currentVehicles = carcount;
    for (int i = 0; i < carcount; i++) {
        double direction = randomInRange(0, 1);
        if (direction > 0.5) {
            vehiclesets.push_back(Vehicle(id, from, to, randomInRange(0, 1)));
        }
        else {
            vehiclesets.push_back(Vehicle(id, to, from, randomInRange(0, 1)));
        }
    }
}

double Edge::getCurrentTravelTime() {
    const double c = 1.0;
    const double threshold = 0.8;
    double ratio = double(vehiclesets.count()) / double(capacity);

    if (ishappeningaccident()) {
        return randomInRange(3000,6000 );
    }

    if (ratio <= threshold) {
        return c * length;
    }
    return c * length * (1 + std::exp(ratio - threshold));
}

bool Edge::ishappeningaccident()
{
    if (vehiclesets.count() > capacity + 20) {
        return (randomInRange(0, 1) > 0.95);
    }
    return 0;
}

// =====网格单元类实现=====
void GridCell::addVertex(int vertexId) {
    vertices.insert(vertexId);
}

// =====地图类实现=====
Map::Map() {
    /*
构造函数中随机生成10000到20000个点，然后使用Delaunay 三角剖分生成边，然后利用lambda 函数遍历产生的三角形集合将所有的边加入到Map
的成员变量QVector<Edge> edges中;在定义边Edge的时候虽然有from 和to，但是在加入到 edges时，按照无向图的方法加入，所以每条边只会
加入一次，并且每个结点的QSet<int> connectedEdges能够正确更新与它相连接的边的id，通过edges可以访问每一条边，通过vertices可以访问
每个节点
    */
    Vertexcount = int(randomInRange(10000, 20000));
    gridSize = GRID_DIM;

    // 随机生成顶点
    for (int i = 0; i < Vertexcount; i++) {
        double x = randomInRange(0, 1000);
        double y = randomInRange(0, 1000);
        vertices.push_back(Vertex(i, QPointF(x, y)));
    }
    //Delaunay 三角剖分保证图的连通性，并且任意两条边之间没有交叉
    // 初始化超级三角形
    double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
    for (const auto& vertex : vertices) {
        minX = std::min(minX, vertex.position.x());
        minY = std::min(minY, vertex.position.y());
        maxX = std::max(maxX, vertex.position.x());
        maxY = std::max(maxY, vertex.position.y());
    }

    double dx = maxX - minX;
    double dy = maxY - minY;
    double deltaMax = std::max(dx, dy);
    double midX = (minX + maxX) / 2;
    double midY = (minY + maxY) / 2;

    QPointF p1(midX - 20 * deltaMax, midY - deltaMax);
    QPointF p2(midX, midY + 20 * deltaMax);
    QPointF p3(midX + 20 * deltaMax, midY - deltaMax);

    vertices.push_back(Vertex(Vertexcount, p1));
    vertices.push_back(Vertex(Vertexcount + 1, p2));
    vertices.push_back(Vertex(Vertexcount + 2, p3));

    std::vector<Triangle> triangulation;
    triangulation.emplace_back(Vertexcount, Vertexcount + 1, Vertexcount + 2);

    // Bowyer-Watson ，算法平均情况下为O(nlogn)
    for (int i = 0; i < Vertexcount; i++) {
        std::vector<Triangle> badTriangles;
        for (const auto& triangle : triangulation) {
            if (isPointInCircumcircle(vertices[i].position, vertices[triangle.a].position, vertices[triangle.b].position, vertices[triangle.c].position)) {
                badTriangles.push_back(triangle);
            }
        }

        std::vector<std::pair<int, int>> polygon;
        for (const auto& triangle : badTriangles) {
            for (int j = 0; j < 3; j++) {
                int a = triangle.a;
                int b = triangle.b;
                int c = triangle.c;
                if (j == 1) {
                    std::swap(a, b);
                }
                else if (j == 2) {
                    std::swap(a, c);
                }

                bool isShared = false;
                for (const auto& otherTriangle : badTriangles) {
                    if (triangle.a == otherTriangle.a && triangle.b == otherTriangle.b && triangle.c == otherTriangle.c) {
                        continue;
                    }
                    if ((a == otherTriangle.a && b == otherTriangle.b) || (a == otherTriangle.b && b == otherTriangle.c) || (a == otherTriangle.c && b == otherTriangle.a)) {
                        isShared = true;
                        break;
                    }
                }

                if (!isShared) {
                    polygon.emplace_back(a, b);
                }
            }
        }

        for (const auto& triangle : badTriangles) {
            triangulation.erase(std::remove(triangulation.begin(), triangulation.end(), triangle), triangulation.end());
        }

        for (const auto& edge : polygon) {
            triangulation.emplace_back(edge.first, edge.second, i);
        }
    }

    // 移除包含超级三角形顶点的三角形
    triangulation.erase(std::remove_if(triangulation.begin(), triangulation.end(), [this](const Triangle& triangle) {
                            return triangle.a >= Vertexcount || triangle.b >= Vertexcount || triangle.c >= Vertexcount;
                        }), triangulation.end());

    // 遍历triangulation，现在triangulation存储了当前已经构建好的所有三角形。
    //利用集合，每条边只会被添加一次
    Edgecount = 0;
    std::set<std::pair<int, int>> addedEdges;//这个集合将记录已经添加到图中的边
    for (const auto& triangle : triangulation) {
        auto addEdge = [this, &addedEdges](int from, int to) {
            // 确保边的顺序一致，避免重复
            std::pair<int, int> edge = std::make_pair(std::min(from, to), std::max(from, to));
            if (addedEdges.find(edge) == addedEdges.end()) {
                QPointF p1 = vertices[from].position;
                QPointF p2 = vertices[to].position;
                double len = std::sqrt((p1.x() - p2.x()) * (p1.x() - p2.x()) + (p1.y() - p2.y()) * (p1.y() - p2.y()));
                edges.push_back(Edge(Edgecount++, from, to, len));
                vertices[from].addEdge(Edgecount - 1);
                vertices[to].addEdge(Edgecount - 1);
                addedEdges.insert(edge);
            }
        };
        /*
auto addEdge = [this, &addedEdges](int from, int to) {... };：定义一个名为 addEdge 的 lambda 函数。
[this, &addedEdges] 是捕获列表，this 捕获当前对象的指针，以便在 lambda 函数中访问类的成员（如 vertices 和 edges），
 &addedEdges 以引用的方式捕获 addedEdges 集合，这样在 lambda 函数中可以修改它。int from 和 int to 是函数的参数，表示边的两个端点的索引。
std::pair<int, int> edge = std::make_pair(std::min(from, to), std::max(from, to));：为了确保无论边的两个端点以何种顺序传入，都能得到一致的表示，
将较小的索引作为 pair 的第一个元素，较大的作为第二个元素。这样可以避免将 (1, 2) 和 (2, 1) 视为不同的边。
if (addedEdges.find(edge) == addedEdges.end()) {... }：使用 addedEdges.find(edge) 在已添加边的集合中查找当前边 edge。
如果 find 函数返回 addedEdges.end()，说明当前边尚未添加过，进入 if 块。
QPointF p1 = vertices[from].position; 和 QPointF p2 = vertices[to].position;：获取边的两个端点的坐标。
double len = std::sqrt((p1.x() - p2.x()) * (p1.x() - p2.x()) + (p1.y() - p2.y()) * (p1.y() - p2.y()))计算边的长度。
edges.push_back(Edge(Edgecount++, from, to, len));：创建一个新的 Edge 对象，包含边的 ID（Edgecount）、起点 from、终点 to 和长度 len，并将其添加到 edges 容器中。
vertices[from].addEdge(Edgecount - 1); 和 vertices[to].addEdge(Edgecount - 1);：将当前边的 ID 添加到边的两个端点对应的顶点对象的 connectedEdges 集合中，
addedEdges.insert(edge);：将当前边添加到 addedEdges 集合中，以便后续检查边是否已经添加过。
         */
        addEdge(triangle.a, triangle.b);
        addEdge(triangle.b, triangle.c);
        addEdge(triangle.c, triangle.a);
    }
    // 移除超级三角形的顶点
    vertices.resize(Vertexcount);//vertices 中元素的顺序与最初随机生成顶点时的顺序保持一致，并且每个顶点的连接信息都已更新，构成了一个通过 Delaunay 三角剖分得到的连通图。
    // 遍历 edges 中的每一条边，对于每条边，将其终点的 id 插入到起点的 connectedVertexs 集合中，同时将起点的 id 插入到终点的 connectedVertexs 集合中。
    for (const auto& edge : edges) {
        vertices[edge.fromVertex].connectedVertexs.insert(edge.toVertex);
        vertices[edge.toVertex].connectedVertexs.insert(edge.fromVertex);
    }
    Edgecount=edges.count();
}


// =====模拟器类实现=====
Simulator::Simulator(Map* map) : m_map(map) {}

void Simulator::update(double deltaTime) {
    // 先将所有车辆的 processed 标记置为 false
    for (auto& edge : m_map->edges) {
        for (auto& vehicle : edge.vehiclesets) {
            vehicle.processed = false;
        }
    }

    // 遍历每条边
    for (auto& edge : m_map->edges) {
        if(edge.ishappeningaccident()){
            continue;
        }

        double travelTime = edge.getCurrentTravelTime();

        // 遍历当前边上的每辆车
        auto it = edge.vehiclesets.begin();
        while (it != edge.vehiclesets.end()) {
            Vehicle& vehicle = *it;

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
                Vertex& vertex = m_map->vertices[currentVertex];
                std::vector<int> connectedEdges; // 当前可用的边的向量，不包含发生事故的边
                std::vector<double> probabilities;
                double totalWeight = 0.0;

                for (int edgeId : vertex.connectedEdges) {
                    // 直接访问 edges 向量获取边
                    Edge& nextEdge = m_map->edges[edgeId];
                    if(nextEdge.ishappeningaccident()){
                        continue;
                    }
                    double weight = 1.0 / (nextEdge.getCurrentTravelTime() + 1e-6); // 反比于通行时间
                    totalWeight += weight;
                    connectedEdges.push_back(edgeId);
                    probabilities.push_back(weight);
                }

                // 归一化概率
                for (double& prob : probabilities) {
                    prob /= totalWeight;
                }

                // 根据概率选择下一条边
                double randomValue = QRandomGenerator::global()->generateDouble();
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
                    Edge& nextEdge = m_map->edges[nextEdgeId];
                    int nextFrom = currentVertex;
                    int nextTo = (nextEdge.fromVertex == currentVertex) ? nextEdge.toVertex : nextEdge.fromVertex;

                    // 将车辆移动到下一条边
                    double newProgress = (vehicle.progress - 1.0) * travelTime / nextEdge.getCurrentTravelTime(); // 计算 progress
                    Vehicle newVehicle(nextEdgeId, nextFrom, nextTo, newProgress);
                    newVehicle.processed = 1;//这里设为1就可以防止重复更新一辆车
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
        if (QRandomGenerator::global()->generateDouble() < 0.02 &&!edge.ishappeningaccident()) {
            int from = edge.fromVertex;
            int to = edge.toVertex;
            edge.vehiclesets.push_back(Vehicle(edge.id, from, to, 0.0));
            edge.currentVehicles++;
        }
        if (QRandomGenerator::global()->generateDouble() > 0.98 &&!edge.ishappeningaccident()) {
            int from = edge.toVertex;
            int to = edge.fromVertex;
            edge.vehiclesets.push_back(Vehicle(edge.id, from, to, 0.0));
            edge.currentVehicles++;
        }
    }
}
