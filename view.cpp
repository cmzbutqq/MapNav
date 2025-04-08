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
QPointF MapView::worldToScreen(const QPointF& worldPos) const
{
    QPointF viewPos = worldPos - m_viewCenter;
    QPointF rotatedPos(
        viewPos.x() * cos(m_rotation) - viewPos.y() * sin(m_rotation),
        viewPos.x() * sin(m_rotation) + viewPos.y() * cos(m_rotation)
        );
    return rotatedPos * m_zoomLevel + QPointF(width()/2, height()/2);
}

QPointF MapView::screenToWorld(const QPoint& screenPos) const
{
    QPointF viewPos = (QPointF(screenPos) - QPointF(width()/2, height()/2)) / m_zoomLevel;
    QPointF rotatedPos(
        viewPos.x() * cos(-m_rotation) - viewPos.y() * sin(-m_rotation),
        viewPos.x() * sin(-m_rotation) + viewPos.y() * cos(-m_rotation)
        );
    return rotatedPos + m_viewCenter;
}

// =====渲染函数=====
void MapView::renderGrid(QPainter& painter)
{
    if (!m_map) return;

    const int gridSize = m_map->getGridSize();
    QRectF visibleRect = getVisibleWorldRect();

    // 计算可见网格范围
    int minX = qMax(0, static_cast<int>(visibleRect.left() / gridSize));
    int maxX = qMin(1000 / gridSize, static_cast<int>(visibleRect.right() / gridSize) + 1);
    int minY = qMax(0, static_cast<int>(visibleRect.top() / gridSize));
    int maxY = qMin(1000 / gridSize, static_cast<int>(visibleRect.bottom() / gridSize) + 1);

    painter.setPen(QPen(Qt::gray, 0.5));
    for (int x = minX; x <= maxX; ++x) {
        painter.drawLine(x * gridSize, minY * gridSize,
                         x * gridSize, maxY * gridSize);
    }
    for (int y = minY; y <= maxY; ++y) {
        painter.drawLine(minX * gridSize, y * gridSize,
                         maxX * gridSize, y * gridSize);
    }
}

void MapView::renderEdges(QPainter& painter)
{
    if (!m_map) return;

    QRectF visibleRect = getVisibleWorldRect();
    const int edgeCount = m_map->getEdgeCount();

    for (int i = 0; i < edgeCount; ++i) {
        const Edge* edge = m_map->getEdge(i);
        if (!edge) continue;

        const Vertex* from = m_map->getVertex(edge->fromVertex);
        const Vertex* to = m_map->getVertex(edge->toVertex);
        if (!from || !to) continue;

        // 简单可见性判断
        if (!visibleRect.contains(from->position) &&
            !visibleRect.contains(to->position)) {
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

    QRectF visibleRect = getVisibleWorldRect();
    const int vertexCount = m_map->getVertexCount();

    painter.setPen(Qt::black);
    painter.setBrush(Qt::blue);

    for (int i = 0; i < vertexCount; ++i) {
        const Vertex* vertex = m_map->getVertex(i);
        if (!vertex || !visibleRect.contains(vertex->position)) continue;

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

QRectF MapView::getVisibleWorldRect() const
{
    QRect screenRect(0, 0, width(), height());
    QPointF topLeft = screenToWorld(screenRect.topLeft());
    QPointF bottomRight = screenToWorld(screenRect.bottomRight());
    return QRectF(topLeft, bottomRight).normalized();
}
