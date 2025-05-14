#include "chart/shape.h"
#include "chart/shapefactory.h"
#include "chart/connection.h"


// Shape基类
Shape::Shape(const QString& type, const int& basis)
    : m_type(type), m_editing(false)
{
    // 初始化默认字体为微软雅黑，12px
    m_font = QFont("微软雅黑", 12);
    m_fontColor = Qt::black;
    m_fillColor = Qt::white;  // 默认填充颜色为白色
    m_lineColor = Qt::black;  // 默认线条颜色为黑色
    m_textAlignment = Qt::AlignCenter;
    m_transparency = 100;     // 默认完全不透明
    m_lineWidth = 1.5;        // 默认线条粗细为1.5px
    m_lineStyle = 0;          // 默认线条样式为实线
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
    // 如果正在编辑或者文本为空，不绘制文本
    if (m_editing || m_text.isEmpty())
        return;
        
    QRect rect = textRect();
    painter->save();
    
    // 根据透明度计算alpha值（0-255）
    int alpha = qRound(m_transparency * 2.55); // 将0-100的值映射到0-255
    
    // 设置考虑透明度的字体颜色
    QColor fontColorWithAlpha = m_fontColor;
    fontColorWithAlpha.setAlpha(alpha);
    painter->setPen(fontColorWithAlpha);
    
    painter->setFont(m_font);  // 使用设置的字体
    painter->drawText(rect, m_textAlignment | Qt::TextWordWrap, m_text);
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
    painter->save();
    setupPainter(painter);
    painter->drawRect(m_rect);
    painter->restore();
    
    // 如果有文本，绘制文本
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
    painter->save();
    setupPainter(painter);
    painter->drawEllipse(m_rect);
    painter->restore();
    
    // 如果有文本，绘制文本
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
    
    int width = 2 * (basis*0.8) * cos18;
    int height = (basis*0.8) * (1 + cos36);
    
    m_rect = QRect(0, 0, width, height);
}

void PentagonShape::paint(QPainter* painter)
{
    painter->save();
    setupPainter(painter);
    painter->drawPolygon(createPentagonPolygon());
    painter->restore();
    
    // 如果有文本，绘制文本
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
    painter->save();
    setupPainter(painter);
    painter->drawEllipse(m_rect);
    painter->restore();
    
    // 如果有文本，绘制文本
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
    painter->save();
    setupPainter(painter);
    painter->drawRoundedRect(m_rect, m_radius, m_radius);
    painter->restore();
    
    // 如果有文本，绘制文本
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

    // 长宽比为98:55，basis为宽
    int width = basis * 98 / 55;
    int height = basis;
    m_rect = QRect(0, 0, width, height);
}

void DiamondShape::paint(QPainter* painter)
{
    painter->save();
    setupPainter(painter);
    painter->drawPolygon(createDiamondPolygon());
    painter->restore();
    
    // 如果有文本，绘制文本
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

    int width = basis * 98 / 55;
    int height = basis;
    m_rect = QRect(0, 0, width, height);
}

void HexagonShape::paint(QPainter* painter)
{
    painter->save();
    setupPainter(painter);
    painter->drawPolygon(createHexagonPolygon());
    painter->restore();
    
    // 如果有文本，绘制文本
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
    painter->save();
    setupPainter(painter);
    painter->drawPolygon(createOctagonPolygon());
    painter->restore();
    
    // 如果有文本，绘制文本
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
    m_cloudPath= createCloudPath();
}

void CloudShape::paint(QPainter* painter)
{
    painter->save();
    setupPainter(painter);
    m_cloudPath= createCloudPath();
    painter->drawPath(m_cloudPath);
    painter->restore();
    
    // 如果有文本，绘制文本
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
    return m_cloudPath.contains(QPointF(point));
}

QPoint CloudShape::getConnectionPoint(ConnectionPoint::Position position) const
{

    QPointF topmost, bottommost, leftmost, rightmost;
    findExtremePointsOnPath(m_cloudPath, topmost, bottommost, 
                                      leftmost, rightmost, 
                                      60);
    switch (position) {
    case ConnectionPoint::Top:
        return topmost.toPoint();
    case ConnectionPoint::Right:
        return rightmost.toPoint();
    case ConnectionPoint::Bottom:
        return bottommost.toPoint();
    case ConnectionPoint::Left:
        return leftmost.toPoint();
    default:
        return m_rect.center();
    }
}
// 辅助函数，通过采样找到路径上的极值点,numberOfSamples采样点数量，越高越精确，但计算量越大
void CloudShape::findExtremePointsOnPath(
    const QPainterPath& path, 
    QPointF& outTopmost, 
    QPointF& outBottommost, 
    QPointF& outLeftmost, 
    QPointF& outRightmost,
    int numberOfSamples ) const
{
    if (path.isEmpty()) {
        qWarning() << "Path is empty, cannot find extreme points.";
        outTopmost = outBottommost = outLeftmost = outRightmost = QPointF(); // 返回无效点
        return;
    }

    // 使用路径的第一个点进行初始化
    outTopmost    = path.pointAtPercent(0.0);
    outBottommost = path.pointAtPercent(0.0);
    outLeftmost   = path.pointAtPercent(0.0);
    outRightmost  = path.pointAtPercent(0.0);

    if (numberOfSamples < 2) {
        numberOfSamples = 2; // 至少需要两个样本点
    }

    for (int i = 0; i <= numberOfSamples; ++i) { // 从0开始确保覆盖起点和终点
        qreal percent = static_cast<qreal>(i) / static_cast<qreal>(numberOfSamples);
        QPointF currentPoint = path.pointAtPercent(percent);

        if (currentPoint.y() < outTopmost.y()) {
            outTopmost = currentPoint;
        }
        if (currentPoint.y() > outBottommost.y()) {
            outBottommost = currentPoint;
        }
        if (currentPoint.x() < outLeftmost.x()) {
            outLeftmost = currentPoint;
        }
        if (currentPoint.x() > outRightmost.x()) {
            outRightmost = currentPoint;
        }
    }
}
void CloudShape::registerShape()
{
    ShapeFactory::instance().registerShape(
        ShapeTypes::Cloud,
        [](const int& basis) -> Shape* { return new CloudShape(basis); }
    );
}



QPainterPath CloudShape::createCloudPath() {

    QRectF targetRect(m_rect);
    QPointF pointA(30, 110);
    QPainterPath prototypeCloudPath;
    prototypeCloudPath.moveTo(pointA);
    // 底部三个凸起
    prototypeCloudPath.cubicTo(QPointF(40, 130), QPointF(65, 130), QPointF(75, 108));
    prototypeCloudPath.cubicTo(QPointF(85, 130), QPointF(115, 140), QPointF(125, 108));
    QPointF endOfBottom(170, 105);
    prototypeCloudPath.cubicTo(QPointF(135, 130), QPointF(160, 130), endOfBottom);
    // 右侧、顶部和左侧的凸起
    prototypeCloudPath.cubicTo(QPointF(200, 100), QPointF(200, 65), QPointF(175, 45));
    prototypeCloudPath.cubicTo(QPointF(180, 15), QPointF(140, 10), QPointF(110, 30));
    prototypeCloudPath.cubicTo(QPointF(80, 5), QPointF(40, 15), QPointF(35, 45));
    prototypeCloudPath.cubicTo(QPointF(0, 55), QPointF(0, 90), pointA);
    prototypeCloudPath.closeSubpath();


    if (m_rect.width() <= 0 || m_rect.height() <= 0) {
        // targetRect 无效 (例如，窗口过小)
        return QPainterPath(); // 返回空路径
    }
    QRectF prototypeCloudBoundingRect =prototypeCloudPath.boundingRect();
    qreal protoX = prototypeCloudBoundingRect.left();
    qreal protoY = prototypeCloudBoundingRect.top();
    qreal protoWidth = prototypeCloudBoundingRect.width();
    qreal protoHeight = prototypeCloudBoundingRect.height();

    qreal scaleX = targetRect.width() / protoWidth;
    qreal scaleY = targetRect.height() / protoHeight;

    // 计算变换矩阵的平移分量 (dx, dy)
    // 使得原型路径的 (protoX, protoY) 点在缩放后映射到 targetRect.left() 和 targetRect.top()
    qreal final_dx = targetRect.left() - protoX * scaleX;
    qreal final_dy = targetRect.top() - protoY * scaleY;

    QTransform transform(scaleX,   // m11
                         0,        // m12 (x 对 y' 的贡献)
                         0,        // m21 (y 对 x' 的贡献)
                         scaleY,   // m22
                         final_dx, // dx
                         final_dy  // dy
                         );
    return transform.map(prototypeCloudPath);
}

// 字体相关方法实现
void Shape::setFontFamily(const QString& family)
{
    m_font.setFamily(family);
}

QString Shape::fontFamily() const
{
    return m_font.family();
}

void Shape::setFontSize(int size)
{
    m_font.setPointSize(size);
}

int Shape::fontSize() const
{
    return m_font.pointSize();
}

void Shape::setFontBold(bool bold)
{
    m_font.setWeight(bold ? QFont::Bold : QFont::Normal);
}

bool Shape::isFontBold() const
{
    return m_font.weight() >= QFont::Bold;
}

void Shape::setFontItalic(bool italic)
{
    m_font.setItalic(italic);
}

bool Shape::isFontItalic() const
{
    return m_font.italic();
}

void Shape::setFontUnderline(bool underline)
{
    m_font.setUnderline(underline);
}

bool Shape::isFontUnderline() const
{
    return m_font.underline();
}

void Shape::setFontColor(const QColor& color)
{
    m_fontColor = color;
}

QColor Shape::fontColor() const
{
    return m_fontColor;
}

void Shape::setTextAlignment(Qt::Alignment alignment)
{
    m_textAlignment = alignment;
}

Qt::Alignment Shape::textAlignment() const
{
    return m_textAlignment;
}

QFont Shape::getFont() const
{
    return m_font;
}

void Shape::setFillColor(const QColor& color)
{
    m_fillColor = color;
}

QColor Shape::fillColor() const
{
    return m_fillColor;
}

void Shape::setLineColor(const QColor& color)
{
    m_lineColor = color;
}

QColor Shape::lineColor() const
{
    return m_lineColor;
}

// 透明度相关方法实现
void Shape::setTransparency(int transparency)
{
    // 确保透明度在0-100范围内
    m_transparency = qBound(0, transparency, 100);
}

int Shape::transparency() const
{
    return m_transparency;
}

// 线条粗细相关方法实现
void Shape::setLineWidth(qreal width)
{
    // 确保线条粗细为正数
    m_lineWidth = qMax(0.0, width);
}

qreal Shape::lineWidth() const
{
    return m_lineWidth;
}

// 线条样式相关方法实现
void Shape::setLineStyle(int style)
{
    // 确保样式值在有效范围内（0-3，0为实线，1-3为各种虚线样式）
    m_lineStyle = qBound(0, style, 3);
}

int Shape::lineStyle() const
{
    return m_lineStyle;
}

// 为所有派生类的paint方法添加一个通用的设置画笔和画刷的方法
void Shape::setupPainter(QPainter* painter) const
{
    // 根据透明度计算alpha值（0-255）
    int alpha = qRound(m_transparency * 2.55); // 将0-100的值映射到0-255
    
    // 设置填充颜色（考虑透明度）
    QColor fillColorWithAlpha = m_fillColor;
    fillColorWithAlpha.setAlpha(alpha);
    painter->setBrush(QBrush(fillColorWithAlpha));
    
    // 设置线条颜色（考虑透明度）
    QColor lineColorWithAlpha = m_lineColor;
    lineColorWithAlpha.setAlpha(alpha);
    
    // 创建画笔并设置宽度
    QPen pen(lineColorWithAlpha);
    pen.setWidthF(m_lineWidth);
    
    // 根据线条样式设置画笔样式
    switch(m_lineStyle) {
    case 0: // 实线
        pen.setStyle(Qt::SolidLine);
        break;
    case 1: // 虚线样式一：pattern<< 1.0 << 1.0
        {
            QVector<qreal> pattern;
            pattern << 3.0 << 3.0;
            pen.setDashPattern(pattern);
        }
        break;
    case 2: // 虚线样式二：pattern<< 8.0 << 3.0
        {
            QVector<qreal> pattern;
            pattern << 8.0 << 3.0;
            pen.setDashPattern(pattern);
        }
        break;
    case 3: // 虚线样式三：pattern<< 7.0 << 3.0<<2.0<<3.0
        {
            QVector<qreal> pattern;
            pattern << 7.0 << 3.0 << 2.0 << 3.0;
            pen.setDashPattern(pattern);
        }
        break;
    }
    
    painter->setPen(pen);
}