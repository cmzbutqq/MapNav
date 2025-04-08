#include <QPointF>
#include <QVector>
#include <QHash>
#include <QRandomGenerator>
#include <QSet>
#include <cmath>

// =====常量定义=====
constexpr int DEFAULT_GRID_SIZE = 100;
constexpr double MAX_EDGE_LENGTH = 200.0; // 最大边长度限制
constexpr int MAX_VEHICLE_CAPACITY = 50;  // 道路最大车容量

// =====核心数据结构=====

// -----顶点类-----
class Vertex {
public:
    Vertex(int id, const QPointF& pos) : id(id), position(pos) {}

    int getId() const { return id; }
    QPointF getPosition() const { return position; }

    void addEdge(int edgeId) { connectedEdges.insert(edgeId); }
    const QSet<int>& getConnectedEdges() const { return connectedEdges; }

private:
    int id;                 // 顶点唯一ID
    QPointF position;       // 二维坐标位置
    QSet<int> connectedEdges; // 连接的边ID集合
};

// -----边类-----
class Edge {
public:
    Edge(int id, int from, int to, double length)
        : id(id), fromVertex(from), toVertex(to),
        length(length), capacity(MAX_VEHICLE_CAPACITY),
        currentVehicles(0) {}

    int getId() const { return id; }
    int getFromVertex() const { return fromVertex; }
    int getToVertex() const { return toVertex; }
    double getLength() const { return length; }
    int getCapacity() const { return capacity; }
    int getCurrentVehicles() const { return currentVehicles; }

    // 计算当前通行时间
    double getCurrentTravelTime() const {
        const double c = 1.0; // 常数
        const double threshold = 0.8; // 阈值
        double ratio = static_cast<double>(currentVehicles) / capacity;

        if (ratio <= threshold) {
            return c * length;
        } else {
            return c * length * (1 + exp(ratio - threshold));
        }
    }

    // 车辆进入道路
    void vehicleEnter() { currentVehicles++; }

    // 车辆离开道路
    void vehicleLeave() {
        if (currentVehicles > 0) currentVehicles--;
    }

private:
    int id;             // 边唯一ID
    int fromVertex;     // 起点ID
    int toVertex;       // 终点ID
    double length;      // 道路长度
    int capacity;       // 车容量
    int currentVehicles;// 当前车辆数
};

// -----车辆类-----
class Vehicle {
public:
    Vehicle(int id, int currentEdge = -1, double progress = 0.0)
        : id(id), currentEdge(currentEdge), progress(progress) {}

    int getId() const { return id; }
    int getCurrentEdge() const { return currentEdge; }
    double getProgress() const { return progress; }

    void setCurrentEdge(int edgeId) { currentEdge = edgeId; }
    void setProgress(double prog) { progress = prog; }

private:
    int id;             // 车辆唯一ID
    int currentEdge;    // 当前所在道路ID
    double progress;    // 在道路上的进度(0.0-1.0)
};

// -----网格单元类-----
class GridCell {
public:
    GridCell() = default;

    void addVertex(int vertexId) { vertices.insert(vertexId); }
    const QSet<int>& getVertices() const { return vertices; }

private:
    QSet<int> vertices; // 该网格内的顶点ID集合
};

// -----地图类-----
class Map {
public:
    Map() : nextVertexId(0), nextEdgeId(0), gridSize(DEFAULT_GRID_SIZE) {}

    // 添加顶点
    int addVertex(const QPointF& pos) {
        int id = nextVertexId++;
        vertices.insert(id, Vertex(id, pos));

        // 添加到网格
        int gridX = static_cast<int>(pos.x() / gridSize);
        int gridY = static_cast<int>(pos.y() / gridSize);
        grid[QPair<int, int>(gridX, gridY)].addVertex(id);

        return id;
    }

    // 添加边
    int addEdge(int from, int to) {
        if (!vertices.contains(from) || !vertices.contains(to)) return -1;

        QPointF fromPos = vertices[from].getPosition();
        QPointF toPos = vertices[to].getPosition();
        double length = sqrt(pow(fromPos.x() - toPos.x(), 2) +
                             pow(fromPos.y() - toPos.y(), 2));

        if (length > MAX_EDGE_LENGTH) return -1;

        int id = nextEdgeId++;
        edges.insert(id, Edge(id, from, to, length));

        // 更新顶点连接信息
        vertices[from].addEdge(id);
        vertices[to].addEdge(id);

        return id;
    }

    // 获取顶点方法
    const Vertex* getVertex(int id) const {
        auto it = vertices.constFind(id);
        return (it != vertices.constEnd()) ? &it.value() : nullptr;
    }

    // 获取边方法
    const Edge* getEdge(int id) const {
        auto it = edges.constFind(id);
        return (it != edges.constEnd()) ? &it.value() : nullptr;
    }

    // 获取网格单元方法
    const GridCell* getGridCell(int x, int y) const {
        auto key = QPair<int, int>(x, y);
        auto it = grid.constFind(key);
        return (it != grid.constEnd()) ? &it.value() : nullptr;
    }

    // 获取网格大小
    int getGridSize() const { return gridSize; }

    // 获取顶点数量
    int getVertexCount() const { return vertices.size(); }

    // 获取边数量
    int getEdgeCount() const { return edges.size(); }

    // 生成随机连通图(占位函数)
    void generateRandomGraph(int vertexCount) {
        // !!注意事项!!
        // 这里只是占位，实际实现需要考虑连通性和避免交叉
        for (int i = 0; i < vertexCount; ++i) {
            double x = QRandomGenerator::global()->bounded(1000.0);
            double y = QRandomGenerator::global()->bounded(1000.0);
            addVertex(QPointF(x, y));
        }

        // 简单连接每个顶点到最近的几个顶点
        for (const auto& v : vertices) {
            QVector<QPair<double, int>> nearby;
            QPointF pos = v.getPosition();

            for (const auto& other : vertices) {
                if (other.getId() == v.getId()) continue;

                double dist = sqrt(pow(pos.x() - other.getPosition().x(), 2) +
                                   pow(pos.y() - other.getPosition().y(), 2));
                nearby.append(qMakePair(dist, other.getId()));
            }

            // 按距离排序
            std::sort(nearby.begin(), nearby.end());

            // 连接到最近的3-5个顶点
            int connections = QRandomGenerator::global()->bounded(3, 6);
            for (int i = 0; i < qMin(connections, nearby.size()); ++i) {
                addEdge(v.getId(), nearby[i].second);
            }
        }
    }

private:
    int nextVertexId;   // 下一个顶点ID
    int nextEdgeId;      // 下一条边ID
    int gridSize;        // 网格大小

    QHash<int, Vertex> vertices;    // 顶点集合
    QHash<int, Edge> edges;         // 边集合
    QHash<QPair<int, int>, GridCell> grid; // 网格系统
};

// =====模拟器类=====
class Simulator {
public:
    Simulator() : nextVehicleId(0) {}

    // 添加车辆
    int addVehicle(int edgeId = -1) {
        int id = nextVehicleId++;
        vehicles.insert(id, Vehicle(id, edgeId));

        if (edgeId != -1) {
            // 更新道路车辆计数
            // !!注意事项!! 这里需要确保边存在
        }

        return id;
    }

    // 更新模拟(占位函数)
    void update(double deltaTime) {
        // 更新所有车辆位置
        for (auto& vehicle : vehicles) {
            // 简单模拟车辆移动
            // 实际实现需要考虑路径规划和交通状况
        }
    }

private:
    int nextVehicleId;   // 下一个车辆ID
    QHash<int, Vehicle> vehicles; // 车辆集合
};

// =====Dummy函数=====
void dummyPathfinding() {}
void dummySearch() {}
void dummyCamera() {}
void dummyGeneration() {}
