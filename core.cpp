#include <QPointF>
#include <QVector>
#include <QSet>
#include <QRandomGenerator>
#include <cmath>
#include <algorithm>

// =====常量定义=====
constexpr int DEFAULT_GRID_SIZE = 100;
constexpr double MAX_EDGE_LENGTH = 200.0; // 最大边长度限制
constexpr int MAX_VEHICLE_CAPACITY = 50;  // 道路最大车容量
constexpr int MAX_VERTICES = 20000;       // 最大顶点数
constexpr int MAX_EDGES = 100000;         // 最大边数
constexpr int MAX_VEHICLES = 5000;        // 最大车辆数

// =====核心数据结构=====

// -----顶点类-----
struct Vertex {
    int id;                 // 顶点唯一ID
    QPointF position;       // 二维坐标位置
    QSet<int> connectedEdges; // 连接的边ID集合

    Vertex(int id = -1, const QPointF& pos = QPointF())
        : id(id), position(pos) {}

    void addEdge(int edgeId) { connectedEdges.insert(edgeId); }
};

// -----边类-----
struct Edge {
    int id;             // 边唯一ID
    int fromVertex;     // 起点ID
    int toVertex;       // 终点ID
    double length;      // 道路长度
    int capacity;       // 车容量
    int currentVehicles;// 当前车辆数

    Edge(int id = -1, int from = -1, int to = -1, double len = 0.0)
        : id(id), fromVertex(from), toVertex(to),
        length(len), capacity(MAX_VEHICLE_CAPACITY),
        currentVehicles(0) {}

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
};

// -----车辆类-----
struct Vehicle {
    int id;             // 车辆唯一ID
    int currentEdge;    // 当前所在道路ID
    double progress;    // 在道路上的进度(0.0-1.0)

    Vehicle(int id = -1, int edge = -1, double prog = 0.0)
        : id(id), currentEdge(edge), progress(prog) {}
};

// -----网格单元类-----
struct GridCell {
    QSet<int> vertices; // 该网格内的顶点ID集合

    void addVertex(int vertexId) { vertices.insert(vertexId); }
};

// -----地图类-----
class Map {
public:
    Map() : nextVertexId(0), nextEdgeId(0), gridSize(DEFAULT_GRID_SIZE) {
        vertices.resize(MAX_VERTICES);
        edges.resize(MAX_EDGES);
    }

    // 添加顶点
    int addVertex(const QPointF& pos) {
        if (nextVertexId >= MAX_VERTICES) return -1;

        int id = nextVertexId++;
        vertices[id] = Vertex(id, pos);

        // 添加到网格
        int gridX = static_cast<int>(pos.x() / gridSize);
        int gridY = static_cast<int>(pos.y() / gridSize);
        grid[gridX][gridY].addVertex(id);

        return id;
    }

    // 添加边
    int addEdge(int from, int to) {
        if (from < 0 || from >= nextVertexId || to < 0 || to >= nextVertexId)
            return -1;

        if (nextEdgeId >= MAX_EDGES) return -1;

        QPointF fromPos = vertices[from].position;
        QPointF toPos = vertices[to].position;
        double length = sqrt(pow(fromPos.x() - toPos.x(), 2) +
                             pow(fromPos.y() - toPos.y(), 2));

        if (length > MAX_EDGE_LENGTH) return -1;

        int id = nextEdgeId++;
        edges[id] = Edge(id, from, to, length);

        // 更新顶点连接信息
        vertices[from].addEdge(id);
        vertices[to].addEdge(id);

        return id;
    }

    // 获取顶点
    const Vertex* getVertex(int id) const {
        if (id < 0 || id >= nextVertexId) return nullptr;
        return &vertices[id];
    }

    // 获取边
    const Edge* getEdge(int id) const {
        if (id < 0 || id >= nextEdgeId) return nullptr;
        return &edges[id];
    }

    // 获取网格单元
    const GridCell* getGridCell(int x, int y) const {
        if (x < 0 || y < 0 || x >= GRID_DIM || y >= GRID_DIM)
            return nullptr;
        return &grid[x][y];
    }

    // 获取网格大小
    int getGridSize() const { return gridSize; }

    // 获取顶点数量
    int getVertexCount() const { return nextVertexId; }

    // 获取边数量
    int getEdgeCount() const { return nextEdgeId; }

    // 生成随机连通图
    void generateRandomGraph(int vertexCount) {
        if (vertexCount <= 0 || vertexCount > MAX_VERTICES) return;

        // 生成顶点
        for (int i = 0; i < vertexCount; ++i) {
            double x = QRandomGenerator::global()->bounded(1000.0);
            double y = QRandomGenerator::global()->bounded(1000.0);
            addVertex(QPointF(x, y));
        }

        // 连接顶点形成连通图
        for (int i = 0; i < vertexCount; ++i) {
            QVector<QPair<double, int>> nearby;
            QPointF pos = vertices[i].position;

            for (int j = 0; j < vertexCount; ++j) {
                if (i == j) continue;

                double dist = sqrt(pow(pos.x() - vertices[j].position.x(), 2) +
                                   pow(pos.y() - vertices[j].position.y(), 2));
                nearby.append(qMakePair(dist, j));
            }

            // 按距离排序
            std::sort(nearby.begin(), nearby.end());

            // 连接到最近的3-5个顶点
            int connections = QRandomGenerator::global()->bounded(3, 6);
            for (int k = 0; k < qMin(connections, nearby.size()); ++k) {
                addEdge(i, nearby[k].second);
            }
        }
    }

private:
    static const int GRID_DIM = 1000 / DEFAULT_GRID_SIZE + 1;

    int nextVertexId;   // 下一个顶点ID
    int nextEdgeId;      // 下一条边ID
    int gridSize;        // 网格大小

    QVector<Vertex> vertices;    // 顶点数组
    QVector<Edge> edges;         // 边数组
    GridCell grid[GRID_DIM][GRID_DIM]; // 网格系统
};

// =====模拟器类=====
class Simulator {
public:
    Simulator() : nextVehicleId(0) {
        vehicles.resize(MAX_VEHICLES);
    }

    // 添加车辆
    int addVehicle(int edgeId = -1) {
        if (nextVehicleId >= MAX_VEHICLES) return -1;

        int id = nextVehicleId++;
        vehicles[id] = Vehicle(id, edgeId);
        return id;
    }

    // 更新模拟
    void update(double deltaTime) {
        // 更新所有车辆位置
        for (int i = 0; i < nextVehicleId; ++i) {
            if (vehicles[i].currentEdge != -1) {
                // 简单模拟车辆移动
                vehicles[i].progress += deltaTime * 0.01;
                if (vehicles[i].progress >= 1.0) {
                    vehicles[i].progress = 0.0;
                    vehicles[i].currentEdge = -1; // 到达终点
                }
            }
        }
    }

private:
    int nextVehicleId;   // 下一个车辆ID
    QVector<Vehicle> vehicles; // 车辆数组
};

// =====Dummy函数=====
void dummyPathfinding() {}
void dummySearch() {}
void dummyCamera() {}
void dummyGeneration() {}
