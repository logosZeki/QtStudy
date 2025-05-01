#include "shape/shape.h"
#include "shape/shapefactory.h"


// Shape基类
Shape::Shape(const QString& type, const int& basis)
    : m_type(type), m_editing(false)
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

void Shape::drawResizeHandles(QPainter* painter) const
{
    painter->save();
    painter->setPen(QPen(Qt::blue, 1));
    painter->setBrush(QBrush(Qt::white));
    
    // 绘制八个调整大小的手柄 (Draw eight resize handles)
    for (int i = 0; i < 8; i++) {
        HandlePosition pos = static_cast<HandlePosition>(i);
        painter->drawRect(handleRect(pos));
    }
    
    painter->restore();
}

Shape::HandlePosition Shape::hitHandle(const QPoint& point) const
{
    // 检查点击的是哪个手柄
    for (int i = 0; i < 8; i++) {
        HandlePosition pos = static_cast<HandlePosition>(i);
        if (handleRect(pos).contains(point)) {
            return pos;
        }
    }
    return None;
}

QRect Shape::handleRect(HandlePosition position) const
{
    int halfSize = HANDLE_SIZE / 2;
    QPoint center;
    
    // 根据位置计算手柄的中心点
    switch (position) {
    case TopLeft:
        center = m_rect.topLeft();
        break;
    case Top:
        center = QPoint(m_rect.center().x(), m_rect.top());
        break;
    case TopRight:
        center = m_rect.topRight();
        break;
    case Right:
        center = QPoint(m_rect.right(), m_rect.center().y());
        break;
    case BottomRight:
        center = m_rect.bottomRight();
        break;
    case Bottom:
        center = QPoint(m_rect.center().x(), m_rect.bottom());
        break;
    case BottomLeft:
        center = m_rect.bottomLeft();
        break;
    case Left:
        center = QPoint(m_rect.left(), m_rect.center().y());
        break;
    default:
        return QRect();
    }
    
    // 返回以中心点为中心的手柄矩形
    return QRect(center.x() - halfSize, center.y() - halfSize, 
                 HANDLE_SIZE, HANDLE_SIZE);
}

void Shape::resize(HandlePosition handle, const QPoint& offset)
{
    QRect newRect = m_rect;
    
    // 根据不同的手柄位置调整矩形的不同部分
    switch (handle) {
    case TopLeft:
        newRect.setTopLeft(newRect.topLeft() + offset);
        break;
    case Top:
        newRect.setTop(newRect.top() + offset.y());
        break;
    case TopRight:
        newRect.setTopRight(newRect.topRight() + offset);
        break;
    case Right:
        newRect.setRight(newRect.right() + offset.x());
        break;
    case BottomRight:
        newRect.setBottomRight(newRect.bottomRight() + offset);
        break;
    case Bottom:
        newRect.setBottom(newRect.bottom() + offset.y());
        break;
    case BottomLeft:
        newRect.setBottomLeft(newRect.bottomLeft() + offset);
        break;
    case Left:
        newRect.setLeft(newRect.left() + offset.x());
        break;
    default:
        return;
    }
    
    // 确保形状有最小尺寸 (Ensure shape has minimum size)
    const int MIN_SIZE = 20;
    if (newRect.width() < MIN_SIZE) {
        if (newRect.left() != m_rect.left()) {
            newRect.setLeft(newRect.right() - MIN_SIZE);
        } else {
            newRect.setRight(newRect.left() + MIN_SIZE);
        }
    }
    
    if (newRect.height() < MIN_SIZE) {
        if (newRect.top() != m_rect.top()) {
            newRect.setTop(newRect.bottom() - MIN_SIZE);
        } else {
            newRect.setBottom(newRect.top() + MIN_SIZE);
        }
    }
    
    // 更新矩形 (Update rectangle)
    m_rect = newRect;
}

void Shape::setText(const QString& text)
{
    m_text = text;
    m_text.remove(QRegularExpression("^\\n+")); // 去除开头的换行符
}

QString Shape::text() const
{
    return m_text;
}

bool Shape::isEditing() const
{
    return m_editing;
}

void Shape::setEditing(bool editing)
{
    m_editing = editing;
}

//void Shape::updateText(const QString& text)
//{
//    m_text = text;
//}

void Shape::drawText(QPainter* painter) const
{
    if (m_text.isEmpty())
        return;
        
    QRect rect = textRect();
    painter->save();
    painter->setPen(Qt::black);
    painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, m_text);
    painter->restore();
}

QRect Shape::textRect() const
{
    // 默认实现，子类可以重写以提供更合适的文本区域
    int margin = qMin(m_rect.width(), m_rect.height()) / 10;
    return m_rect.adjusted(margin, margin, -margin, -margin);
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
    
    // 绘制文本
    drawText(painter);
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
    
    // 绘制文本
    drawText(painter);
}

QRect CircleShape::textRect() const
{
    // 为圆形提供合适的文本区域
    int side = qMin(m_rect.width(), m_rect.height());
    int margin = side / 4;  // 较大的边距，因为圆形有较小的有效区域
    
    return QRect(
        m_rect.center().x() - side / 2 + margin,
        m_rect.center().y() - side / 2 + margin,
        side - 2 * margin,
        side - 2 * margin
    );
}

bool CircleShape::contains(const QPoint& point) const
{
    // 检查点是否在椭圆内
    QPoint center = m_rect.center();
    double a = m_rect.width() / 2.0;  // 半长轴
    double b = m_rect.height() / 2.0; // 半短轴
    
    double normX = (point.x() - center.x()) / a;
    double normY = (point.y() - center.y()) / b;
    
    return (normX * normX + normY * normY) <= 1.0;
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
    : Shape(ShapeTypes::Pentagon, basis)
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
    
    // 绘制文本
    drawText(painter);
}

QPolygon PentagonShape::createPentagonPolygon() const
{
    // 获取矩形的宽和高
    int w = m_rect.width();
    int h = m_rect.height();
    
    // 计算五边形的五个顶点
    QPolygon polygon;
    
    // 按照新的公式计算五个点
    double cos18 = cos(M_PI / 10); // cos(18°)
    double sin36 = sin(M_PI / 5);  // sin(36°)
    double cos36 = cos(M_PI / 5);  // cos(36°)
    
    // 点A - 左下
    polygon << QPoint(m_rect.left() + ((cos18-sin36)/(2*cos18))*w, m_rect.top() + h);
    
    // 点B - 右下
    polygon << QPoint(m_rect.left() + ((cos18+sin36)/(2*cos18))*w, m_rect.top() + h);
    
    // 点C - 右中
    polygon << QPoint(m_rect.left() + w, m_rect.top() + (2*sin36*sin36)/(1+cos36)*h);
    
    // 点D - 顶点
    polygon << QPoint(m_rect.left() + w/2, m_rect.top());
    
    // 点E - 左中
    polygon << QPoint(m_rect.left(), m_rect.top() + (2*sin36*sin36)/(1+cos36)*h);
    
    return polygon;
}

QRect PentagonShape::textRect() const
{
    // 为五边形提供合适的文本区域
    int margin = qMin(m_rect.width(), m_rect.height()) / 4;
    return m_rect.adjusted(margin, margin, -margin, -margin);
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
    
    // 绘制文本
    drawText(painter);
}

QRect EllipseShape::textRect() const
{
    // 为椭圆提供合适的文本区域
    int marginX = m_rect.width() / 4;
    int marginY = m_rect.height() / 4;
    return m_rect.adjusted(marginX, marginY, -marginX, -marginY);
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