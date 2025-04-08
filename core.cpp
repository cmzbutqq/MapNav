#include "core.h"
#include <QRandomGenerator>
#include <algorithm>

// =====顶点类实现=====
Vertex::Vertex(int id, const QPointF& pos) : id(id), position(pos) {}

void Vertex::addEdge(int edgeId) {
    connectedEdges.insert(edgeId);
}

// =====边类实现=====
Edge::Edge(int id, int from, int to, double len) :
    id(id), fromVertex(from), toVertex(to),
    length(len), capacity(MAX_VEHICLE_CAPACITY),
    currentVehicles(0) {}

double Edge::getCurrentTravelTime() const {
    const double c = 1.0;
    const double threshold = 0.8;
    double ratio = static_cast<double>(currentVehicles) / capacity;

    if (ratio <= threshold) {
        return c * length;
    }
    return c * length * (1 + exp(ratio - threshold));
}

void Edge::vehicleEnter() { currentVehicles++; }
void Edge::vehicleLeave() { if (currentVehicles > 0) currentVehicles--; }

// =====车辆类实现=====
Vehicle::Vehicle(int id, int edge, double prog) :
    id(id), currentEdge(edge), progress(prog) {}

// =====网格单元类实现=====
void GridCell::addVertex(int vertexId) {
    vertices.insert(vertexId);
}

// =====地图类实现=====
Map::Map() : nextVertexId(0), nextEdgeId(0), gridSize(DEFAULT_GRID_SIZE) {
    vertices.resize(MAX_VERTICES);
    edges.resize(MAX_EDGES);
}

int Map::addVertex(const QPointF& pos) {
    if (nextVertexId >= MAX_VERTICES) return -1;

    int id = nextVertexId++;
    vertices[id] = Vertex(id, pos);

    int gridX = static_cast<int>(pos.x() / gridSize);
    int gridY = static_cast<int>(pos.y() / gridSize);
    grid[gridX][gridY].addVertex(id);

    return id;
}

int Map::addEdge(int from, int to) {
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

    vertices[from].addEdge(id);
    vertices[to].addEdge(id);

    return id;
}

const Vertex* Map::getVertex(int id) const {
    if (id < 0 || id >= nextVertexId) return nullptr;
    return &vertices[id];
}

const Edge* Map::getEdge(int id) const {
    if (id < 0 || id >= nextEdgeId) return nullptr;
    return &edges[id];
}

const GridCell* Map::getGridCell(int x, int y) const {
    if (x < 0 || y < 0 || x >= GRID_DIM || y >= GRID_DIM)
        return nullptr;
    return &grid[x][y];
}

int Map::getGridSize() const { return gridSize; }
int Map::getVertexCount() const { return nextVertexId; }
int Map::getEdgeCount() const { return nextEdgeId; }

void Map::generateRandomGraph(int vertexCount) {
    if (vertexCount <= 0 || vertexCount > MAX_VERTICES) return;

    for (int i = 0; i < vertexCount; ++i) {
        double x = QRandomGenerator::global()->bounded(1000.0);
        double y = QRandomGenerator::global()->bounded(1000.0);
        addVertex(QPointF(x, y));
    }

    for (int i = 0; i < vertexCount; ++i) {
        QVector<QPair<double, int>> nearby;
        QPointF pos = vertices[i].position;

        for (int j = 0; j < vertexCount; ++j) {
            if (i == j) continue;

            double dist = sqrt(pow(pos.x() - vertices[j].position.x(), 2) +
                               pow(pos.y() - vertices[j].position.y(), 2));
            nearby.append(qMakePair(dist, j));
        }

        std::sort(nearby.begin(), nearby.end());

        int connections = QRandomGenerator::global()->bounded(3, 6);
        for (int k = 0; k < qMin(connections, nearby.size()); ++k) {
            addEdge(i, nearby[k].second);
        }
    }
}

// =====模拟器类实现=====
Simulator::Simulator() : nextVehicleId(0) {
    vehicles.resize(MAX_VEHICLES);
}

int Simulator::addVehicle(int edgeId) {
    if (nextVehicleId >= MAX_VEHICLES) return -1;

    int id = nextVehicleId++;
    vehicles[id] = Vehicle(id, edgeId);
    return id;
}

void Simulator::update(double deltaTime) {
    for (int i = 0; i < nextVehicleId; ++i) {
        if (vehicles[i].currentEdge != -1) {
            vehicles[i].progress += deltaTime * 0.01;
            if (vehicles[i].progress >= 1.0) {
                vehicles[i].progress = 0.0;
                vehicles[i].currentEdge = -1;
            }
        }
    }
}
