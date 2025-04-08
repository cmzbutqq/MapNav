#include "view.h"
#include <cmath>
#include <QDebug>

MapView::MapView(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

void MapView::setMap(Map* map)
{
    m_map = map;
    update();
}

void MapView::setSimulator(Simulator* simulator)
{
    m_simulator = simulator;
    update();
}

void MapView::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if (!m_map) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 设置视图变换
    painter.translate(width() / 2, height() / 2);
    painter.scale(m_zoomLevel, m_zoomLevel);
    painter.rotate(m_rotation * 180 / M_PI);
    painter.translate(-m_viewCenter.x(), -m_viewCenter.y());

    // 渲染顺序：网格 -> 边 -> 顶点 -> 车辆
    renderGrid(painter);
    renderEdges(painter);
    renderVertices(painter);
    renderVehicles(painter);
}

void MapView::wheelEvent(QWheelEvent* event)
{
    double zoomFactor = 1.0 + event->angleDelta().y() * 0.001;
    zoom(zoomFactor);
}

void MapView::keyPressEvent(QKeyEvent* event)
{
    const double panStep = 50.0 / m_zoomLevel;
    const double rotateStep = 0.1;

    switch (event->key()) {
    case Qt::Key_W: pan(QPointF(0, -panStep)); break;
    case Qt::Key_S: pan(QPointF(0, panStep)); break;
    case Qt::Key_A: pan(QPointF(-panStep, 0)); break;
    case Qt::Key_D: pan(QPointF(panStep, 0)); break;
    case Qt::Key_Q: rotate(-rotateStep); break;
    case Qt::Key_E: rotate(rotateStep); break;
    default: QWidget::keyPressEvent(event);
    }
}

void MapView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_lastMousePos = event->pos();
    }
}

void MapView::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        QPoint delta = event->pos() - m_lastMousePos;
        pan(QPointF(-delta.x() / m_zoomLevel, -delta.y() / m_zoomLevel));
        m_lastMousePos = event->pos();
    }
}

// =====坐标转换=====
QTransform MapView::getWorldToScreenTransform() const
{
    QTransform transform;
    transform.translate(width()/2, height()/2);
    transform.scale(m_zoomLevel, m_zoomLevel);
    transform.rotate(m_rotation * 180 / M_PI);
    transform.translate(-m_viewCenter.x(), -m_viewCenter.y());
    return transform;
}
QPointF MapView::worldToScreen(const QPointF& worldPos) const
{
    return getWorldToScreenTransform().map(worldPos);
}
QPointF MapView::screenToWorld(const QPoint& screenPos) const
{
    return getWorldToScreenTransform().inverted().map(QPointF(screenPos));
}
// =====可见性判断=====
QPolygonF MapView::getVisibleWorldPolygon() const
{
    QPolygonF screenPoly;
    screenPoly << QPointF(0, 0)
               << QPointF(width(), 0)
               << QPointF(width(), height())
               << QPointF(0, height());

    QTransform screenToWorld = getWorldToScreenTransform().inverted();
    return screenToWorld.map(screenPoly);
}
bool MapView::isVisibleInView(const QPointF& worldPos) const
{
    QPolygonF visiblePoly = getVisibleWorldPolygon();
    return visiblePoly.containsPoint(worldPos, Qt::OddEvenFill);
}
// =====渲染函数=====
void MapView::renderGrid(QPainter& painter)
{
    if (!m_map) return;
    const int gridSize = m_map->getGridSize();
    QPolygonF visiblePoly = getVisibleWorldPolygon();
    QRectF visibleRect = visiblePoly.boundingRect();
    // 计算可见网格范围
    int minX = qMax(0, static_cast<int>(visibleRect.left() / gridSize) - 1);
    int maxX = qMin(1000 / gridSize, static_cast<int>(visibleRect.right() / gridSize) + 1);
    int minY = qMax(0, static_cast<int>(visibleRect.top() / gridSize) - 1);
    int maxY = qMin(1000 / gridSize, static_cast<int>(visibleRect.bottom() / gridSize) + 1);
    painter.setPen(QPen(Qt::gray, 0.5));
    for (int x = minX; x <= maxX; ++x) {
        QPointF p1(x * gridSize, minY * gridSize);
        QPointF p2(x * gridSize, maxY * gridSize);

        // 只绘制在可见区域内的网格线
        if (isVisibleInView(p1) || isVisibleInView(p2)) {
            painter.drawLine(p1, p2);
        }
    }
    for (int y = minY; y <= maxY; ++y) {
        QPointF p1(minX * gridSize, y * gridSize);
        QPointF p2(maxX * gridSize, y * gridSize);

        if (isVisibleInView(p1) || isVisibleInView(p2)) {
            painter.drawLine(p1, p2);
        }
    }
}







void MapView::renderEdges(QPainter& painter)
{
    if (!m_map) return;
    QPolygonF visiblePoly = getVisibleWorldPolygon();
    const int edgeCount = m_map->getEdgeCount();
    for (int i = 0; i < edgeCount; ++i) {
        const Edge* edge = m_map->getEdge(i);
        if (!edge) continue;
        const Vertex* from = m_map->getVertex(edge->fromVertex);
        const Vertex* to = m_map->getVertex(edge->toVertex);
        if (!from || !to) continue;
        // 精确的可见性判断
        if (!isVisibleInView(from->position) &&
            !isVisibleInView(to->position)) {
            continue;
        }

        // 根据车流量设置颜色
        double ratio = static_cast<double>(edge->currentVehicles) / edge->capacity;
        QColor color;
        if (ratio < 0.3) color = Qt::green;
        else if (ratio < 0.7) color = Qt::yellow;
        else color = Qt::red;

        painter.setPen(QPen(color, 2));
        painter.drawLine(from->position, to->position);
    }
}


void MapView::renderVertices(QPainter& painter)
{
    if (!m_map) return;
    QPolygonF visiblePoly = getVisibleWorldPolygon();
    const int vertexCount = m_map->getVertexCount();
    painter.setPen(Qt::black);
    painter.setBrush(Qt::blue);
    for (int i = 0; i < vertexCount; ++i) {
        const Vertex* vertex = m_map->getVertex(i);
        if (!vertex || !isVisibleInView(vertex->position)) continue;
        painter.drawEllipse(vertex->position, 3, 3);
    }
}

void MapView::renderVehicles(QPainter& painter)
{
    if (!m_map || !m_simulator) return;

    painter.setPen(Qt::black);
    painter.setBrush(Qt::red);

    // 这里简化处理，实际应该遍历所有车辆
    // 注意：需要Simulator提供车辆遍历接口
    // 这里仅作示例
}

// =====视角控制=====
void MapView::zoom(double factor)
{
    m_zoomLevel *= factor;
    m_zoomLevel = qBound(0.1, m_zoomLevel, 10.0);
    update();
}

void MapView::pan(const QPointF& delta)
{
    m_viewCenter += delta;
    update();
}

void MapView::rotate(double angle)
{
    m_rotation += angle;
    update();
}
