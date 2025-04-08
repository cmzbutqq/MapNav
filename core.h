#ifndef CORE_H
#define CORE_H

#include <QPointF>
#include <QVector>
#include <QSet>
#include <cmath>

// =====常量定义=====
constexpr int DEFAULT_GRID_SIZE = 100;
constexpr double MAX_EDGE_LENGTH = 200.0;
constexpr int MAX_VEHICLE_CAPACITY = 50;
constexpr int MAX_VERTICES = 20000;
constexpr int MAX_EDGES = 100000;
constexpr int MAX_VEHICLES = 5000;

// =====前置声明=====
struct Vertex;
struct Edge;
struct Vehicle;
struct GridCell;

// =====顶点类=====
struct Vertex {
    int id;
    QPointF position;
    QSet<int> connectedEdges;

    Vertex(int id = -1, const QPointF& pos = QPointF());
    void addEdge(int edgeId);
};

// =====边类=====
struct Edge {
    int id;
    int fromVertex;
    int toVertex;
    double length;
    int capacity;
    int currentVehicles;

    Edge(int id = -1, int from = -1, int to = -1, double len = 0.0);
    double getCurrentTravelTime() const;
    void vehicleEnter();
    void vehicleLeave();
};

// =====车辆类=====
struct Vehicle {
    int id;
    int currentEdge;
    double progress;

    Vehicle(int id = -1, int edge = -1, double prog = 0.0);
};

// =====网格单元类=====
struct GridCell {
    QSet<int> vertices;
    void addVertex(int vertexId);
};

// =====地图类=====
class Map {
public:
    Map();
    int addVertex(const QPointF& pos);
    int addEdge(int from, int to);
    const Vertex* getVertex(int id) const;
    const Edge* getEdge(int id) const;
    const GridCell* getGridCell(int x, int y) const;
    int getGridSize() const;
    int getVertexCount() const;
    int getEdgeCount() const;
    void generateRandomGraph(int vertexCount);

private:
    static const int GRID_DIM = 1000 / DEFAULT_GRID_SIZE + 1;
    int nextVertexId;
    int nextEdgeId;
    int gridSize;
    QVector<Vertex> vertices;
    QVector<Edge> edges;
    GridCell grid[GRID_DIM][GRID_DIM];
};

// =====模拟器类=====
class Simulator {
public:
    Simulator();
    int addVehicle(int edgeId = -1);
    void update(double deltaTime);

private:
    int nextVehicleId;
    QVector<Vehicle> vehicles;
};

#endif // CORE_H
