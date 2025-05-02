#include "shape/connection.h"
#include "shape/shape.h"
#include <cmath>

// ConnectionPoint实现
ConnectionPoint::ConnectionPoint(Shape* owner, Position position)
    : m_owner(owner), m_position(position)
{
}

QPoint ConnectionPoint::getPosition() const
{
    if (!m_owner) {
        return QPoint(0, 0);//是什么坐标系里的0,0
    }

    // QRect rect = m_owner->getRect();
    QPoint point = m_owner->getConnectionPoint(m_position);
    // switch (m_position) {
    // case Top:
    //     return QPoint(rect.center().x(), rect.top());
    // case Right:
    //     return QPoint(rect.right(), rect.center().y());
    // case Bottom:
    //     return QPoint(rect.center().x(), rect.bottom());
    // case Left:
    //     return QPoint(rect.left(), rect.center().y());
    // default:
    //     return rect.center();
    // }
}

QString ConnectionPoint::positionToString(Position pos)
{
    switch (pos) {
    case Top: return "Top";
    case Right: return "Right";
    case Bottom: return "Bottom";
    case Left: return "Left";
    default: return "Unknown";
    }
}

ConnectionPoint::Position ConnectionPoint::stringToPosition(const QString& str)
{
    if (str == "Top") return Top;
    if (str == "Right") return Right;
    if (str == "Bottom") return Bottom;
    if (str == "Left") return Left;
    return Top; // 默认为Top
}

// Connection实现
Connection::Connection(ConnectionPoint* startPoint, ConnectionPoint* endPoint)
    : m_startPoint(startPoint), m_endPoint(endPoint)
{
}

Connection::~Connection()
{
    // 连接点由Shape负责管理，这里不需要删除
}

void Connection::paint(QPainter* painter)
{
    if (!m_startPoint) {
        return;
    }
    
    painter->save();
    
    // 设置连线样式
    QPen pen(Qt::black, 1.5);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    
    QPoint startPos = m_startPoint->getPosition();
    QPoint endPos;
    
    if (m_endPoint) {
        endPos = m_endPoint->getPosition();
    } else {
        endPos = m_temporaryEndPoint;
    }
    
    // 绘制连线
    painter->drawLine(startPos, endPos);
    
    // 绘制箭头
    const int arrowSize = 10;
    double angle = std::atan2(endPos.y() - startPos.y(), endPos.x() - startPos.x());
    
    QPointF arrowP1 = endPos - QPointF(cos(angle - M_PI/6) * arrowSize, 
                                       sin(angle - M_PI/6) * arrowSize);
    QPointF arrowP2 = endPos - QPointF(cos(angle + M_PI/6) * arrowSize, 
                                       sin(angle + M_PI/6) * arrowSize);
    
    QPolygonF arrowHead;
    arrowHead << endPos << arrowP1 << arrowP2;
    painter->setBrush(Qt::black);
    painter->drawPolygon(arrowHead);
    
    painter->restore();
}

void Connection::setStartPoint(ConnectionPoint* point)
{
    m_startPoint = point;
}

void Connection::setEndPoint(ConnectionPoint* point)
{
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