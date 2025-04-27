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

void Shape::setRect(const QRect &rect)
{
    m_rect = rect;
}

bool Shape::contains(const QPoint &point) const
{
    // 基类默认用矩形包含判断，子类可以重写提供更精确的检测
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
    // 初始化时就将矩形调整为正方形
    int side = qMin(rect.width(), rect.height());
    m_rect = QRect(
        rect.x() + (rect.width() - side) / 2,
        rect.y() + (rect.height() - side) / 2,
        side, side
    );
}

void CircleShape::paint(QPainter *painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(QColor(255, 255, 255, 128)));
    
    // 直接绘制圆形，不需要额外计算
    painter->drawEllipse(m_rect);
}

void CircleShape::setRect(const QRect &rect)
{
    // 确保圆形的边界矩形始终是正方形
    int side = qMin(rect.width(), rect.height());
    m_rect = QRect(
        rect.x() + (rect.width() - side) / 2,
        rect.y() + (rect.height() - side) / 2,
        side, side
    );
}

bool CircleShape::contains(const QPoint &point) const
{
    // 计算点到圆心的距离
    QPoint center = m_rect.center();
    int radius = m_rect.width() / 2;  // 已经确保是正方形，所以宽高相等
    int dx = point.x() - center.x();
    int dy = point.y() - center.y();
    
    return (dx * dx + dy * dy <= radius * radius);
}

// PentagonShape实现
PentagonShape::PentagonShape(const QRect &rect)
    : Shape(Pentagon, rect)
{
    // 确保五边形的边界矩形是正方形，便于计算五边形顶点
    int side = qMin(rect.width(), rect.height());
    m_rect = QRect(
        rect.x() + (rect.width() - side) / 2,
        rect.y() + (rect.height() - side) / 2,
        side, side
    );
}

void PentagonShape::paint(QPainter *painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(QColor(255, 255, 255, 128)));
    
    // 创建五边形的点
    QPolygon polygon = createPentagonPolygon();
    painter->drawPolygon(polygon);
}

QPolygon PentagonShape::createPentagonPolygon() const
{
    QPolygon polygon;
    int centerX = m_rect.center().x();
    int centerY = m_rect.center().y();
    int radius = m_rect.width() / 2;  // 已经确保是正方形，所以宽高相等
    
    for (int i = 0; i < 5; ++i) {
        double angle = i * 2 * M_PI / 5 - M_PI / 2; // 从顶部开始
        int x = centerX + radius * cos(angle);
        int y = centerY + radius * sin(angle);
        polygon << QPoint(x, y);
    }
    
    return polygon;
}

bool PentagonShape::contains(const QPoint &point) const
{
    return createPentagonPolygon().containsPoint(point, Qt::OddEvenFill);
}

// EllipseShape实现
EllipseShape::EllipseShape(const QRect &rect)
    : Shape(Ellipse, rect)
{
    // 椭圆形使用原始矩形，不需要调整
}

void EllipseShape::paint(QPainter *painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(QColor(255, 255, 255, 128)));
    painter->drawEllipse(m_rect);
}

bool EllipseShape::contains(const QPoint &point) const
{
    // 计算椭圆公式 (x-h)²/a² + (y-k)²/b² <= 1
    QPoint center = m_rect.center();
    double a = m_rect.width() / 2.0;
    double b = m_rect.height() / 2.0;
    
    if (a <= 0 || b <= 0) return false;
    
    double dx = (point.x() - center.x()) / a;
    double dy = (point.y() - center.y()) / b;
    
    return (dx * dx + dy * dy <= 1.0);
}