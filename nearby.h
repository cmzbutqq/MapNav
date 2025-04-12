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
    void highlightNearby(const QPointF& center);//根据给定的中心点 center，找出附近的顶点和边，并将它们标记为高亮显示。
    bool isVertexHighlighted(int vertexId) const;
    //分别用于检查指定的顶点和边是否处于高亮状态。
    bool isEdgeHighlighted(int edgeId) const;

signals:
    void updateRequested();//当高亮状态改变时，发出该信号通知视图进行更新。

private slots:
    void resetHighlight();//重置高亮状态：用于在一定时间后将所有高亮的顶点和边重置为非高亮状态。

private:
    Map* m_map;
    QTimer m_resetTimer;
    QSet<int> m_highlightedVertices;
    QSet<int> m_highlightedEdges;
/*m_resetTimer：QTimer 对象，用于定时重置高亮状态。
m_highlightedVertices：QSet 容器，存储高亮显示的顶点的 ID。
m_highlightedEdges：QSet 容器，存储高亮显示的边的 ID。*/
};

#endif // NEARBY_H
