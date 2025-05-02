#ifndef SHAPE_H
#define SHAPE_H

#include <QRect>
#include <QPainter>
#include <QString>
#include <QRegularExpression> 
#include <QVector>
#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "chart/connection.h" //因为要用到ConnectionPoint里的枚举

// 前向声明
class ConnectionPoint;

// 常量定义形状类型
namespace ShapeTypes {
    const QString Rectangle = "Rectangle";
    const QString Circle = "Circle";
    const QString Pentagon = "Pentagon";
    const QString Ellipse = "Ellipse";
}

// 基础形状类
class Shape
{
public:
    // 类型标识符现在是字符串
    Shape(const QString& type, const int& basis);
    virtual ~Shape();
    
    virtual void paint(QPainter* painter) = 0;
    
    virtual QRect getRect() const { return m_rect; }
    virtual void setRect(const QRect& rect);
    
    // 返回形状类型标识符
    QString type() const { return m_type; }
    
    // 返回显示名称（可以与类型不同）
    virtual QString displayName() const { return QObject::tr(m_type.toUtf8().constData()); }
    
    virtual bool contains(const QPoint& point) const;

    // 调整大小相关的枚举和方法
    enum HandlePosition {
        None = -1,
        TopLeft = 0,
        Top = 1,
        TopRight = 2,
        Right = 3,
        BottomRight = 4,
        Bottom = 5,
        BottomLeft = 6,
        Left = 7
    };
    
    // 绘制调整大小的手柄
    void drawResizeHandles(QPainter* painter) const;
    
    // 检测点击的是哪个手柄
    HandlePosition hitHandle(const QPoint& point) const;
    
    // 获取手柄的矩形区域
    QRect handleRect(HandlePosition position) const;
    
    // 调整大小
    void resize(HandlePosition handle, const QPoint& offset);
    
    // 文本相关方法
    void setText(const QString& text);
    QString text() const;
    bool isEditing() const;
    void setEditing(bool editing);
    //void updateText(const QString& text);
    
    // 绘制文本
    void drawText(QPainter* painter) const;
    
    // 获取文本位置
    virtual QRect textRect() const;

    // 连接点相关方法
    void drawConnectionPoints(QPainter* painter) const;

    virtual QPoint getConnectionPoint(ConnectionPoint::Position position) const;

    bool isHovered() const { return m_hovered; }
    void setHovered(bool hovered) { m_hovered = hovered; }
    
    // 获取所有可用的连接点
    virtual QVector<ConnectionPoint*> getConnectionPoints();

    // 检测点击了哪个连接点（如果有）
    ConnectionPoint* hitConnectionPoint(const QPoint& point) const;

protected:
    QString m_type;
    QRect m_rect;
    QString m_text;  // 存储形状中的文本
    bool m_editing;  // 标记是否处于编辑状态
    bool m_hovered;  // 标记鼠标是否悬停在形状上
    
    // 手柄大小常量
    static const int HANDLE_SIZE = 8;
    // 连接点大小常量
    static const int CONNECTION_POINT_SIZE = 8;
    
    // 存储连接点
    mutable QVector<ConnectionPoint*> m_connectionPoints;
    
    // 创建连接点（惰性初始化）
    virtual void createConnectionPoints() const;
};

// 矩形形状
class RectangleShape : public Shape
{
public:
    RectangleShape(const int& basis);
    void paint(QPainter* painter) override;
    QString displayName() const override { return QObject::tr("Rectangle"); }
    
    // 向工厂注册
    static void registerShape();
};

// 圆形形状
class CircleShape : public Shape
{
public:
    CircleShape(const int& basis);
    void paint(QPainter* painter) override;
    bool contains(const QPoint& point) const override;
    // void setRect(const QRect& rect) override;
    QString displayName() const override { return QObject::tr("Circle"); }
    QRect textRect() const override;
    
    // 向工厂注册
    static void registerShape();
};

// 五边形形状
class PentagonShape : public Shape
{
public:
    PentagonShape(const int& basis);
    void paint(QPainter* painter) override;
    bool contains(const QPoint& point) const override;
    QString displayName() const override { return QObject::tr("Pentagon"); }
    QRect textRect() const override;

    virtual QPoint getConnectionPoint(ConnectionPoint::Position position) const;
    
    // 向工厂注册
    static void registerShape();
    
private:
    QPolygon createPentagonPolygon() const;
};

// 椭圆形形状
class EllipseShape : public Shape
{
public:
    EllipseShape(const int& basis);
    void paint(QPainter* painter) override;
    bool contains(const QPoint& point) const override;
    QString displayName() const override { return QObject::tr("Ellipse"); }
    QRect textRect() const override;
    
    // 向工厂注册
    static void registerShape();
};


#endif // SHAPE_H
