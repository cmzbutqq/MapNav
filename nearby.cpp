#include "nearby.h"
#include "core.h"
#include <QDebug>
#include <QPair>
#include <algorithm>

NearbyHighlighter::NearbyHighlighter(Map *map, QObject *parent)
    : QObject(parent), m_map(map) {
    m_resetTimer.setSingleShot(true);
    connect(&m_resetTimer, &QTimer::timeout, this,
            &NearbyHighlighter::resetHighlight);
}

void NearbyHighlighter::highlightNearby(const QPointF &center) {
    if (!m_map)
        return;

    // 清空之前的高亮
    m_highlightedVertices.clear();
    m_highlightedEdges.clear();

    // 收集所有顶点及其距离
    QVector<QPair<double, int>> verticesWithDistance;
    for (int i = 0; i < m_map->Vertexcount; ++i) {
        const Vertex *vertex = m_map->getVertex(i);
        if (!vertex)
            continue;

        double dx = vertex->position.x() - center.x();
        double dy = vertex->position.y() - center.y();
        double distance = sqrt(dx * dx + dy * dy);
        verticesWithDistance.append(qMakePair(distance, i));
    }

    // 按距离排序并取最近的100个
    std::sort(verticesWithDistance.begin(), verticesWithDistance.end());
    int count = qMin(100, verticesWithDistance.size());

    // 记录高亮的顶点和相关边
    for (int i = 0; i < count; ++i) {
        int vertexId = verticesWithDistance[i].second;
        m_highlightedVertices.insert(vertexId);

        const Vertex *vertex = m_map->getVertex(vertexId);
        if (!vertex)
            continue;

        // 添加所有相连的边
        for (int edgeId : vertex->connectedEdges) {
            m_highlightedEdges.insert(edgeId);
        }
    }

    // 2秒后重置高亮
    m_resetTimer.start(2000);
    emit updateRequested();
}

void NearbyHighlighter::resetHighlight() {
    m_highlightedVertices.clear();
    m_highlightedEdges.clear();
    emit updateRequested();
}
bool NearbyHighlighter::isVertexHighlighted(int vertexId) const {
    return m_highlightedVertices.contains(vertexId);
}

bool NearbyHighlighter::isEdgeHighlighted(int edgeId) const {
    return m_highlightedEdges.contains(edgeId);
}
