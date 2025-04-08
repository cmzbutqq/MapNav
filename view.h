#ifndef VIEW_H
#define VIEW_H

#include <QWidget>
#include <QPainter>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QPolygonF>
#include "core.h"

class MapView : public QWidget
{
    Q_OBJECT

public:
    explicit MapView(QWidget *parent = nullptr);
    void setMap(Map* map);
    void setSimulator(Simulator* simulator);

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    // 视角控制参数
    QPointF m_viewCenter{500, 500};
    double m_zoomLevel = 1.0;
    double m_rotation = 0.0;
    QPoint m_lastMousePos;

    // 数据引用
    Map* m_map = nullptr;
    Simulator* m_simulator = nullptr;

    // 转换函数
    QPointF worldToScreen(const QPointF& worldPos) const;
    QPointF screenToWorld(const QPoint& screenPos) const;
    QTransform getWorldToScreenTransform() const;

    // 渲染函数
    void renderGrid(QPainter& painter);
    void renderVertices(QPainter& painter);
    void renderEdges(QPainter& painter);
    void renderVehicles(QPainter& painter);

    // 视角控制
    void zoom(double factor);
    void pan(const QPointF& delta);
    void rotate(double angle);

    // 可见区域计算
    QPolygonF getVisibleWorldPolygon() const;
    bool isVisibleInView(const QPointF& worldPos) const;
};

#endif // VIEW_H
