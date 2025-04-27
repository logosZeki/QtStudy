#ifndef SHAPE_H
#define SHAPE_H

#include <QRect>
#include <QPainter>
#include <QString>
#define _USE_MATH_DEFINES
#include <cmath>

// 基础形状类
class Shape
{
public:
    // 类型标识符现在是字符串
    Shape(const QString& type, const int& basis);
    virtual ~Shape();
    
    virtual void paint(QPainter* painter) = 0;
    
    QRect rect() const { return m_rect; }
    virtual void setRect(const QRect& rect);
    
    // 返回形状类型标识符
    QString type() const { return m_type; }
    
    // 返回显示名称（可以与类型不同）
    virtual QString displayName() const { return m_type; }
    
    virtual bool contains(const QPoint& point) const;

protected:
    QString m_type;
    QRect m_rect;
};

// 矩形形状
class RectangleShape : public Shape
{
public:
    RectangleShape(const int& basis);
    void paint(QPainter* painter) override;
    QString displayName() const override { return "矩形"; }
    
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
    void setRect(const QRect& rect) override;
    QString displayName() const override { return "圆形"; }
    
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
    QString displayName() const override { return "五边形"; }
    
    // 向工厂注册
    static void registerShape();
    
private:
    QPolygon createPentagonPolygon() const;
    int m_basis; // 存储基准值用于绘制
};

// 椭圆形形状
class EllipseShape : public Shape
{
public:
    EllipseShape(const int& basis);
    void paint(QPainter* painter) override;
    bool contains(const QPoint& point) const override;
    QString displayName() const override { return "椭圆形"; }
    
    // 向工厂注册
    static void registerShape();
};

// 常量定义形状类型
namespace ShapeTypes {
    const QString Rectangle = "Rectangle";
    const QString Circle = "Circle";
    const QString Pentagon = "Pentagon";
    const QString Ellipse = "Ellipse";
}

#endif // SHAPE_H
