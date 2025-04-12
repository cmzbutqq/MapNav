
#ifndef CORE_H
#define CORE_H
#include <random>
#include <QPointF>
#include <QVector>
#include <QSet>
#include <cmath>


// =====前置声明=====
struct Vertex;
struct Edge;
struct Vehicle;
struct GridCell;

// =====顶点类=====
struct Vertex {
    int id;
    QPointF position;
    QSet<int> connectedEdges;//与其相连的边的id
    QSet<int> connectedVertexs;//与其相连的点的id
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
    QVector<Vehicle> vehiclesets;//在边中包含车辆类，作为类的属性


    Edge(int id , int from,int to, double len );
    double getCurrentTravelTime() ;
    bool ishappeningaccident();

    // 生成指定范围 [a, b] 内的随机数
    double randomInRange(double a, double b) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(a, b);
        return dis(gen);
    }
};

// =====车辆类以及实现=====
struct Vehicle {

    int currentEdge;
    double progress;
    int from ;
    int to;
    bool processed;//专门用于模拟类中防止一辆车在update中被多次更新
    Vehicle( int edge ,int fromv,int tov,double pro):currentEdge(edge),progress(pro) , from(fromv), to(tov)
    {};

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

    const Vertex* getVertex(int id) const;
    const Edge* getEdge(int id) ;
    const GridCell* getGridCell(int x, int y) ;


    // 生成指定范围 [a, b] 内的随机数
    double randomInRange(double a, double b) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(a, b);
        return dis(gen);
    }

//我直接声明为公有的了，私有太麻烦了
    static const int GRID_DIM = 15;
    int Vertexcount;
    int Edgecount;
    int gridSize;
    QVector<Vertex> vertices;
    QVector<Edge> edges;
    GridCell grid[GRID_DIM][GRID_DIM];

};

// =====模拟器类=====
class Simulator
{
public:
    Simulator(Map* map);

    void update(double deltaTime);

private:

    Map* m_map;

};
#endif // CORE_H
