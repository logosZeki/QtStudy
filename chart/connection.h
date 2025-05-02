#ifndef CONNECTION_H
#define CONNECTION_H

#include <QPoint>
#include <QPainter>

// 前向声明
class Shape;

// 连接点类
class ConnectionPoint
{
public:
    enum  Position {
        Top,
        Right,
        Bottom,
        Left
    };

    ConnectionPoint(Shape* owner, Position position);
    
    QPoint getPosition() const;
    Shape* getOwner() const { return m_owner; }
    Position getPositionType() const { return m_position; }
    
    static QString positionToString(Position pos);
    static Position stringToPosition(const QString& str);

private:
    Shape* m_owner;
    Position m_position;
};

// 连线类
class Connection
{
public:
    Connection(ConnectionPoint* startPoint = nullptr, ConnectionPoint* endPoint = nullptr);
    ~Connection();
    
    void paint(QPainter* painter);
    
    void setStartPoint(ConnectionPoint* point);
    void setEndPoint(ConnectionPoint* point);
    void setTemporaryEndPoint(const QPoint& point);
    
    ConnectionPoint* getStartPoint() const { return m_startPoint; }
    ConnectionPoint* getEndPoint() const { return m_endPoint; }
    
    bool isComplete() const { return m_startPoint && m_endPoint; }
    bool isTemporary() const { return m_startPoint && !m_endPoint; }
    
    bool contains(const QPoint& point, int threshold = 5) const;
    
private:
    ConnectionPoint* m_startPoint;
    ConnectionPoint* m_endPoint;
    QPoint m_temporaryEndPoint; // 用于绘制连线预览
    
    // 辅助方法计算线段到点的距离
    double pointToLineDistance(const QPoint& point, 
                              const QPoint& lineStart, 
                              const QPoint& lineEnd) const;
};

#endif // CONNECTION_H 