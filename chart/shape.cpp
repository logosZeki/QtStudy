#include "chart/shape.h"
#include "chart/shapefactory.h"
#include "chart/connection.h"
Shape::Shape(const QString& type, const int& basis)
    : m_type(type), m_editing(false)
{
    m_font = QFont("寰蒋闆呴粦", 12);
    m_fontColor = Qt::black;
    m_fillColor = Qt::white;  
    m_lineColor = Qt::black;  
    m_textAlignment = Qt::AlignCenter;
    m_transparency = 100;     
    m_lineWidth = 1.5;        
    m_lineStyle = 0;          
}
Shape::~Shape()
{
    qDeleteAll(m_connectionPoints);
    m_connectionPoints.clear();
}
void Shape::setRect(const QRect& rect)
{
    m_rect = rect;
}
bool Shape::contains(const QPoint& point) const
{
    return m_rect.contains(point);
}
void Shape::drawResizeHandles(QPainter* painter) const
{
    painter->save();
    painter->setPen(QPen(Qt::blue, 1));
    painter->setBrush(QBrush(Qt::white));
    for (int i = 0; i < 8; i++) {
        HandlePosition pos = static_cast<HandlePosition>(i);
        painter->drawRect(handleRect(pos));
    }
    painter->restore();
}
Shape::HandlePosition Shape::hitHandle(const QPoint& point) const
{
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
    return QRect(center.x() - halfSize, center.y() - halfSize, 
                 HANDLE_SIZE, HANDLE_SIZE);
}
void Shape::resize(HandlePosition handle, const QPoint& offset)
{
    QRect newRect = m_rect;
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
    m_rect = newRect;
}
void Shape::setText(const QString& text)
{
    m_text = text;
    m_text.remove(QRegularExpression("^\\n+")); 
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
void Shape::drawText(QPainter* painter) const
{
    if (m_editing || m_text.isEmpty())
        return;
    QRect rect = textRect();
    painter->save();
    int alpha = qRound(m_transparency * 2.55); 
    QColor fontColorWithAlpha = m_fontColor;
    fontColorWithAlpha.setAlpha(alpha);
    painter->setPen(fontColorWithAlpha);
    painter->setFont(m_font);  
    painter->drawText(rect, m_textAlignment | Qt::TextWordWrap, m_text);
    painter->restore();
}
QRect Shape::textRect() const
{
    return m_rect;
}
void Shape::drawConnectionPoints(QPainter* painter) const
{
    if (m_connectionPoints.isEmpty()) {
        createConnectionPoints();
    }
    painter->save();
    painter->setPen(Qt::blue);
    painter->setBrush(Qt::white);
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
    qDeleteAll(m_connectionPoints);
    m_connectionPoints.clear();
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
    if(isStart){
        for (ConnectionPoint* cp : m_connectionPoints) {
            QPoint pos = cp->getPosition();
            int halfSize = CONNECTION_POINT_SIZE*3/4;
            QRect pointRect(pos.x() - halfSize, pos.y() - halfSize, 
                            CONNECTION_POINT_SIZE*3/2, CONNECTION_POINT_SIZE*3/2);
            if (pointRect.contains(point)) {
                return cp;
            }
        }
    }else{
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
RectangleShape::RectangleShape(const int& basis)
    : Shape(ShapeTypes::Rectangle, basis)
{
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
    drawText(painter);
}
void RectangleShape::registerShape()
{
    ShapeFactory::instance().registerShape(
        ShapeTypes::Rectangle,
        [](const int& basis) -> Shape* { return new RectangleShape(basis); }
    );
}
CircleShape::CircleShape(const int& basis)
    : Shape(ShapeTypes::Circle, basis)
{
    int size = 1.5 * basis;
    m_rect = QRect(0, 0, size, size);
}
void CircleShape::paint(QPainter* painter)
{
    painter->save();
    setupPainter(painter);
    painter->drawEllipse(m_rect);
    painter->restore();
    drawText(painter);
}
bool CircleShape::contains(const QPoint& point) const
{
    QPoint center = m_rect.center();
    double a = m_rect.width() / 2.0;  
    double b = m_rect.height() / 2.0; 
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
PentagonShape::PentagonShape(const int& basis)
    : Shape(ShapeTypes::Pentagon, basis)
{
    double cos18 = cos(M_PI / 10); 
    double cos36 = cos(M_PI / 5);  
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
    drawText(painter);
}
QPolygon PentagonShape::createPentagonPolygon() const
{
    int w = m_rect.width();
    int h = m_rect.height();
    QPolygon polygon;
    double cos18 = cos(M_PI / 10); 
    double sin36 = sin(M_PI / 5);  
    double cos36 = cos(M_PI / 5);  
    polygon << QPoint(m_rect.left() + ((cos18-sin36)/(2*cos18))*w, m_rect.top() + h);
    polygon << QPoint(m_rect.left() + ((cos18+sin36)/(2*cos18))*w, m_rect.top() + h);
    polygon << QPoint(m_rect.left() + w, m_rect.top() + (2*sin36*sin36)/(1+cos36)*h);
    polygon << QPoint(m_rect.left() + w/2, m_rect.top());
    polygon << QPoint(m_rect.left(), m_rect.top() + (2*sin36*sin36)/(1+cos36)*h);
    return polygon;
}
bool PentagonShape::contains(const QPoint& point) const
{
    return createPentagonPolygon().containsPoint(point, Qt::OddEvenFill);
}
QPoint PentagonShape::getConnectionPoint(ConnectionPoint::Position position) const{
    QRect rect = this->getRect();
    int w = rect.width();
    int h = rect.height();
    double cos18 = cos(M_PI / 10); 
    double sin36 = sin(M_PI / 5);  
    double cos36 = cos(M_PI / 5);  
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
EllipseShape::EllipseShape(const int& basis)
    : Shape(ShapeTypes::Ellipse, basis)
{
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
    drawText(painter);
}
bool EllipseShape::contains(const QPoint& point) const
{
    QPoint center = m_rect.center();
    double a = m_rect.width() / 2.0;  
    double b = m_rect.height() / 2.0; 
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
RoundedRectangleShape::RoundedRectangleShape(const int& basis)
    : Shape(ShapeTypes::RoundedRectangle, basis)
{
    int width = basis * 98 / 55;
    int height = basis;
    m_rect = QRect(0, 0, width, height);
    m_radius = height / 6;
}
void RoundedRectangleShape::paint(QPainter* painter)
{
    painter->save();
    setupPainter(painter);
    painter->drawRoundedRect(m_rect, m_radius, m_radius);
    painter->restore();
    drawText(painter);
}
void RoundedRectangleShape::registerShape()
{
    ShapeFactory::instance().registerShape(
        ShapeTypes::RoundedRectangle,
        [](const int& basis) -> Shape* { return new RoundedRectangleShape(basis); }
    );
}
DiamondShape::DiamondShape(const int& basis)
    : Shape(ShapeTypes::Diamond, basis)
{
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
    drawText(painter);
}
QPolygon DiamondShape::createDiamondPolygon() const
{
    int w = m_rect.width();
    int h = m_rect.height();
    QPolygon polygon;
    polygon << QPoint(m_rect.left() + w/2, m_rect.top());
    polygon << QPoint(m_rect.right(), m_rect.top() + h/2);
    polygon << QPoint(m_rect.left() + w/2, m_rect.bottom());
    polygon << QPoint(m_rect.left(), m_rect.top() + h/2);
    return polygon;
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
    drawText(painter);
}
QPolygon HexagonShape::createHexagonPolygon() const
{
    int w = m_rect.width();
    int h = m_rect.height();
    QPolygon polygon;
    polygon << QPoint(m_rect.left() + w/4, m_rect.top());
    polygon << QPoint(m_rect.left() + 3*w/4, m_rect.top());
    polygon << QPoint(m_rect.right(), m_rect.top() + h/2);
    polygon << QPoint(m_rect.left() + 3*w/4, m_rect.bottom());
    polygon << QPoint(m_rect.left() + w/4, m_rect.bottom());
    polygon << QPoint(m_rect.left(), m_rect.top() + h/2);
    return polygon;
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
OctagonShape::OctagonShape(const int& basis)
    : Shape(ShapeTypes::Octagon, basis)
{
    int size = basis * 1.2;
    m_rect = QRect(0, 0, size, size);
}
void OctagonShape::paint(QPainter* painter)
{
    painter->save();
    setupPainter(painter);
    painter->drawPolygon(createOctagonPolygon());
    painter->restore();
    drawText(painter);
}
QPolygon OctagonShape::createOctagonPolygon() const
{
    int w = m_rect.width();
    int h = m_rect.height();
    QPolygon polygon;
    int wOffset = w / 4; 
    int HOffset = h / 4; 
    polygon << QPoint(m_rect.left() + wOffset, m_rect.top());
    polygon << QPoint(m_rect.right() - wOffset, m_rect.top());
    polygon << QPoint(m_rect.right(), m_rect.top() + HOffset);
    polygon << QPoint(m_rect.right(), m_rect.bottom() - HOffset);
    polygon << QPoint(m_rect.right() - wOffset, m_rect.bottom());
    polygon << QPoint(m_rect.left() + wOffset, m_rect.bottom());
    polygon << QPoint(m_rect.left(), m_rect.bottom() - HOffset);
    polygon << QPoint(m_rect.left(), m_rect.top() + HOffset);
    return polygon;
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
CloudShape::CloudShape(const int& basis)
    : Shape(ShapeTypes::Cloud, basis)
{
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
    drawText(painter);
}
bool CloudShape::contains(const QPoint& point) const
{
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
        outTopmost = outBottommost = outLeftmost = outRightmost = QPointF(); 
        return;
    }
    outTopmost    = path.pointAtPercent(0.0);
    outBottommost = path.pointAtPercent(0.0);
    outLeftmost   = path.pointAtPercent(0.0);
    outRightmost  = path.pointAtPercent(0.0);
    if (numberOfSamples < 2) {
        numberOfSamples = 2; 
    }
    for (int i = 0; i <= numberOfSamples; ++i) { 
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
    prototypeCloudPath.cubicTo(QPointF(40, 130), QPointF(65, 130), QPointF(75, 108));
    prototypeCloudPath.cubicTo(QPointF(85, 130), QPointF(115, 140), QPointF(125, 108));
    QPointF endOfBottom(170, 105);
    prototypeCloudPath.cubicTo(QPointF(135, 130), QPointF(160, 130), endOfBottom);
    prototypeCloudPath.cubicTo(QPointF(200, 100), QPointF(200, 65), QPointF(175, 45));
    prototypeCloudPath.cubicTo(QPointF(180, 15), QPointF(140, 10), QPointF(110, 30));
    prototypeCloudPath.cubicTo(QPointF(80, 5), QPointF(40, 15), QPointF(35, 45));
    prototypeCloudPath.cubicTo(QPointF(0, 55), QPointF(0, 90), pointA);
    prototypeCloudPath.closeSubpath();
    if (m_rect.width() <= 0 || m_rect.height() <= 0) {
        return QPainterPath(); 
    }
    QRectF prototypeCloudBoundingRect =prototypeCloudPath.boundingRect();
    qreal protoX = prototypeCloudBoundingRect.left();
    qreal protoY = prototypeCloudBoundingRect.top();
    qreal protoWidth = prototypeCloudBoundingRect.width();
    qreal protoHeight = prototypeCloudBoundingRect.height();
    qreal scaleX = targetRect.width() / protoWidth;
    qreal scaleY = targetRect.height() / protoHeight;
    qreal final_dx = targetRect.left() - protoX * scaleX;
    qreal final_dy = targetRect.top() - protoY * scaleY;
    QTransform transform(scaleX,   
                         0,        
                         0,        
                         scaleY,   
                         final_dx, 
                         final_dy  
                         );
    return transform.map(prototypeCloudPath);
}
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
void Shape::setTransparency(int transparency)
{
    m_transparency = qBound(0, transparency, 100);
}
int Shape::transparency() const
{
    return m_transparency;
}
void Shape::setLineWidth(qreal width)
{
    m_lineWidth = qMax(0.0, width);
}
qreal Shape::lineWidth() const
{
    return m_lineWidth;
}
void Shape::setLineStyle(int style)
{
    m_lineStyle = qBound(0, style, 3);
}
int Shape::lineStyle() const
{
    return m_lineStyle;
}
void Shape::setupPainter(QPainter* painter) const
{
    int alpha = qRound(m_transparency * 2.55); 
    QColor fillColorWithAlpha = m_fillColor;
    fillColorWithAlpha.setAlpha(alpha);
    painter->setBrush(QBrush(fillColorWithAlpha));
    QColor lineColorWithAlpha = m_lineColor;
    lineColorWithAlpha.setAlpha(alpha);
    QPen pen(lineColorWithAlpha);
    pen.setWidthF(m_lineWidth);
    switch(m_lineStyle) {
    case 0: 
        pen.setStyle(Qt::SolidLine);
        break;
    case 1: 
        {
            QVector<qreal> pattern;
            pattern << 3.0 << 3.0;
            pen.setDashPattern(pattern);
        }
        break;
    case 2: 
        {
            QVector<qreal> pattern;
            pattern << 8.0 << 3.0;
            pen.setDashPattern(pattern);
        }
        break;
    case 3: 
        {
            QVector<qreal> pattern;
            pattern << 7.0 << 3.0 << 2.0 << 3.0;
            pen.setDashPattern(pattern);
        }
        break;
    }
    painter->setPen(pen);
}
