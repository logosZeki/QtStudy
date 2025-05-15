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
        Left,
        Free 
    };

    ConnectionPoint(Shape* owner, Position position);
    ConnectionPoint(const QPoint& freePosition); 
    QPoint getPosition() const;
    Shape* getOwner() const { return m_owner; }
    Position getPositionType() const { return m_position; }

    bool equalTo(const ConnectionPoint* other) const;
    void setPosition(const QPoint& pos); 

    static QString positionToString(Position pos);
    static Position stringToPosition(const QString& str);

private:
    Shape* m_owner;
    Position m_position;
    QPoint m_freePosition; 
};

class Connection
{
public:
    Connection(ConnectionPoint* startPoint = nullptr, ConnectionPoint* endPoint = nullptr);
    virtual ~Connection();
    
    virtual void paint(QPainter* painter);
    
    void setStartPoint(ConnectionPoint* point);
    void setEndPoint(ConnectionPoint* point);
    void setTemporaryEndPoint(const QPoint& point);
    ConnectionPoint* getStartPoint() const { return m_startPoint; }
    ConnectionPoint* getEndPoint() const { return m_endPoint; }
    
    bool isComplete() const { return m_startPoint && m_endPoint; }
    bool isTemporary() const { return m_startPoint && !m_endPoint; }
    
    bool contains(const QPoint& point, int threshold = 5) const;
    
    // 获取连接线的起点和终点位置
    QPoint getStartPosition() const;
    QPoint getEndPosition() const;
    
    bool isNearStartPoint(const QPoint& point, int threshold = 10) const;
    bool isNearEndPoint(const QPoint& point, int threshold = 10) const;
    
    // 选中状态控制
    void setSelected(bool selected) { m_selected = selected; }
    bool isSelected() const { return m_selected; }

    // 绘制连线
    static void drawConnectionLine(QPainter* painter, 
                                 const QPoint& startPos, 
                                 const QPoint& endPos, 
                                 bool selected,
                                 bool drawArrow);
    
protected:
    ConnectionPoint* m_startPoint;
    ConnectionPoint* m_endPoint;
    QPoint m_temporaryEndPoint; // 用于绘制连线预览
    bool m_selected; // 是否被选中
    double pointToLineDistance(const QPoint& point, 
                              const QPoint& lineStart, 
                              const QPoint& lineEnd) const;
};


class ArrowLine : public Connection
{
public:
    ArrowLine(const QPoint& startPoint, const QPoint& endPoint);
    virtual ~ArrowLine();
    
    // 支持选中状态
    virtual void paint(QPainter* painter) override;
};

#endif // CONNECTION_H 