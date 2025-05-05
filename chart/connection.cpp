#include "chart/connection.h"
#include "chart/shape.h"
#include <cmath>

// ConnectionPoint实现
ConnectionPoint::ConnectionPoint(Shape* owner, Position position)
    : m_owner(owner), m_position(position), m_freePosition(0, 0)
{
}

// 自由位置构造函数实现
ConnectionPoint::ConnectionPoint(const QPoint& freePosition)
    : m_owner(nullptr), m_position(Free), m_freePosition(freePosition)
{
}

QPoint ConnectionPoint::getPosition() const
{
    if (m_position == Free) {
        return m_freePosition;
    }
    
    if (!m_owner) {
        return QPoint(0, 0);
    }

    return m_owner->getConnectionPoint(m_position);
}

void ConnectionPoint::setPosition(const QPoint& pos)
{
    if (m_position == Free) {
        m_freePosition = pos;
    }
    // 如果不是自由位置，则忽略设置
}

bool ConnectionPoint::equalTo(const ConnectionPoint* other) const{
    if (!other) {
        return false;  // 如果 other 是 nullptr，直接返回 false
    }
    
    if (m_position == Free && other->m_position == Free) {
        return m_freePosition == other->m_freePosition;
    }
    
    return (m_owner == other->m_owner) && (m_position == other->m_position);
}

QString ConnectionPoint::positionToString(Position pos)
{
    switch (pos) {
    case Top: return "Top";
    case Right: return "Right";
    case Bottom: return "Bottom";
    case Left: return "Left";
    case Free: return "Free";
    default: return "Unknown";
    }
}

ConnectionPoint::Position ConnectionPoint::stringToPosition(const QString& str)
{
    if (str == "Top") return Top;
    if (str == "Right") return Right;
    if (str == "Bottom") return Bottom;
    if (str == "Left") return Left;
    if (str == "Free") return Free;
    return Top; // 默认为Top
}

// Connection实现
Connection::Connection(ConnectionPoint* startPoint, ConnectionPoint* endPoint)
    : m_startPoint(startPoint), m_endPoint(endPoint), m_selected(false)
{
}

Connection::~Connection()
{
    // 连接点由Shape负责管理，这里不需要删除
    // 但需要删除自由位置的连接点
    if (m_startPoint && m_startPoint->getPositionType() == ConnectionPoint::Free) {
        delete m_startPoint;
    }
    
    if (m_endPoint && m_endPoint->getPositionType() == ConnectionPoint::Free) {
        delete m_endPoint;
    }
}

void Connection::drawConnectionLine(QPainter* painter,
                                 const QPoint& startPos, 
                                 const QPoint& endPos, 
                                 bool selected,
                                 bool drawArrow)
{
    // 保存画家状态
    painter->save();
    
    // 计算线条方向向量
    QPointF direction(endPos.x() - startPos.x(), endPos.y() - startPos.y());
    double length = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    
    // 防止除以零
    if (length < 0.001) {
        painter->restore();
        return;
    }
    
    // 计算单位向量
    QPointF unitDirection(direction.x() / length, direction.y() / length);
    
    // 计算垂直于方向的法向量
    QPointF normal(-unitDirection.y(), unitDirection.x());
    
    // 计算箭头相关参数
    const int arrowSize = 10;
    double angle = std::atan2(endPos.y() - startPos.y(), endPos.x() - startPos.x());
    
    // 计算线段终点（为避免与箭头重叠，稍微缩短线段）
    QPointF lineEndPoint = endPos;
    if (drawArrow) {
        lineEndPoint = endPos - QPointF(cos(angle) * arrowSize * 0.8, 
                                      sin(angle) * arrowSize * 0.8);
    }
    
    // 多层线条绘制
    if (selected) {
        // 渐变从线条外围向内层，颜色逐渐加深
        
        // 第四层（最外层）- 浅蓝色半透明效果
        QColor outerColor(100, 150, 255, 100); // RGBA，最后一个参数是透明度
        painter->setPen(QPen(outerColor, 4.0));
        painter->drawLine(startPos, lineEndPoint);
        
        // 第三层 - 中等蓝色
        QColor mediumColor(80, 130, 235, 150);
        painter->setPen(QPen(mediumColor, 3.5));
        painter->drawLine(startPos, lineEndPoint);
        
        // 第二层 - 深蓝色
        QColor innerColor(40, 100, 220, 200);
        painter->setPen(QPen(innerColor, 3.0));
        painter->drawLine(startPos, lineEndPoint);
    }
    
    // 最内层 - 黑色线条（无论是否选中）
    painter->setPen(QPen(Qt::black, 1.5));
    painter->drawLine(startPos, lineEndPoint);
    
    // 绘制箭头 - 始终为黑色
    if (drawArrow) {
        QPointF arrowP1 = endPos - QPointF(cos(angle - M_PI/6) * arrowSize, 
                                         sin(angle - M_PI/6) * arrowSize);
        QPointF arrowP2 = endPos - QPointF(cos(angle + M_PI/6) * arrowSize, 
                                         sin(angle + M_PI/6) * arrowSize);
        
        QPolygonF arrowHead;
        arrowHead << endPos << arrowP1 << arrowP2;
        painter->setBrush(Qt::black);
        painter->drawPolygon(arrowHead);
    }
    
    painter->restore();
}
void Connection::paint(QPainter* painter)
{
    if (!m_startPoint) {
        return;
    }
    
    QPoint startPos = m_startPoint->getPosition();
    QPoint endPos;
    
    if (m_endPoint) {
        endPos = m_endPoint->getPosition();
    } else {
        endPos = m_temporaryEndPoint;
    }
    // 使用共享绘制函数
    drawConnectionLine(painter, startPos, endPos, m_selected, true);

}

void Connection::setStartPoint(ConnectionPoint* point)
{
    // 删除旧的自由位置连接点
    if (m_startPoint && m_startPoint->getPositionType() == ConnectionPoint::Free) {
        delete m_startPoint;
    }
    
    m_startPoint = point;
}

void Connection::setEndPoint(ConnectionPoint* point)
{
    // 删除旧的自由位置连接点
    if (m_endPoint && m_endPoint->getPositionType() == ConnectionPoint::Free) {
        delete m_endPoint;
    }
    
    m_endPoint = point;
}

void Connection::setTemporaryEndPoint(const QPoint& point)
{
    m_temporaryEndPoint = point;
}

bool Connection::contains(const QPoint& point, int threshold) const
{
    if (!isComplete()) {
        return false;
    }
    
    QPoint startPos = m_startPoint->getPosition();
    QPoint endPos = m_endPoint->getPosition();
    
    // 计算点到线段的距离
    double distance = pointToLineDistance(point, startPos, endPos);
    
    return distance <= threshold;
}

QPoint Connection::getStartPosition() const
{
    return m_startPoint ? m_startPoint->getPosition() : QPoint(0, 0);
}

QPoint Connection::getEndPosition() const
{
    return m_endPoint ? m_endPoint->getPosition() : m_temporaryEndPoint;
}

bool Connection::isNearStartPoint(const QPoint& point, int threshold) const
{
    if (!m_startPoint) return false;
    
    QPoint startPos = m_startPoint->getPosition();
    double distance = std::sqrt(std::pow(point.x() - startPos.x(), 2) + 
                              std::pow(point.y() - startPos.y(), 2));
    
    return distance <= threshold;
}

bool Connection::isNearEndPoint(const QPoint& point, int threshold) const
{
    if (!m_endPoint) return false;
    
    QPoint endPos = m_endPoint->getPosition();
    double distance = std::sqrt(std::pow(point.x() - endPos.x(), 2) + 
                              std::pow(point.y() - endPos.y(), 2));
    
    return distance <= threshold;
}

double Connection::pointToLineDistance(const QPoint& point, 
                                     const QPoint& lineStart, 
                                     const QPoint& lineEnd) const
{
    // 线段长度为0的情况
    if (lineStart == lineEnd) {
        return std::sqrt(std::pow(point.x() - lineStart.x(), 2) + 
                         std::pow(point.y() - lineStart.y(), 2));
    }
    
    // 计算点到线段的距离
    double lineLength = std::sqrt(std::pow(lineEnd.x() - lineStart.x(), 2) + 
                                 std::pow(lineEnd.y() - lineStart.y(), 2));
    
    double t = ((point.x() - lineStart.x()) * (lineEnd.x() - lineStart.x()) + 
               (point.y() - lineStart.y()) * (lineEnd.y() - lineStart.y())) / 
               (lineLength * lineLength);
    
    if (t < 0) {
        // 点到起点的距离
        return std::sqrt(std::pow(point.x() - lineStart.x(), 2) + 
                         std::pow(point.y() - lineStart.y(), 2));
    }
    
    if (t > 1) {
        // 点到终点的距离
        return std::sqrt(std::pow(point.x() - lineEnd.x(), 2) + 
                         std::pow(point.y() - lineEnd.y(), 2));
    }
    
    // 点到线段的垂直距离
    double projX = lineStart.x() + t * (lineEnd.x() - lineStart.x());
    double projY = lineStart.y() + t * (lineEnd.y() - lineStart.y());
    
    return std::sqrt(std::pow(point.x() - projX, 2) + 
                     std::pow(point.y() - projY, 2));
}

// ArrowLine实现
ArrowLine::ArrowLine(const QPoint& startPoint, const QPoint& endPoint)
    : Connection(new ConnectionPoint(startPoint), new ConnectionPoint(endPoint))
{
}

ArrowLine::~ArrowLine()
{
    // 基类析构函数会处理连接点的释放
}

void ArrowLine::paint(QPainter* painter)
{
    // 直接使用基类的绘制方法，选中状态已在基类中处理
    Connection::paint(painter);
} 