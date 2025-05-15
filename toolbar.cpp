#include "toolbar.h"
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QIcon>
#include "chart/shape.h"
ShapeItem::ShapeItem(const QString& type, QWidget* parent)
    : QWidget(parent), m_type(type)
{
    setFixedSize(46, 46);
    if ( type == ShapeTypes::Circle   || type == ShapeTypes::Octagon || 
        type == ShapeTypes::Pentagon) {
        m_shapeRect = QRect(11, 11, 24, 24);
    } else if (type == ShapeTypes::Ellipse||type == ShapeTypes::Rectangle|| 
        type == ShapeTypes::RoundedRectangle|| type == ShapeTypes::Diamond||
        type == ShapeTypes::Cloud|| type == ShapeTypes::Hexagon) {
        m_shapeRect = QRect(8, 15, 30,18);
    } else if (type == ShapeTypes::ArrowLine) {
        m_shapeRect = QRect(6, 23, 32, 1); 
    } else {
        m_shapeRect = QRect(11, 11, 24, 24);
    }
    if (type == ShapeTypes::Rectangle) {
        setToolTip(tr("Rectangle"));
    } else if (type == ShapeTypes::Circle) {
        setToolTip(tr("Circle"));
    } else if (type == ShapeTypes::Pentagon) {
        setToolTip(tr("Pentagon"));
    } else if (type == ShapeTypes::Ellipse) {
        setToolTip(tr("Ellipse"));
    } else if (type == ShapeTypes::ArrowLine) {
        setToolTip(tr("ArrowLine"));
    } else if (type == ShapeTypes::RoundedRectangle) {
        setToolTip(tr("RoundedRectangle"));
    } else if (type == ShapeTypes::Diamond) {
        setToolTip(tr("Diamond"));
    } else if (type == ShapeTypes::Hexagon) {
        setToolTip(tr("Hexagon"));
    } else if (type == ShapeTypes::Octagon) {
        setToolTip(tr("Octagon"));
    } else if (type == ShapeTypes::Cloud) {
        setToolTip(tr("Cloud"));
    }
    setMouseTracking(true);
}
void ShapeItem::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor bgColor = QColor(250, 250, 250);
    painter.fillRect(rect(), bgColor);
    painter.setPen(QPen(QColor(220, 220, 220), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    painter.setPen(QPen(Qt::black, 1.5));
    painter.setBrush(QBrush(QColor(255, 255, 255)));
    if (m_type == ShapeTypes::Rectangle) {
        painter.drawRect(m_shapeRect);
    } else if (m_type == ShapeTypes::Circle) {
        painter.drawEllipse(m_shapeRect);
    } else if (m_type == ShapeTypes::Pentagon) {
        QPolygon polygon;
        int centerX = m_shapeRect.center().x();
        int centerY = m_shapeRect.center().y();
        int radius = qMin(m_shapeRect.width(), m_shapeRect.height()) / 2;
        for (int i = 0; i < 5; ++i) {
            double angle = i * 2 * M_PI / 5 - M_PI / 2;
            int x = centerX + radius * cos(angle);
            int y = centerY + radius * sin(angle);
            polygon << QPoint(x, y);
        }
        painter.drawPolygon(polygon);
    } else if (m_type == ShapeTypes::Ellipse) {
        painter.drawEllipse(m_shapeRect);
    } else if (m_type == ShapeTypes::ArrowLine) {
        QPoint startPoint = m_shapeRect.topLeft();
        QPoint endPoint = m_shapeRect.topRight();
        painter.setPen(QPen(Qt::black, 1));
        painter.drawLine(startPoint, endPoint);
        const int arrowSize = 6;
        QPoint arrowP1 = endPoint - QPoint(arrowSize, arrowSize / 2);
        QPoint arrowP2 = endPoint - QPoint(arrowSize, -arrowSize / 2);
        QPolygon arrow;
        arrow << endPoint << arrowP1 << arrowP2;
        painter.setBrush(Qt::black);
        painter.drawPolygon(arrow);
    } else if (m_type == ShapeTypes::RoundedRectangle) {
        int radius = m_shapeRect.height() / 6;
        painter.drawRoundedRect(m_shapeRect, radius, radius);
    } else if (m_type == ShapeTypes::Diamond) {
        QPolygon polygon;
        int w = m_shapeRect.width();
        int h = m_shapeRect.height();
        polygon << QPoint(m_shapeRect.left() + w/2, m_shapeRect.top());
        polygon << QPoint(m_shapeRect.right(), m_shapeRect.top() + h/2);
        polygon << QPoint(m_shapeRect.left() + w/2, m_shapeRect.bottom());
        polygon << QPoint(m_shapeRect.left(), m_shapeRect.top() + h/2);
        painter.drawPolygon(polygon);
    } else if (m_type == ShapeTypes::Hexagon) {
        QPolygon polygon;
        int w = m_shapeRect.width();
        int h = m_shapeRect.height();
        polygon << QPoint(m_shapeRect.left() + w/4, m_shapeRect.top());
        polygon << QPoint(m_shapeRect.left() + 3*w/4, m_shapeRect.top());
        polygon << QPoint(m_shapeRect.right(), m_shapeRect.top() + h/2);
        polygon << QPoint(m_shapeRect.left() + 3*w/4, m_shapeRect.bottom());
        polygon << QPoint(m_shapeRect.left() + w/4, m_shapeRect.bottom());
        polygon << QPoint(m_shapeRect.left(), m_shapeRect.top() + h/2);
        painter.drawPolygon(polygon);
    } else if (m_type == ShapeTypes::Octagon) {
        int w = m_shapeRect.width();
        int h = m_shapeRect.height();
        QPolygon polygon;
        int wOffset = w / 4; 
        int HOffset = h / 4; 
        polygon << QPoint(m_shapeRect.left() + wOffset, m_shapeRect.top());
        polygon << QPoint(m_shapeRect.right() - wOffset, m_shapeRect.top());
        polygon << QPoint(m_shapeRect.right(), m_shapeRect.top() + HOffset);
        polygon << QPoint(m_shapeRect.right(), m_shapeRect.bottom() - HOffset);
        polygon << QPoint(m_shapeRect.right() - wOffset, m_shapeRect.bottom());
        polygon << QPoint(m_shapeRect.left() + wOffset, m_shapeRect.bottom());
        polygon << QPoint(m_shapeRect.left(), m_shapeRect.bottom() - HOffset);
        polygon << QPoint(m_shapeRect.left(), m_shapeRect.top() + HOffset);
        painter.drawPolygon(polygon);
    } else if (m_type == ShapeTypes::Cloud) {
        QRectF targetRect(m_shapeRect);
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
        painter.drawPath(transform.map(prototypeCloudPath));
    }
}
void ShapeItem::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QDrag* drag = new QDrag(this);
        QMimeData* mimeData = new QMimeData;
        mimeData->setText(m_type);
        drag->setMimeData(mimeData);
        QRect tempRect(
            m_shapeRect.topLeft().x() - m_shapeRect.width()/4,   
            m_shapeRect.topLeft().y() - m_shapeRect.height()/4,  
            m_shapeRect.width() * 2,    
            m_shapeRect.height() * 2    
        );
        QSize dragSize(92, 92); 
        QPixmap pixmap(dragSize);
        pixmap.fill(Qt::transparent); 
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(Qt::black, 1.5));
        painter.setBrush(QBrush(QColor(255, 255, 255)));
        tempRect.moveCenter(QPoint(dragSize.width()/2, dragSize.height()/2));
        if (m_type == ShapeTypes::Rectangle) {
            painter.drawRect(tempRect);
        } else if (m_type == ShapeTypes::Circle) {
            painter.drawEllipse(tempRect);
        } else if (m_type == ShapeTypes::Pentagon) {
            QPolygon polygon;
            int centerX = tempRect.center().x();
            int centerY = tempRect.center().y();
            int radius = qMin(tempRect.width(), tempRect.height()) / 2;
            for (int i = 0; i < 5; ++i) {
                double angle = i * 2 * M_PI / 5 - M_PI / 2;
                int x = centerX + radius * cos(angle);
                int y = centerY + radius * sin(angle);
                polygon << QPoint(x, y);
            }
            painter.drawPolygon(polygon);
        } else if (m_type == ShapeTypes::Ellipse) {
            painter.drawEllipse(tempRect);
        } else if (m_type == ShapeTypes::ArrowLine) {
            QPoint startPoint = tempRect.topLeft();
            QPoint endPoint = tempRect.topRight();
            painter.setPen(QPen(Qt::black, 1));
            painter.drawLine(startPoint, endPoint);
            const int arrowSize = 6;
            QPoint arrowP1 = endPoint - QPoint(arrowSize, arrowSize / 2);
            QPoint arrowP2 = endPoint - QPoint(arrowSize, -arrowSize / 2);
            QPolygon arrow;
            arrow << endPoint << arrowP1 << arrowP2;
            painter.setBrush(Qt::black);
            painter.drawPolygon(arrow);
        } else if (m_type == ShapeTypes::RoundedRectangle) {
            int radius = tempRect.height() / 6;
            painter.drawRoundedRect(tempRect, radius, radius);
        } else if (m_type == ShapeTypes::Diamond) {
            QPolygon polygon;
            int w = tempRect.width();
            int h = tempRect.height();
            polygon << QPoint(tempRect.left() + w/2, tempRect.top());
            polygon << QPoint(tempRect.right(), tempRect.top() + h/2);
            polygon << QPoint(tempRect.left() + w/2, tempRect.bottom());
            polygon << QPoint(tempRect.left(), tempRect.top() + h/2);
            painter.drawPolygon(polygon);
        } else if (m_type == ShapeTypes::Hexagon) {
            QPolygon polygon;
            int w = tempRect.width();
            int h = tempRect.height();
            polygon << QPoint(tempRect.left() + w/4, tempRect.top());
            polygon << QPoint(tempRect.left() + 3*w/4, tempRect.top());
            polygon << QPoint(tempRect.right(), tempRect.top() + h/2);
            polygon << QPoint(tempRect.left() + 3*w/4, tempRect.bottom());
            polygon << QPoint(tempRect.left() + w/4, tempRect.bottom());
            polygon << QPoint(tempRect.left(), tempRect.top() + h/2);
            painter.drawPolygon(polygon);
        } else if (m_type == ShapeTypes::Octagon) {
            int w = tempRect.width();
            int h = tempRect.height();
            QPolygon polygon;
            int wOffset = w / 4; 
            int HOffset = h / 4; 
            polygon << QPoint(tempRect.left() + wOffset, tempRect.top());
            polygon << QPoint(tempRect.right() - wOffset, tempRect.top());
            polygon << QPoint(tempRect.right(), tempRect.top() + HOffset);
            polygon << QPoint(tempRect.right(), tempRect.bottom() - HOffset);
            polygon << QPoint(tempRect.right() - wOffset, tempRect.bottom());
            polygon << QPoint(tempRect.left() + wOffset, tempRect.bottom());
            polygon << QPoint(tempRect.left(), tempRect.bottom() - HOffset);
            polygon << QPoint(tempRect.left(), tempRect.top() + HOffset);
            painter.drawPolygon(polygon);
        } else if (m_type == ShapeTypes::Cloud) {
            QRectF targetRect(tempRect);
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
            painter.drawPath(transform.map(prototypeCloudPath));
        }
        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(dragSize.width()/2, dragSize.height()/2));
        drag->exec(Qt::CopyAction);
    }
}
void ShapeItem::mouseMoveEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
}
ShapeCategory::ShapeCategory(const QString& title, QWidget* parent)
    : QWidget(parent), m_title(title), m_currentRow(0), m_currentColumn(0)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 0);
    m_mainLayout->setSpacing(5);
    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setStyleSheet("font-size: 12px; font-weight: bold; padding-bottom: 5px;");
    m_mainLayout->addWidget(m_titleLabel);
    m_shapesLayout = new QGridLayout();
    m_shapesLayout->setContentsMargins(0, 0, 0, 0);
    m_shapesLayout->setSpacing(2);
    m_mainLayout->addLayout(m_shapesLayout);
}
void ShapeCategory::addShape(const QString& type)
{
    ShapeItem* item = new ShapeItem(type, this);
    m_shapesLayout->addWidget(item, m_currentRow, m_currentColumn);
    m_currentColumn++;
    if (m_currentColumn >= 5) {
        m_currentColumn = 0;
        m_currentRow++;
    }
}
ToolBar::ToolBar(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}
void ToolBar::setupUI()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_tabBar = new QTabBar(this);
    m_tabBar->addTab(tr("Shape library"));
    m_tabBar->setExpanding(true);  
    m_tabBar->setDocumentMode(true);
    m_tabBar->setDrawBase(false);
    m_tabBar->setStyleSheet("QTabBar::tab { padding: 8px 0; border: none; border-bottom: 1px solid #e0e0e0; background-color: white; }"
                          "QTabBar::tab:selected { border-bottom: 2px solid #1a73e8; color: #1a73e8; }"
                          "QTabBar::tab:hover:!selected { background-color: #f5f5f5; }");
    m_layout->addWidget(m_tabBar);
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText(tr("Please enter search content"));
    m_searchBox->setStyleSheet("QLineEdit { border: 1px solid #e0e0e0; border-radius: 3px; padding: 5px; margin: 8px; }");
    m_layout->addWidget(m_searchBox);
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_layout->addWidget(m_scrollArea, 1); 
    m_libraryWidget = new QWidget(this);
    createGraphicsLibTab();
    m_scrollArea->setWidget(m_libraryWidget);
}
void ToolBar::createGraphicsLibTab()
{
    QVBoxLayout* layout = new QVBoxLayout(m_libraryWidget);
    layout->setContentsMargins(5, 0, 5, 5);
    layout->setSpacing(10);
    ShapeCategory* basicCategory = new ShapeCategory(tr("Basic shapes"), m_libraryWidget);
    basicCategory->addShape(ShapeTypes::Rectangle);
    basicCategory->addShape(ShapeTypes::Circle);
    basicCategory->addShape(ShapeTypes::Ellipse);
    basicCategory->addShape(ShapeTypes::Pentagon);
    basicCategory->addShape(ShapeTypes::ArrowLine);
    basicCategory->addShape(ShapeTypes::RoundedRectangle);
    basicCategory->addShape(ShapeTypes::Diamond);
    basicCategory->addShape(ShapeTypes::Hexagon);
    basicCategory->addShape(ShapeTypes::Octagon);
    basicCategory->addShape(ShapeTypes::Cloud);
    layout->addWidget(basicCategory);
    ShapeCategory* flowchartCategory = new ShapeCategory(tr("Flowchart"), m_libraryWidget);
    flowchartCategory->addShape(ShapeTypes::Pentagon);
    flowchartCategory->addShape(ShapeTypes::Octagon);
    flowchartCategory->addShape(ShapeTypes::Hexagon);
    flowchartCategory->addShape(ShapeTypes::RoundedRectangle);
    flowchartCategory->addShape(ShapeTypes::Diamond);
    layout->addWidget(flowchartCategory);
    layout->addStretch();
}
