#include "shape/shape.h"
#include "shape/shapefactory.h"

// Shape基类
Shape::Shape(const QString& type, const int& basis)
    : m_type(type)
{
    // m_rect will be initialized in derived classes
}

Shape::~Shape()
{
}

void Shape::setRect(const QRect& rect)
{
    m_rect = rect;
}

bool Shape::contains(const QPoint& point) const
{
    // 基类默认用矩形包含判断，子类可以重写提供更精确的检测
    return m_rect.contains(point);
}

// RectangleShape实现
RectangleShape::RectangleShape(const int& basis)
    : Shape(ShapeTypes::Rectangle, basis)
{
    // 长宽比为98:55，basis为宽
    int width = basis * 98 / 55;
    int height = basis;
    m_rect = QRect(0, 0, width, height);
}

void RectangleShape::paint(QPainter* painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(QColor(255, 255, 255, 128)));
    painter->drawRect(m_rect);
}

void RectangleShape::registerShape()
{
    ShapeFactory::instance().registerShape(
        ShapeTypes::Rectangle,
        [](const int& basis) -> Shape* { return new RectangleShape(basis); }
    );
}

// CircleShape实现
CircleShape::CircleShape(const int& basis)
    : Shape(ShapeTypes::Circle, basis)
{
    // 长和宽都为1.5*basis
    int size = 1.5 * basis;
    m_rect = QRect(0, 0, size, size);
}

void CircleShape::paint(QPainter* painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(QColor(255, 255, 255, 128)));
    
    // 直接绘制圆形，不需要额外计算
    painter->drawEllipse(m_rect);
}

void CircleShape::setRect(const QRect& rect)
{
    // 确保圆形的边界矩形始终是正方形
    int side = qMin(rect.width(), rect.height());
    m_rect = QRect(
        rect.x() + (rect.width() - side) / 2,
        rect.y() + (rect.height() - side) / 2,
        side, side
    );
}

bool CircleShape::contains(const QPoint& point) const
{
    // 计算点到圆心的距离，与半径比较
    QPoint center = m_rect.center();
    int dx = point.x() - center.x();
    int dy = point.y() - center.y();
    int distanceSquared = dx * dx + dy * dy;
    
    int radius = qMin(m_rect.width(), m_rect.height()) / 2;
    return distanceSquared <= radius * radius;
}

void CircleShape::registerShape()
{
    ShapeFactory::instance().registerShape(
        ShapeTypes::Circle,
        [](const int& basis) -> Shape* { return new CircleShape(basis); }
    );
}

// PentagonShape实现
PentagonShape::PentagonShape(const int& basis)
    : Shape(ShapeTypes::Pentagon, basis), m_basis(basis)
{
    // 根据数学公式计算五边形的边界矩形
    double cos18 = cos(M_PI / 10); // cos(18°)
    double cos36 = cos(M_PI / 5);  // cos(36°)
    
    int width = 2 * basis * cos18;
    int height = basis * (1 + cos36);
    
    m_rect = QRect(0, 0, width, height);
}

void PentagonShape::paint(QPainter* painter)
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
    QPoint center = m_rect.center();
    double cos36 = cos(M_PI / 5);  // cos(36°)
    center.setY(center.y() + m_basis*(1-cos36)/2);// 调整Y坐标，使五边形底边与矩形底边对齐
    int radius = m_basis; // 外接圆半径为basis
    
    // 计算五边形的五个顶点
    for (int i = 0; i < 5; ++i) {
        // 从顶部开始，顺时针计算顶点，第一个点在正上方
        double angle = (M_PI * 2 / 5) * i - M_PI / 2;
        int x = center.x() + radius * cos(angle);
        int y = center.y() + radius * sin(angle);
        polygon << QPoint(x, y);
    }
    
    return polygon;
}

bool PentagonShape::contains(const QPoint& point) const
{
    return createPentagonPolygon().containsPoint(point, Qt::OddEvenFill);
}

void PentagonShape::registerShape()
{
    ShapeFactory::instance().registerShape(
        ShapeTypes::Pentagon,
        [](const int& basis) -> Shape* { return new PentagonShape(basis); }
    );
}

// EllipseShape实现
EllipseShape::EllipseShape(const int& basis)
    : Shape(ShapeTypes::Ellipse, basis)
{
    // 长宽比为98:55，basis为宽
    int width = basis * 98 / 55;
    int height = basis;
    m_rect = QRect(0, 0, width, height);
}

void EllipseShape::paint(QPainter* painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(QColor(255, 255, 255, 128)));
    painter->drawEllipse(m_rect);
}

bool EllipseShape::contains(const QPoint& point) const
{
    // 检查点是否在椭圆内
    QPoint center = m_rect.center();
    double a = m_rect.width() / 2.0;  // 半长轴
    double b = m_rect.height() / 2.0; // 半短轴
    
    double normX = (point.x() - center.x()) / a;
    double normY = (point.y() - center.y()) / b;
    
    return (normX * normX + normY * normY) <= 1.0;
}

void EllipseShape::registerShape()
{
    ShapeFactory::instance().registerShape(
        ShapeTypes::Ellipse,
        [](const int& basis) -> Shape* { return new EllipseShape(basis); }
    );
}