#ifndef SHAPE_H
#define SHAPE_H

#include <QRect>
#include <QPainter>
#include <QString>
#define _USE_MATH_DEFINES // <--- 添加这一行，确保 M_PI 可用
#include <cmath>          // <--- 添加这一行，包含数学函数和常量

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
    Shape(ShapeType type, const QRect &rect);
    virtual ~Shape();

    // 绘制形状
    virtual void paint(QPainter *painter) = 0;

    // 获取和设置位置和大小
    QRect rect() const { return m_rect; }
    void setRect(const QRect &rect) { m_rect = rect; }

    // 获取形状类型
    ShapeType type() const { return m_type; }

    // 检查点是否在形状内
    virtual bool contains(const QPoint &point) const;

    // 获取形状名称
    QString name() const;

protected:
    ShapeType m_type;
    QRect m_rect;
};

// 矩形形状
class RectangleShape : public Shape
{
public:
    RectangleShape(const QRect &rect);
    void paint(QPainter *painter) override;
};

// 圆形形状
class CircleShape : public Shape
{
public:
    CircleShape(const QRect &rect);
    void paint(QPainter *painter) override;
};

// 五边形形状
class PentagonShape : public Shape
{
public:
    PentagonShape(const QRect &rect);
    void paint(QPainter *painter) override;
};

// 椭圆形形状
class EllipseShape : public Shape
{
public:
    EllipseShape(const QRect &rect);
    void paint(QPainter *painter) override;
};

#endif // SHAPE_H
