#ifndef CORE_H
#define CORE_H

#include <QPointF>
#include <QVector>
#include <QHash>
#include <QRandomGenerator>
#include <QSet>
#include <cmath>
#include <QPair>

// =====常量定义=====
constexpr int DEFAULT_GRID_SIZE = 100;
constexpr double MAX_EDGE_LENGTH = 200.0;
constexpr int MAX_VEHICLE_CAPACITY = 50;

// =====核心数据结构=====
class Vertex {
public:
    Vertex(int id = -1, const QPointF& pos = QPointF());
    int getId() const;
    QPointF getPosition() const;
    void addEdge(int edgeId);
    const QSet<int>& getConnectedEdges() const;

private:
    int id;
    QPointF position;
    QSet<int> connectedEdges;
};

class Edge {
public:
    Edge(int id = -1, int from = -1, int to = -1, double length = 0.0);
    int getId() const;
    int getFromVertex() const;
    int getToVertex() const;
    double getLength() const;
    int getCapacity() const;
    int getCurrentVehicles() const;
    double getCurrentTravelTime() const;
    void vehicleEnter();
    void vehicleLeave();

private:
    int id;
    int fromVertex;
    int toVertex;
    double length;
    int capacity;
    int currentVehicles;
};

class Vehicle {
public:
    Vehicle(int id = -1, int currentEdge = -1, double progress = 0.0);
    int getId() const;
    int getCurrentEdge() const;
    double getProgress() const;
    void setCurrentEdge(int edgeId);
    void setProgress(double prog);

private:
    int id;
    int currentEdge;
    double progress;
};

class GridCell {
public:
    GridCell();
    void addVertex(int vertexId);
    const QSet<int>& getVertices() const;

private:
    QSet<int> vertices;
};

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
    int nextVertexId;
    int nextEdgeId;
    int gridSize;
    QHash<int, Vertex> vertices;
    QHash<int, Edge> edges;
    QHash<QPair<int, int>, GridCell> grid;
};

class Simulator {
public:
    Simulator();
    int addVehicle(int edgeId = -1);
    void update(double deltaTime);

private:
    int nextVehicleId;
    QHash<int, Vehicle> vehicles;
};

#endif // CORE_H
