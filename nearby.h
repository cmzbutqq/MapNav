#ifndef NEARBY_H
#define NEARBY_H

#include "core.h"
#include <QObject>
#include <QTimer>
#include <QSet>

class NearbyHighlighter : public QObject
{
    Q_OBJECT

public:
    explicit NearbyHighlighter(Map* map, QObject* parent = nullptr);
    void highlightNearby(const QPointF& center);
    bool isVertexHighlighted(int vertexId) const;
    bool isEdgeHighlighted(int edgeId) const;

signals:
    void updateRequested();

private slots:
    void resetHighlight();

private:
    Map* m_map;
    QTimer m_resetTimer;
    QSet<int> m_highlightedVertices;
    QSet<int> m_highlightedEdges;
};

#endif // NEARBY_H
