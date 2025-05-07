#include "chart/shape.h"
#include "chart/shapefactory.h"
#include "chart/connection.h"


// Shape基类
Shape::Shape(const QString& type, const int& basis)
    : m_type(type), m_editing(false)
{
    // m_rect will be initialized in derived classes
}

Shape::~Shape()
{
    // 清理连接点
    qDeleteAll(m_connectionPoints);
    m_connectionPoints.clear();
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
    case HandlePosition::TopLeft:
        center = m_rect.topLeft();
        break;
    case HandlePosition::Top:
        center = QPoint(m_rect.center().x(), m_rect.top());
        break;
    case HandlePosition::TopRight:
        center = m_rect.topRight();
        break;
    case HandlePosition::Right:
        center = QPoint(m_rect.right(), m_rect.center().y());
        break;
    case HandlePosition::BottomRight:
        center = m_rect.bottomRight();
        break;
    case HandlePosition::Bottom:
        center = QPoint(m_rect.center().x(), m_rect.bottom());
        break;
    case HandlePosition::BottomLeft:
        center = m_rect.bottomLeft();
        break;
    case HandlePosition::Left:
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
    case HandlePosition::TopLeft:
        newRect.setTopLeft(newRect.topLeft() + offset);
        break;
    case HandlePosition::Top:
        newRect.setTop(newRect.top() + offset.y());
        break;
    case HandlePosition::TopRight:
        newRect.setTopRight(newRect.topRight() + offset);
        break;
    case HandlePosition::Right:
        newRect.setRight(newRect.right() + offset.x());
        break;
    case HandlePosition::BottomRight:
        newRect.setBottomRight(newRect.bottomRight() + offset);
        break;
    case HandlePosition::Bottom:
        newRect.setBottom(newRect.bottom() + offset.y());
        break;
    case HandlePosition::BottomLeft:
        newRect.setBottomLeft(newRect.bottomLeft() + offset);
        break;
    case HandlePosition::Left:
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



// 连接点相关实现
void Shape::drawConnectionPoints(QPainter* painter) const
{
    
    // 如果连接点列表为空，创建它们
    if (m_connectionPoints.isEmpty()) {
        createConnectionPoints();
    }
    
    painter->save();
    
    // 设置连接点样式
    painter->setPen(Qt::blue);
    painter->setBrush(Qt::white);
    
    // 绘制所有连接点
    for (const ConnectionPoint* point : m_connectionPoints) {
        QPoint pos = point->getPosition();
        int halfSize = CONNECTION_POINT_SIZE / 2;
        QRect pointRect(pos.x() - halfSize, pos.y() - halfSize, 
                       CONNECTION_POINT_SIZE, CONNECTION_POINT_SIZE);
        painter->drawEllipse(pointRect);
    }
    
    painter->restore();
}

QPoint Shape::getConnectionPoint(ConnectionPoint::Position position) const{
    QRect rect = this->getRect();
    //QPoint point = m_owner->getConnectionPoint(m_position);
    switch (position) {
    case ConnectionPoint::Top:
        return QPoint(rect.center().x(), rect.top());
    case ConnectionPoint::Right:
        return QPoint(rect.right(), rect.center().y());
    case ConnectionPoint::Bottom:
        return QPoint(rect.center().x(), rect.bottom());
    case ConnectionPoint::Left:
        return QPoint(rect.left(), rect.center().y());
    default:
        return rect.center();
    }
}

void Shape::createConnectionPoints() const
{
    // 清理旧的连接点（如果有）
    qDeleteAll(m_connectionPoints);
    m_connectionPoints.clear();
    
    // 创建四个标准连接点
    m_connectionPoints.append(new ConnectionPoint(const_cast<Shape*>(this), ConnectionPoint::Top));
    m_connectionPoints.append(new ConnectionPoint(const_cast<Shape*>(this), ConnectionPoint::Right));
    m_connectionPoints.append(new ConnectionPoint(const_cast<Shape*>(this), ConnectionPoint::Bottom));
    m_connectionPoints.append(new ConnectionPoint(const_cast<Shape*>(this), ConnectionPoint::Left));
}

QVector<ConnectionPoint*> Shape::getConnectionPoints()
{
    if (m_connectionPoints.isEmpty()) {
        createConnectionPoints();
    }
    
    return m_connectionPoints;
}

ConnectionPoint* Shape::hitConnectionPoint(const QPoint& point,bool isStart) const
{
    if (m_connectionPoints.isEmpty()) {
        createConnectionPoints();
    }
    if(isStart){//起点判定范围小
        // 检测点击的是哪个连接点
        for (ConnectionPoint* cp : m_connectionPoints) {
            QPoint pos = cp->getPosition();
            int halfSize = CONNECTION_POINT_SIZE*3/4;
            QRect pointRect(pos.x() - halfSize, pos.y() - halfSize, 
                            CONNECTION_POINT_SIZE*3/2, CONNECTION_POINT_SIZE*3/2);
            
            if (pointRect.contains(point)) {
                return cp;
            }
        }

    }else{//终点判定范围大
            // 检测点击的是哪个连接点
        for (ConnectionPoint* cp : m_connectionPoints) {
            QPoint pos = cp->getPosition();
            int halfSize = CONNECTION_POINT_SIZE;
            QRect pointRect(pos.x() - halfSize, pos.y() - halfSize, 
                            CONNECTION_POINT_SIZE*2, CONNECTION_POINT_SIZE*2);
            
            if (pointRect.contains(point)) {
                return cp;
            }
        }

    }
    
    
    return nullptr;
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
    painter->setBrush(QBrush(Qt::white));
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
    painter->setBrush(QBrush(Qt::white));
    
    // 直接绘制圆形，不需要额外计算
    painter->drawEllipse(m_rect);
    
    // 绘制文本
    drawText(painter);
}

QRect CircleShape::textRect() const
{
    // 为椭圆提供合适的文本区域
    int marginX = m_rect.width() / 4;
    int marginY = m_rect.height() / 4;
    return m_rect.adjusted(marginX, marginY, -marginX, -marginY);
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
    painter->setBrush(QBrush(Qt::white));
    
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

QPoint PentagonShape::getConnectionPoint(ConnectionPoint::Position position) const{

    QRect rect = this->getRect();
    // 获取矩形的宽和高
    int w = rect.width();
    int h = rect.height();
    // 按照新的公式计算五个点
    double cos18 = cos(M_PI / 10); // cos(18°)
    double sin36 = sin(M_PI / 5);  // sin(36°)
    double cos36 = cos(M_PI / 5);  // cos(36°)
    //QPoint point = m_owner->getConnectionPoint(m_position);
    switch (position) {
    case ConnectionPoint::Top:
        return QPoint(m_rect.left() + w/2, m_rect.top());
    case ConnectionPoint::Right:
        return QPoint(m_rect.left() + w, m_rect.top() + (2*sin36*sin36)/(1+cos36)*h);
    case ConnectionPoint::Bottom:
        return QPoint(m_rect.left() + w/2, m_rect.top() + h);
    case ConnectionPoint::Left:
        return QPoint(m_rect.left(), m_rect.top() + (2*sin36*sin36)/(1+cos36)*h);
    default:
        return rect.center();
    }
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
    painter->setBrush(QBrush(Qt::white));
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

// 圆角矩形实现
RoundedRectangleShape::RoundedRectangleShape(const int& basis)
    : Shape(ShapeTypes::RoundedRectangle, basis)
{
    // 长宽比为98:55，basis为宽
    int width = basis * 98 / 55;
    int height = basis;
    m_rect = QRect(0, 0, width, height);
    
    // 圆角半径为矩形高度的1/6
    m_radius = height / 6;
}

void RoundedRectangleShape::paint(QPainter* painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(Qt::white));
    painter->drawRoundedRect(m_rect, m_radius, m_radius);
    
    // 绘制文本
    drawText(painter);
}

void RoundedRectangleShape::registerShape()
{
    ShapeFactory::instance().registerShape(
        ShapeTypes::RoundedRectangle,
        [](const int& basis) -> Shape* { return new RoundedRectangleShape(basis); }
    );
}

// 菱形实现
DiamondShape::DiamondShape(const int& basis)
    : Shape(ShapeTypes::Diamond, basis)
{
    // 设定菱形的宽高
    int width = basis * 1.5;
    int height = basis;
    m_rect = QRect(0, 0, width, height);
}

void DiamondShape::paint(QPainter* painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(Qt::white));
    
    // 创建菱形的点
    QPolygon polygon = createDiamondPolygon();
    painter->drawPolygon(polygon);
    
    // 绘制文本
    drawText(painter);
}

QPolygon DiamondShape::createDiamondPolygon() const
{
    // 获取矩形的宽和高
    int w = m_rect.width();
    int h = m_rect.height();
    
    // 计算菱形的四个顶点
    QPolygon polygon;
    
    // 上中点
    polygon << QPoint(m_rect.left() + w/2, m_rect.top());
    
    // 右中点
    polygon << QPoint(m_rect.right(), m_rect.top() + h/2);
    
    // 下中点
    polygon << QPoint(m_rect.left() + w/2, m_rect.bottom());
    
    // 左中点
    polygon << QPoint(m_rect.left(), m_rect.top() + h/2);
    
    return polygon;
}

QRect DiamondShape::textRect() const
{
    // 为菱形提供合适的文本区域
    int marginX = m_rect.width() / 4;
    int marginY = m_rect.height() / 4;
    return m_rect.adjusted(marginX, marginY, -marginX, -marginY);
}

bool DiamondShape::contains(const QPoint& point) const
{
    return createDiamondPolygon().containsPoint(point, Qt::OddEvenFill);
}

QPoint DiamondShape::getConnectionPoint(ConnectionPoint::Position position) const
{
    QRect rect = this->getRect();
    
    switch (position) {
    case ConnectionPoint::Top:
        return QPoint(rect.center().x(), rect.top());
    case ConnectionPoint::Right:
        return QPoint(rect.right(), rect.center().y());
    case ConnectionPoint::Bottom:
        return QPoint(rect.center().x(), rect.bottom());
    case ConnectionPoint::Left:
        return QPoint(rect.left(), rect.center().y());
    default:
        return rect.center();
    }
}

void DiamondShape::registerShape()
{
    ShapeFactory::instance().registerShape(
        ShapeTypes::Diamond,
        [](const int& basis) -> Shape* { return new DiamondShape(basis); }
    );
}

// 六边形实现
HexagonShape::HexagonShape(const int& basis)
    : Shape(ShapeTypes::Hexagon, basis)
{
    // 设定六边形的宽高
    double h = basis;
    double w = h * 1.1547; // 近似值，等于h*2/sqrt(3)
    m_rect = QRect(0, 0, w, h);
}

void HexagonShape::paint(QPainter* painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(Qt::white));
    
    // 创建六边形的点
    QPolygon polygon = createHexagonPolygon();
    painter->drawPolygon(polygon);
    
    // 绘制文本
    drawText(painter);
}

QPolygon HexagonShape::createHexagonPolygon() const
{
    // 获取矩形的宽和高
    int w = m_rect.width();
    int h = m_rect.height();
    
    // 计算六边形的六个顶点
    QPolygon polygon;
    
    // 上左点
    polygon << QPoint(m_rect.left() + w/4, m_rect.top());
    
    // 上右点
    polygon << QPoint(m_rect.left() + 3*w/4, m_rect.top());
    
    // 右点
    polygon << QPoint(m_rect.right(), m_rect.top() + h/2);
    
    // 下右点
    polygon << QPoint(m_rect.left() + 3*w/4, m_rect.bottom());
    
    // 下左点
    polygon << QPoint(m_rect.left() + w/4, m_rect.bottom());
    
    // 左点
    polygon << QPoint(m_rect.left(), m_rect.top() + h/2);
    
    return polygon;
}

QRect HexagonShape::textRect() const
{
    // 为六边形提供合适的文本区域
    int marginX = m_rect.width() / 4;
    int marginY = m_rect.height() / 4;
    return m_rect.adjusted(marginX, marginY, -marginX, -marginY);
}

bool HexagonShape::contains(const QPoint& point) const
{
    return createHexagonPolygon().containsPoint(point, Qt::OddEvenFill);
}

QPoint HexagonShape::getConnectionPoint(ConnectionPoint::Position position) const
{
    QRect rect = this->getRect();
    int w = rect.width();
    int h = rect.height();
    
    switch (position) {
    case ConnectionPoint::Top:
        return QPoint(rect.left() + w/2, rect.top());
    case ConnectionPoint::Right:
        return QPoint(rect.right(), rect.top() + h/2);
    case ConnectionPoint::Bottom:
        return QPoint(rect.left() + w/2, rect.bottom());
    case ConnectionPoint::Left:
        return QPoint(rect.left(), rect.top() + h/2);
    default:
        return rect.center();
    }
}

void HexagonShape::registerShape()
{
    ShapeFactory::instance().registerShape(
        ShapeTypes::Hexagon,
        [](const int& basis) -> Shape* { return new HexagonShape(basis); }
    );
}

// 八边形实现
OctagonShape::OctagonShape(const int& basis)
    : Shape(ShapeTypes::Octagon, basis)
{
    // 设定八边形的宽高
    int size = basis * 1.2;
    m_rect = QRect(0, 0, size, size);
}

void OctagonShape::paint(QPainter* painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(Qt::white));
    
    // 创建八边形的点
    QPolygon polygon = createOctagonPolygon();
    painter->drawPolygon(polygon);
    
    // 绘制文本
    drawText(painter);
}

QPolygon OctagonShape::createOctagonPolygon() const
{
    // 获取矩形的宽和高
    int w = m_rect.width();
    int h = m_rect.height();
    
    // 计算八边形的八个顶点
    QPolygon polygon;
    int wOffset = w / 4; // 缩进量
    int HOffset = h / 4; // 缩进量
    
    // 上左
    polygon << QPoint(m_rect.left() + wOffset, m_rect.top());
    
    // 上右
    polygon << QPoint(m_rect.right() - wOffset, m_rect.top());
    
    // 右上
    polygon << QPoint(m_rect.right(), m_rect.top() + HOffset);
    
    // 右下
    polygon << QPoint(m_rect.right(), m_rect.bottom() - HOffset);
    
    // 下右
    polygon << QPoint(m_rect.right() - wOffset, m_rect.bottom());
    
    // 下左
    polygon << QPoint(m_rect.left() + wOffset, m_rect.bottom());
    
    // 左下
    polygon << QPoint(m_rect.left(), m_rect.bottom() - HOffset);
    
    // 左上
    polygon << QPoint(m_rect.left(), m_rect.top() + HOffset);
    
    return polygon;
}

QRect OctagonShape::textRect() const
{
    // 为八边形提供合适的文本区域
    int margin = m_rect.width() / 4;
    return m_rect.adjusted(margin, margin, -margin, -margin);
}

bool OctagonShape::contains(const QPoint& point) const
{
    return createOctagonPolygon().containsPoint(point, Qt::OddEvenFill);
}

QPoint OctagonShape::getConnectionPoint(ConnectionPoint::Position position) const
{
    QRect rect = this->getRect();
    int w = rect.width();
    int h = rect.height();
    int offset = w / 4;
    
    switch (position) {
    case ConnectionPoint::Top:
        return QPoint(rect.left() + w/2, rect.top());
    case ConnectionPoint::Right:
        return QPoint(rect.right(), rect.top() + h/2);
    case ConnectionPoint::Bottom:
        return QPoint(rect.left() + w/2, rect.bottom());
    case ConnectionPoint::Left:
        return QPoint(rect.left(), rect.top() + h/2);
    default:
        return rect.center();
    }
}

void OctagonShape::registerShape()
{
    ShapeFactory::instance().registerShape(
        ShapeTypes::Octagon,
        [](const int& basis) -> Shape* { return new OctagonShape(basis); }
    );
}

// 云朵形状实现
CloudShape::CloudShape(const int& basis)
    : Shape(ShapeTypes::Cloud, basis)
{
    // 设定云朵的宽高
    int width = basis * 1.5;
    int height = basis;
    m_rect = QRect(0, 0, width, height);
}

void CloudShape::paint(QPainter* painter)
{
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QBrush(Qt::white));
    
    // 绘制云朵形状
    QPainterPath path;
    
    // 获取矩形的宽和高
    int w = m_rect.width();
    int h = m_rect.height();
    
    // 圆的半径
    int r1 = h * 0.4;  // 大圆
    int r2 = h * 0.3;  // 中圆
    int r3 = h * 0.25; // 小圆
    
    // 画云朵的左部
    path.addEllipse(m_rect.left() + r1*0.25, m_rect.top() + r1*0.5, r1, r1);
    
    // 画云朵的中上部
    path.addEllipse(m_rect.left() + w*0.4, m_rect.top() + r2*0.1, r2, r2);
    
    // 画云朵的右部
    path.addEllipse(m_rect.right() - r1*1.25, m_rect.top() + r1*0.5, r1, r1);
    
    // 画云朵的右下部
    path.addEllipse(m_rect.left() + w*0.7, m_rect.top() + h*0.5, r3, r3);
    
    // 画云朵的左下部
    path.addEllipse(m_rect.left() + w*0.2, m_rect.top() + h*0.5, r3, r3);
    
    painter->drawPath(path);
    
    // 绘制文本
    drawText(painter);
}

QRect CloudShape::textRect() const
{
    // 为云朵提供合适的文本区域
    int marginX = m_rect.width() / 5;
    int marginY = m_rect.height() / 5;
    return m_rect.adjusted(marginX, marginY, -marginX, -marginY);
}

bool CloudShape::contains(const QPoint& point) const
{
    // 简化云朵的形状判断，使用外接矩形
    return m_rect.contains(point);
}

QPoint CloudShape::getConnectionPoint(ConnectionPoint::Position position) const
{
    QRect rect = this->getRect();
    
    switch (position) {
    case ConnectionPoint::Top:
        return QPoint(rect.center().x(), rect.top() + rect.height()*0.2);
    case ConnectionPoint::Right:
        return QPoint(rect.right() - rect.width()*0.1, rect.center().y());
    case ConnectionPoint::Bottom:
        return QPoint(rect.center().x(), rect.bottom() - rect.height()*0.2);
    case ConnectionPoint::Left:
        return QPoint(rect.left() + rect.width()*0.1, rect.center().y());
    default:
        return rect.center();
    }
}

void CloudShape::registerShape()
{
    ShapeFactory::instance().registerShape(
        ShapeTypes::Cloud,
        [](const int& basis) -> Shape* { return new CloudShape(basis); }
    );
}

