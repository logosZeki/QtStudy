#include "shape.h"
#include <QPolygon>

// Shape基类
Shape::Shape(ShapeType type, const QRect &rect)
    : m_type(type), m_rect(rect)
{
}

Shape::~Shape()
{
}

bool Shape::contains(const QPoint &point) const
{
    return m_rect.contains(point);
}

QString Shape::name() const
{
    switch (m_type) {
    case Rectangle: return "矩形";
    case Circle: return "圆形";
    case Pentagon: return "五边形";
    case Ellipse: return "椭圆形";
    default: return "未知形状";
    }
}

// RectangleShape实现
RectangleShape::RectangleShape(const QRect &rect)
    : Shape(Rectangle, rect)
{
}

void RectangleShape::paint(QPainter *painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(QColor(255, 255, 255, 128)));
    painter->drawRect(m_rect);
}

// CircleShape实现
CircleShape::CircleShape(const QRect &rect)
    : Shape(Circle, rect)
{
}

void CircleShape::paint(QPainter *painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(QColor(255, 255, 255, 128)));

    // 使用最小边长来确保是圆形
    int side = qMin(m_rect.width(), m_rect.height());
    QRect adjustedRect(m_rect.x(), m_rect.y(), side, side);
    painter->drawEllipse(adjustedRect);
}

// PentagonShape实现
PentagonShape::PentagonShape(const QRect &rect)
    : Shape(Pentagon, rect)
{
}

void PentagonShape::paint(QPainter *painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(QColor(255, 255, 255, 128)));

    // 创建五边形的点
    QPolygon polygon;
    int centerX = m_rect.center().x();
    int centerY = m_rect.center().y();
    int radius = qMin(m_rect.width(), m_rect.height()) / 2;

    for (int i = 0; i < 5; ++i) {
        double angle = i * 2 * M_PI / 5 - M_PI / 2; // 从顶部开始
        int x = centerX + radius * cos(angle);
        int y = centerY + radius * sin(angle);
        polygon << QPoint(x, y);
    }

    painter->drawPolygon(polygon);
}

// EllipseShape实现
EllipseShape::EllipseShape(const QRect &rect)
    : Shape(Ellipse, rect)
{
}

void EllipseShape::paint(QPainter *painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(QColor(255, 255, 255, 128)));
    painter->drawEllipse(m_rect);
}
