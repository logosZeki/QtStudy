#ifndef SHAPE_H
#define SHAPE_H

#include <QRect>
#include <QPainter>
#include <QString>
#define _USE_MATH_DEFINES
#include <cmath>

// 形状类型枚举
enum ShapeType {
    Rectangle,
    Circle,
    Pentagon,
    Ellipse
};

// 形状基类
class Shape
{
public:
    Shape(ShapeType type, const int &basis);
    virtual ~Shape();
    
    // 绘制形状
    virtual void paint(QPainter *painter) = 0;
    
    // 获取和设置位置和大小
    QRect rect() const { return m_rect; }
    virtual void setRect(const QRect &rect);
    
    // 获取形状类型
    ShapeType type() const { return m_type; }
    
    // 检查点是否在形状内
    virtual bool contains(const QPoint &point) const;
    
    // 获取形状名称
    QString name() const;

protected:
    ShapeType m_type;
    QRect m_rect;  // 形状的边界矩形
};

// 矩形形状
class RectangleShape : public Shape
{
public:
    RectangleShape(const int &basis);
    void paint(QPainter *painter) override;
};

// 圆形形状
class CircleShape : public Shape
{
public:
    CircleShape(const int &basis);
    void paint(QPainter *painter) override;
    bool contains(const QPoint &point) const override;
    void setRect(const QRect &rect) override;
};

// 五边形形状
class PentagonShape : public Shape
{
public:
    PentagonShape(const int &basis);
    void paint(QPainter *painter) override;
    bool contains(const QPoint &point) const override;
    
private:
    QPolygon createPentagonPolygon() const;
    int m_basis; // 存储基准值用于绘制
};

// 椭圆形形状
class EllipseShape : public Shape
{
public:
    EllipseShape(const int &basis);
    void paint(QPainter *painter) override;
    bool contains(const QPoint &point) const override;
};

#endif // SHAPE_H
