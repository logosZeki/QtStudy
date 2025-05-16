#include "chart/connection.h"
#include "chart/shape.h"
#include <cmath>
ConnectionPoint::ConnectionPoint(Shape* owner, Position position)
    : m_owner(owner), m_position(position), m_freePosition(0, 0)
{
}
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
}
bool ConnectionPoint::equalTo(const ConnectionPoint* other) const{
    if (!other) {
        return false;  
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
    return Top; 
}
Connection::Connection(ConnectionPoint* startPoint, ConnectionPoint* endPoint)
    : m_startPoint(startPoint), m_endPoint(endPoint), m_selected(false)
{
}
Connection::~Connection()
{
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
    painter->save();
    QPointF direction(endPos.x() - startPos.x(), endPos.y() - startPos.y());
    double length = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    if (length < 0.001) {
        painter->restore();
        return;
    }
    QPointF unitDirection(direction.x() / length, direction.y() / length);
    QPointF normal(-unitDirection.y(), unitDirection.x());
    const int arrowSize = 10;
    double angle = std::atan2(endPos.y() - startPos.y(), endPos.x() - startPos.x());
    QPointF lineEndPoint = endPos;
    if (drawArrow) {
        lineEndPoint = endPos - QPointF(cos(angle) * arrowSize * 0.8, 
                                      sin(angle) * arrowSize * 0.8);
    }
    if (selected) {
        QColor outerColor(100, 150, 255, 100); 
        painter->setPen(QPen(outerColor, 4.0));
        painter->drawLine(startPos, lineEndPoint);
        QColor mediumColor(80, 130, 235, 150);
        painter->setPen(QPen(mediumColor, 3.5));
        painter->drawLine(startPos, lineEndPoint);
        QColor innerColor(40, 100, 220, 200);
        painter->setPen(QPen(innerColor, 3.0));
        painter->drawLine(startPos, lineEndPoint);
    }
    painter->setPen(QPen(Qt::black, 1.5));
    painter->drawLine(startPos, lineEndPoint);
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
    drawConnectionLine(painter, startPos, endPos, m_selected, true);
}
void Connection::setStartPoint(ConnectionPoint* point)
{
    if (m_startPoint && m_startPoint->getPositionType() == ConnectionPoint::Free) {
        delete m_startPoint;
    }
    m_startPoint = point;
}
void Connection::setEndPoint(ConnectionPoint* point)
{
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
    if (lineStart == lineEnd) {
        return std::sqrt(std::pow(point.x() - lineStart.x(), 2) + 
                         std::pow(point.y() - lineStart.y(), 2));
    }
    double lineLength = std::sqrt(std::pow(lineEnd.x() - lineStart.x(), 2) + 
                                 std::pow(lineEnd.y() - lineStart.y(), 2));
    double t = ((point.x() - lineStart.x()) * (lineEnd.x() - lineStart.x()) + 
               (point.y() - lineStart.y()) * (lineEnd.y() - lineStart.y())) / 
               (lineLength * lineLength);
    if (t < 0) {
        return std::sqrt(std::pow(point.x() - lineStart.x(), 2) + 
                         std::pow(point.y() - lineStart.y(), 2));
    }
    if (t > 1) {
        return std::sqrt(std::pow(point.x() - lineEnd.x(), 2) + 
                         std::pow(point.y() - lineEnd.y(), 2));
    }
    double projX = lineStart.x() + t * (lineEnd.x() - lineStart.x());
    double projY = lineStart.y() + t * (lineEnd.y() - lineStart.y());
    return std::sqrt(std::pow(point.x() - projX, 2) + 
                     std::pow(point.y() - projY, 2));
}
ArrowLine::ArrowLine(const QPoint& startPoint, const QPoint& endPoint)
    : Connection(new ConnectionPoint(startPoint), new ConnectionPoint(endPoint))
{
}
ArrowLine::~ArrowLine()
{
}
void ArrowLine::paint(QPainter* painter)
{
    Connection::paint(painter);
} 
