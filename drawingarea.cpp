#include "drawingarea.h"
#include <QPainter>
#include <QMimeData>
#include <QKeyEvent>
#include <QApplication>
#include <QContextMenuEvent>
#include <QScrollArea>
#include <algorithm>  
#include <QFontMetrics>
#include <QTextCharFormat>
#include <QTimer>
#include <QSvgGenerator>
#include <QDomDocument>
#include <QFile>
#include <QSvgRenderer>
#include <QDebug> 
#include "chart/shapefactory.h"
#include "chart/customtextedit.h"
#include "chart/shape.h"
#include "chart/connection.h"




DrawingArea::DrawingArea(QWidget *parent)
    : QWidget(parent),
      m_selectedShape(nullptr),
      m_dragging(false),
      m_scale(1.0),
      m_isPanning(false),
      m_activeHandle(Shape::None),
      m_resizing(false),
      m_textEditor(nullptr),
      m_currentConnection(nullptr),
      m_hoveredShape(nullptr),
      m_selectedConnection(nullptr),
      m_shapeContextMenu(nullptr),
      m_canvasContextMenu(nullptr),
      m_copiedShape(nullptr),
      m_copiedShapes(),
      m_copiedShapesPositions(),
      m_movingConnectionPoint(false),
      m_activeConnectionPoint(nullptr),
      m_backgroundColor(Qt::white),
      m_drawingAreaSize(Default_WIDTH, Default_HEIGHT),
      m_showGrid(true),
      m_gridColor(QColor(220, 220, 220)),
      m_gridSize(15),
      m_gridThickness(2),
      m_multiSelectedShapes(),
      m_multySelectedConnections(),
      m_multyShapesStartPos(),
      m_isMultiRectSelecting(false),
      m_multiSelectionStart(),
      m_multiSelectionRect()
{
    setAcceptDrops(true);
    setMouseTracking(true);
    m_backgroundColor = Qt::white;
    m_drawingAreaSize = QSize(Default_WIDTH, Default_HEIGHT); 
    m_showGrid = true;
    m_gridColor = QColor(220, 220, 220);
    m_gridSize = 15;
    m_gridThickness = 2;
    setStyleSheet("DrawingArea { background-color: white; }");
    setFocusPolicy(Qt::StrongFocus);
    createTextEditor();
    createContextMenus();
    setMinimumSize(m_drawingAreaSize.width() * 3, m_drawingAreaSize.height() * 3);
    QTimer::singleShot(0, this, [this]() {
        setScale(m_scale);
    });
    QTimer::singleShot(100, this, [this]() {
        centerDrawingArea();
    });
}
DrawingArea::~DrawingArea()
{
    for (Shape* shape : m_shapes) {
        delete shape;
    }
    for (Connection* connection : m_connections) {
        delete connection;
    }
    if (m_currentConnection) {
        delete m_currentConnection;
    }
    if (m_copiedShape) {
        delete m_copiedShape;
    }
    for (Shape* shape : m_copiedShapes) {
        delete shape;
    }
    for (Connection* conn : m_copiedConnections) {
        delete conn;
    }
    if (m_shapeContextMenu) {
        delete m_shapeContextMenu;
    }
    if (m_canvasContextMenu) {
        delete m_canvasContextMenu;
    }
    if (m_textEditor) {
        m_textEditor->deleteLater();
    }
}

void DrawingArea::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(230, 230, 230));
    painter.save();
    QRectF drawingRect(
        (width() - m_drawingAreaSize.width() * m_scale) / 2,
        (height() - m_drawingAreaSize.height() * m_scale) / 2,
        m_drawingAreaSize.width() * m_scale,
        m_drawingAreaSize.height() * m_scale
    );
    painter.translate(drawingRect.topLeft());
    painter.scale(m_scale, m_scale);
    painter.translate(-m_viewOffset);
    QRectF bgRect(0, 0, m_drawingAreaSize.width(), m_drawingAreaSize.height());
    painter.fillRect(bgRect, m_backgroundColor);
    if (m_showGrid) {
        painter.setClipRect(bgRect);
        drawGrid(&painter);
        painter.setClipping(false);
    }
    for (Shape *shape : m_shapes) {
        shape->paint(&painter);
        if(m_hoveredShape==shape){
            shape->drawConnectionPoints(&painter);
        }
        if (shape == m_selectedShape) {
            painter.setRenderHint(QPainter::Antialiasing, false);
            QPen dashPen(Qt::blue, 0); 
            QVector<qreal> dashes;
            dashes << 2 << 2;
            dashPen.setDashPattern(dashes);
            dashPen.setStyle(Qt::CustomDashLine); 
            painter.setPen(dashPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(shape->getRect());
            painter.setRenderHint(QPainter::Antialiasing, true);
            shape->drawResizeHandles(&painter);
        }
        if (m_multiSelectedShapes.contains(shape)) {
            painter.setRenderHint(QPainter::Antialiasing, false);
            QPen dashPen(Qt::blue, 0); 
            QVector<qreal> dashes;
            dashes << 2 << 2;
            dashPen.setDashPattern(dashes);
            dashPen.setStyle(Qt::CustomDashLine);
            painter.setPen(dashPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(shape->getRect());
            painter.setRenderHint(QPainter::Antialiasing, true);
        }
    }
    for (Connection *connection : m_connections) {
        if (m_movingConnectionPoint && connection == m_selectedConnection) {
            drawConnectionPreview(&painter, connection);
        } else {
            connection->paint(&painter);
        }
    }
    if (m_currentConnection) {
        m_currentConnection->paint(&painter);
    }
    painter.restore();
    if (m_isMultiRectSelecting) {
        drawMultiSelectionRect(&painter);
    }
}
void DrawingArea::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}
void DrawingArea::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasText()) {
        event->acceptProposedAction();
    }
}
void DrawingArea::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasText()) {
        QString type = event->mimeData()->text();
        QPoint scenePos = mapToScene(event->pos());
        if (type == ShapeTypes::ArrowLine) {
            QPoint center = scenePos;
            QPoint startPoint = center - QPoint(40, 0);
            QPoint endPoint = center + QPoint(40, 0);
            createArrowLine(startPoint, endPoint);
            event->acceptProposedAction();
            emit shapesCountChanged(getShapesCount());
            return;
        }
        int basis = 85; 
        Shape *newShape = ShapeFactory::instance().createShape(type, basis);
        if (newShape) {
            QRect shapeRect = newShape->getRect();
            shapeRect.moveCenter(scenePos);
            newShape->setRect(shapeRect);
            m_selectedShape = newShape; 
            m_shapes.append(newShape);
            emit shapesCountChanged(getShapesCount());        
            emit shapeSelectionChanged(true);
            update();
        }
        event->acceptProposedAction();
    }
}
void DrawingArea::updateCursor(QMouseEvent *event){
}
void DrawingArea::mouseMoveEvent(QMouseEvent *event)
{
    QPoint scenePos = mapToScene(event->pos());
    m_lastMousePos = event->pos();
    if (m_isMultiRectSelecting) {
        updateRectMultiSelection(event->pos());
        return;
    }
    if (m_currentConnection) {
        m_temporaryEndPoint = scenePos;
        m_currentConnection->setTemporaryEndPoint(scenePos);
        bool isOverShape = false;
        for (int i = m_shapes.size() - 1; i >= 0; --i) {
            ConnectionPoint* cp = m_shapes[i]->hitConnectionPoint(scenePos, false);
            if(cp || m_shapes[i]->contains(scenePos)) {
                m_hoveredShape = m_shapes[i];
                setCursor(Qt::ArrowCursor); 
                isOverShape = true;
                break;
            }
        }
        if(!isOverShape) {
            m_hoveredShape = nullptr;
        }
        update();
        return;
    }
    if (m_movingConnectionPoint && m_activeConnectionPoint) {
        m_connectionDragPoint = scenePos;
        bool isOverShape = false;
        for (int i = m_shapes.size() - 1; i >= 0; --i) {
            ConnectionPoint* cp = m_shapes[i]->hitConnectionPoint(scenePos, false);
            if (cp) {
                m_hoveredShape = m_shapes[i];
                setCursor(Qt::CrossCursor);
                isOverShape = true;
                break;
            } else if (m_shapes[i]->contains(scenePos)) {
                m_hoveredShape = m_shapes[i];
                setCursor(Qt::ArrowCursor);
                isOverShape = true;
                break;
            }
        }
        if (!isOverShape) {
            if (m_activeConnectionPoint->getOwner() == nullptr) {
                m_activeConnectionPoint->setPosition(scenePos);
            }
            m_hoveredShape = nullptr;
        }
        update();
        return;
    }
    if (m_resizing && m_selectedShape) {
        QPoint delta = event->pos() - m_dragStart;
        QPoint sceneDelta = mapToScene(delta) - mapToScene(QPoint(0, 0));
        m_selectedShape->resize(m_activeHandle, sceneDelta);
        m_dragStart = event->pos();
        emit shapeSizeChanged(m_selectedShape->getRect().size());
        update();
        return;
    }
    if (m_dragging) {
        QPoint delta = event->pos() - m_dragStart;
        QPoint sceneDelta = mapToScene(delta) - mapToScene(QPoint(0, 0));
        if (m_selectedShape) {
            QRect newRect = m_selectedShape->getRect();
            newRect.moveTo(m_shapeStart + sceneDelta);
            m_selectedShape->setRect(newRect);
            emit shapePositionChanged(newRect.topLeft());
        } else if (!m_multiSelectedShapes.isEmpty()) {
            for (int i = 0; i < m_multiSelectedShapes.size(); ++i) {
                QRect newRect = m_multiSelectedShapes[i]->getRect();
                newRect.moveTo(m_multyShapesStartPos[i] + sceneDelta);
                m_multiSelectedShapes[i]->setRect(newRect);
            }
        } else if (m_selectedConnection) {
            Connection* conn = m_selectedConnection;
            if (conn->getStartPoint() && conn->getEndPoint() && 
                conn->getStartPoint()->getOwner() == nullptr && 
                conn->getEndPoint()->getOwner() == nullptr) {
                QPoint startPos = conn->getStartPosition();
                QPoint endPos = conn->getEndPosition();
                QPoint newStartPos = startPos + sceneDelta;
                QPoint newEndPos = endPos + sceneDelta;
                conn->getStartPoint()->setPosition(newStartPos);
                conn->getEndPoint()->setPosition(newEndPos);
                m_dragStart = event->pos();
            }
        }
        update();
        return;
    }
    for (int i = m_connections.size() - 1; i >= 0; --i) {
        Connection* conn = m_connections[i];
        if (conn->isNearStartPoint(scenePos, 20)) {
            if (conn->getStartPoint()->getOwner() == nullptr || 
                !conn->getStartPoint()->getOwner()->hitConnectionPoint(scenePos, true)) {
                setCursor(Qt::SizeAllCursor); 
                return;
            }
        } else if (conn->isNearEndPoint(scenePos, 20)) {
            if (conn->getEndPoint()->getOwner() == nullptr || 
                !conn->getEndPoint()->getOwner()->hitConnectionPoint(scenePos, true)) {
                setCursor(Qt::SizeAllCursor); 
                return;
            }
        } else if (conn->contains(scenePos)) {
            if (conn->getStartPoint()->getOwner() == nullptr && 
                conn->getEndPoint()->getOwner() == nullptr) {
                setCursor(Qt::SizeAllCursor);
            } else {
                setCursor(Qt::PointingHandCursor);
            }
            return;
        }
    }
    if (m_selectedShape) {
        Shape::HandlePosition handle = m_selectedShape->hitHandle(scenePos);
        if (handle != Shape::None) {
            switch (handle) {
                case Shape::TopLeft:
                case Shape::BottomRight:
                    setCursor(Qt::SizeFDiagCursor); 
                    return;
                case Shape::TopRight:
                case Shape::BottomLeft:
                    setCursor(Qt::SizeBDiagCursor); 
                    return;
                case Shape::Top:
                case Shape::Bottom:
                    setCursor(Qt::SizeVerCursor); 
                    return;
                case Shape::Left:
                case Shape::Right:
                    setCursor(Qt::SizeHorCursor); 
                    return;
            }
        }   
    }
    for (int i = m_shapes.size() - 1; i >= 0; --i) {
        ConnectionPoint* cp = m_shapes[i]->hitConnectionPoint(scenePos, true);
        if (cp && m_selectedShape != m_shapes[i]) {
            setCursor(Utils::getCrossCursor()); 
            if (m_hoveredShape != m_shapes[i]) {
                m_hoveredShape = m_shapes[i];
                update(); 
            }
            return;
        } else if (m_shapes[i]->contains(scenePos)) {
            if (m_hoveredShape != m_shapes[i]) {
                m_hoveredShape = m_shapes[i];
                update(); 
            }
            setCursor(Qt::SizeAllCursor); 
            return;
        }
    }
    setCursor(Qt::ArrowCursor);
    if (m_hoveredShape) {
        m_hoveredShape = nullptr;
        update();
    }
}
void DrawingArea::mousePressEvent(QMouseEvent *event)
{
    setFocus();
    if (m_textEditor && m_textEditor->isVisible()) {
        QRect editorRect = m_textEditor->geometry();
        if (!editorRect.contains(event->pos())) {
            finishTextEditing();
        }
        return;
    }
    if (event->button() == Qt::LeftButton) {
        QPoint scenePos = mapToScene(event->pos());
        if (m_hoveredShape && m_hoveredShape!=m_selectedShape){
            ConnectionPoint* cp = m_hoveredShape->hitConnectionPoint(scenePos, true);
            if (cp) {
                startConnection(cp);
                return;
            }
        }
        for (int i = m_connections.size() - 1; i >= 0; --i) {
            Connection* conn = m_connections[i];
            if (conn->isNearStartPoint(scenePos, 20)) {
                if (conn->getStartPoint()->getOwner() == nullptr || 
                    !conn->getStartPoint()->getOwner()->hitConnectionPoint(scenePos, true)) {
                    selectConnection(conn);
                    m_movingConnectionPoint = true;
                    m_activeConnectionPoint = conn->getStartPoint();
                    m_dragStart = event->pos();
                    m_connectionDragPoint = scenePos;
                    setCursor(Qt::SizeAllCursor); 
                    return;
                }
            } else if (conn->isNearEndPoint(scenePos, 20)) {
                if (conn->getEndPoint()->getOwner() == nullptr || 
                    !conn->getEndPoint()->getOwner()->hitConnectionPoint(scenePos, true)) {
                    selectConnection(conn);
                    m_movingConnectionPoint = true;
                    m_activeConnectionPoint = conn->getEndPoint();
                    m_dragStart = event->pos();
                    m_connectionDragPoint = scenePos;
                    setCursor(Qt::SizeAllCursor); 
                    return;
                }
            } else if (conn->contains(scenePos)) {
                bool isIndependentLine = 
                    (conn->getStartPoint()->getOwner() == nullptr && 
                     conn->getEndPoint()->getOwner() == nullptr);
                selectConnection(conn);
                if (isIndependentLine) {
                    m_dragging = true;
                    m_dragStart = event->pos();
                }
                return;
            }
        }
        if (m_selectedShape) {
            Shape::HandlePosition handle = m_selectedShape->hitHandle(scenePos);
            if (handle != Shape::None) {
                m_activeHandle = handle;
                m_resizing = true;
                m_dragStart = event->pos();
                return;
            }
        }
        if (!m_multiSelectedShapes.isEmpty()) {
            bool clickedSelectedShape = false;
            for (int i = 0; i < m_multiSelectedShapes.size(); ++i) {
                if (m_multiSelectedShapes[i]->contains(scenePos)) {
                    m_dragging = true;
                    m_dragStart = event->pos();
                    m_multyShapesStartPos.clear();
                    for (int j = 0; j < m_multiSelectedShapes.size(); ++j) {
                        m_multyShapesStartPos.append(m_multiSelectedShapes[j]->getRect().topLeft());
                    }
                    clickedSelectedShape = true;
                    break;
                }
            }
            if (clickedSelectedShape) {
                return;
            }
        }
        Shape* oldSelectedShape = m_selectedShape;  
        for (int i = m_shapes.size() - 1; i >= 0; --i) {
            if (m_shapes[i]->contains(scenePos)) {
                if (event->modifiers() & Qt::ControlModifier) {
                    if (m_shapes[i] == m_selectedShape) {
                        m_selectedShape = nullptr;
                        emit shapeSelectionChanged(false);
                    } else if (m_multiSelectedShapes.contains(m_shapes[i])) {
                        m_multiSelectedShapes.removeOne(m_shapes[i]);
                        if (m_multiSelectedShapes.isEmpty()) {
                            emit multiSelectionChanged(false);
                        }
                    } else {
                        if (m_selectedShape) {
                            m_multiSelectedShapes.append(m_selectedShape);
                            m_selectedShape = nullptr;
                        }
                        m_multiSelectedShapes.append(m_shapes[i]);
                        emit multiSelectionChanged(true);
                    }
                } else {
                    if (m_selectedConnection) {
                        m_selectedConnection->setSelected(false);
                        m_selectedConnection = nullptr;
                    }
                    if (!m_multiSelectedShapes.isEmpty()) {
                        m_multiSelectedShapes.clear();
                        emit multiSelectionChanged(false);
                    }
                    m_selectedShape = m_shapes[i];
                    m_dragging = true;
                    m_dragStart = event->pos();
                    m_shapeStart = m_selectedShape->getRect().topLeft();
                    if (oldSelectedShape != m_selectedShape) {
                        emit shapeSelectionChanged(true);
                    }
                }
                update();
                return;
            }
        }
        if (!(event->modifiers() & Qt::ControlModifier)) {
            if (m_selectedShape) {
                m_selectedShape = nullptr;
                update();
                emit shapeSelectionChanged(false);
            }
            if (!m_multiSelectedShapes.isEmpty()) {
                m_multiSelectedShapes.clear();
                update();
                emit multiSelectionChanged(false);
            }
            if (m_selectedConnection) {
                m_selectedConnection->setSelected(false);
                m_selectedConnection = nullptr;
                update();
            }
            startRectMultiSelection(event->pos());
        }
    }
}
void DrawingArea::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint scenePos = mapToScene(event->pos());
    if (m_isMultiRectSelecting && event->button() == Qt::LeftButton) {
        finishRectMultiSelection();
		update();
        return;
    }
    if (m_currentConnection && event->button() == Qt::LeftButton) {
        if (m_hoveredShape) {
            ConnectionPoint* nearestPoint = findNearestConnectionPoint(m_hoveredShape, scenePos);
            if (nearestPoint && (!m_currentConnection->getStartPoint()->equalTo(nearestPoint))) {
                completeConnection(nearestPoint);
            } else {
                completeConnection(nullptr);
            }
        } else {
            completeConnection(nullptr);
        }
        update();
        emit shapeSelectionChanged(m_selectedShape != nullptr);
        return;
    }
    if (m_movingConnectionPoint && event->button() == Qt::LeftButton) {
        if (m_hoveredShape) {
            ConnectionPoint* nearestPoint = findNearestConnectionPoint(m_hoveredShape, scenePos);
            if (nearestPoint) {
                Connection* conn = m_selectedConnection;
                bool isStartPoint = (m_activeConnectionPoint == conn->getStartPoint());
                if (isStartPoint) {
                    if (conn->getEndPoint() && !nearestPoint->equalTo(conn->getEndPoint())) {
                        conn->setStartPoint(nearestPoint);
                    }
                } else {
                    if (conn->getStartPoint() && !nearestPoint->equalTo(conn->getStartPoint())) {
                        conn->setEndPoint(nearestPoint);
                    }
                }
            } else {
            }
        } else {
            ConnectionPoint* freePoint = new ConnectionPoint(scenePos);
            Connection* conn = m_selectedConnection;
            bool isStartPoint = (m_activeConnectionPoint == conn->getStartPoint());
            if (isStartPoint) {
                conn->setStartPoint(freePoint);
            } else {
                conn->setEndPoint(freePoint);
            }
        }
        m_movingConnectionPoint = false;
        m_activeConnectionPoint = nullptr;
        setCursor(Qt::ArrowCursor);
        update();
        return;
    }
    if (m_resizing && event->button() == Qt::LeftButton) {
        m_resizing = false;
        m_activeHandle = Shape::None;
        setCursor(Qt::ArrowCursor);
        if (m_selectedShape) {
            emit shapeSizeChanged(m_selectedShape->getRect().size());
        }
        update();
        return;
    }
    if (m_dragging && event->button() == Qt::LeftButton) {
        m_dragging = false;
        m_multyShapesStartPos.clear(); 
        setCursor(Qt::ArrowCursor);
        if (m_selectedShape) {
            emit shapePositionChanged(m_selectedShape->getRect().topLeft());
        }
        update();
        return;
    }
}
void DrawingArea::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_textEditor && m_textEditor->isVisible()) {
        return;  
    }
    QPoint scenePos = mapToScene(event->pos());
    Shape* clickedShape = nullptr;
    for (int i = m_shapes.size() - 1; i >= 0; i--) {
        if (m_shapes.at(i)->contains(scenePos)) {
            clickedShape = m_shapes.at(i);
            break;
        }
    }
    if (clickedShape) {
        m_selectedShape = clickedShape;
        startTextEditing();
        update();
    }
}
void DrawingArea::createTextEditor()
{
    if (!m_textEditor) {
        m_textEditor = new CustomTextEdit(this);
        m_textEditor->setFrameStyle(QFrame::NoFrame);  
        m_textEditor->installEventFilter(this);
        m_textEditor->hide();
    }
}
void DrawingArea::startTextEditing()
{
if (!m_selectedShape) return;
    createTextEditor();
    QRect textRect = m_selectedShape->textRect();
    QPoint topLeft = mapFromScene(textRect.topLeft());
    QPoint bottomRight = mapFromScene(textRect.bottomRight());
    QRect viewRect(topLeft, bottomRight);
    m_textEditor->setGeometry(viewRect);
    m_textEditor->setPlainText(m_selectedShape->text());
    QColor fillColor = m_selectedShape->fillColor();
    QColor transparentFillColor = fillColor; 
    transparentFillColor.setAlphaF(0.0); 
    QString styleSheet = QString("background-color: rgba(%1, %2, %3, %4); border: none; font-family: '%5'; font-size: %6pt;")
                            .arg(transparentFillColor.red())
                            .arg(transparentFillColor.green())
                            .arg(transparentFillColor.blue())
                            .arg(transparentFillColor.alphaF()) 
                            .arg(m_selectedShape->fontFamily())
                            .arg(m_selectedShape->fontSize());
    m_textEditor->setStyleSheet(styleSheet);
    m_selectedShape->setEditing(true);
    m_textEditor->show();
    m_textEditor->setFocus();
}
void DrawingArea::finishTextEditing()
{
    if (!m_textEditor || !m_selectedShape) return;
    m_selectedShape->setText(m_textEditor->toPlainText());
    m_selectedShape->setEditing(false);
    m_textEditor->hide();
    update();
}
void DrawingArea::cancelTextEditing()
{
    if (!m_textEditor || !m_selectedShape) return;
    m_selectedShape->setEditing(false);
    m_textEditor->hide();
    update();
}
bool DrawingArea::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_textEditor) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                cancelTextEditing();
                return true;
            } else if (keyEvent->key() == Qt::Key_Return && 
                      (keyEvent->modifiers() & Qt::ControlModifier)) {
                finishTextEditing();
                return true;
            }
        } else if (event->type() == QEvent::FocusOut) {
            finishTextEditing();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
void DrawingArea::startConnection(ConnectionPoint* startPoint)
{
    m_currentConnection = new Connection(startPoint);
    QPoint cursorPos = mapToScene(mapFromGlobal(QCursor::pos()));
    m_temporaryEndPoint = cursorPos;
    m_currentConnection->setTemporaryEndPoint(cursorPos);
    update();
}
void DrawingArea::completeConnection(ConnectionPoint* endPoint)
{
    if (!m_currentConnection) {
        return;
    }
    if (endPoint) {
        m_currentConnection->setEndPoint(endPoint);
    } else {
        QPoint tempPos = m_currentConnection->getEndPosition(); 
        double distance = std::sqrt(std::pow(tempPos.x() - m_currentConnection->getStartPosition().x(), 2) + 
                                  std::pow(tempPos.y() - m_currentConnection->getStartPosition().y(), 2));
        if (distance < 20.0) {
            m_currentConnection = nullptr;
            m_selectedShape = nullptr;
            return;
        }
        ConnectionPoint* freeEndPoint = new ConnectionPoint(tempPos);
        m_currentConnection->setEndPoint(freeEndPoint);
    }
    if (m_currentConnection->isComplete()) {
        m_connections.append(m_currentConnection);
        selectConnection(m_currentConnection); 
        emit shapesCountChanged(getShapesCount());
    } else {
        delete m_currentConnection;
        m_currentConnection = nullptr;
    }
    m_currentConnection = nullptr;
    m_selectedShape = nullptr;
    if (m_hoveredShape) {
        m_hoveredShape = nullptr;
    }
}
void DrawingArea::cancelConnection()
{
    if (m_currentConnection) {
        delete m_currentConnection;
        m_currentConnection = nullptr;
    }
    if (m_hoveredShape) {
        m_hoveredShape = nullptr;
    }
}
ConnectionPoint* DrawingArea::findNearestConnectionPoint(Shape* shape, const QPoint& pos)
{
    if (!shape) {
        return nullptr;
    }
    QVector<ConnectionPoint*> points = shape->getConnectionPoints();
    if (points.isEmpty()) {
        return nullptr;
    }
    ConnectionPoint* nearest = nullptr;
    double minDistance = std::numeric_limits<double>::max();
    for (ConnectionPoint* point : points) {
        QPoint pointPos = point->getPosition();
        double distance = std::sqrt(std::pow(pointPos.x() - pos.x(), 2) + 
                                  std::pow(pointPos.y() - pos.y(), 2));
        if (distance < minDistance) {
            minDistance = distance;
            nearest = point;
        }
    }
    return nearest;
}
void DrawingArea::contextMenuEvent(QContextMenuEvent *event)
{
    QPoint pos = event->pos();
    QPoint scenePos = mapToScene(pos);
    if (!m_multiSelectedShapes.isEmpty()) {
        bool clickedOnSelectedShape = false;
        for (Shape* shape : m_multiSelectedShapes) {
            if (shape->contains(scenePos)) {
                clickedOnSelectedShape = true;
                break;
            }
        }
        if (clickedOnSelectedShape) {
            showShapeContextMenu(pos);
            event->accept();
            return;
        }
    } 
    else if (m_selectedShape && m_selectedShape->contains(scenePos)) {
        showShapeContextMenu(pos);
        event->accept();
        return;
    }
    else if (m_selectedConnection) {
        for (Connection* connection : m_connections) {
            if (connection == m_selectedConnection && connection->contains(scenePos)) {
                showShapeContextMenu(pos);
                event->accept();
                return;
            }
        }
    }
    showCanvasContextMenu(pos);
    event->accept();
}
void DrawingArea::createContextMenus()
{
    m_shapeContextMenu = new QMenu(this);
    QAction *copyAction = m_shapeContextMenu->addAction(tr("Copy"));
    copyAction->setShortcut(QKeySequence::Copy);  
    copyAction->setShortcutVisibleInContextMenu(true);  
    connect(copyAction, &QAction::triggered, this, &DrawingArea::copySelectedShape);
    QAction *cutAction = m_shapeContextMenu->addAction(tr("Cut"));
    cutAction->setShortcut(QKeySequence::Cut);  
    cutAction->setShortcutVisibleInContextMenu(true);  
    connect(cutAction, &QAction::triggered, this, &DrawingArea::cutSelectedShape);
    QAction *deleteAction = m_shapeContextMenu->addAction(tr("Delete"));
    deleteAction->setShortcut(QKeySequence(Qt::Key_Delete));  
    deleteAction->setShortcutVisibleInContextMenu(true);  
    connect(deleteAction, &QAction::triggered, this, &DrawingArea::deleteSelectedShape);
    m_shapeContextMenu->addSeparator();
    QMenu *layerMenu = m_shapeContextMenu->addMenu(tr("Layer"));
    QAction *bringToFrontAction = layerMenu->addAction(tr("Move Shape To Top"));
    connect(bringToFrontAction, &QAction::triggered, this, &DrawingArea::moveShapeToTop);
    QAction *sendToBackAction = layerMenu->addAction(tr("Move Shape To Bottom"));
    connect(sendToBackAction, &QAction::triggered, this, &DrawingArea::moveShapeToBottom);
    QAction *bringForwardAction = layerMenu->addAction(tr("Move Shape Up"));
    connect(bringForwardAction, &QAction::triggered, this, &DrawingArea::moveShapeUp);
    QAction *sendBackwardAction = layerMenu->addAction(tr("Move Shape Down"));
    connect(sendBackwardAction, &QAction::triggered, this, &DrawingArea::moveShapeDown);
    m_canvasContextMenu = new QMenu(this);
    QAction *pasteAction = m_canvasContextMenu->addAction(tr("Paste"));
    pasteAction->setShortcut(QKeySequence::Paste);  
    pasteAction->setShortcutVisibleInContextMenu(true);  
    QAction *selectAllAction = m_canvasContextMenu->addAction(tr("Select All"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);  
    selectAllAction->setShortcutVisibleInContextMenu(true);  
    connect(selectAllAction, &QAction::triggered, this, &DrawingArea::selectAllShapes);
}
void DrawingArea::showShapeContextMenu(const QPoint &pos)
{
    if (!m_shapeContextMenu) {
        createContextMenus();
    }
    QAction *copyAction = m_shapeContextMenu->actions().at(0);
    QAction *cutAction = m_shapeContextMenu->actions().at(1);
    QAction *deleteAction = m_shapeContextMenu->actions().at(2);
    QMenu *layerMenu = m_shapeContextMenu->actions().at(4)->menu();
    disconnect(copyAction, nullptr, this, nullptr);
    disconnect(cutAction, nullptr, this, nullptr);
    disconnect(deleteAction, nullptr, this, nullptr);
    if (!m_multiSelectedShapes.isEmpty()) {
        connect(copyAction, &QAction::triggered, this, &DrawingArea::copyMultiSelectedShapes);
        connect(cutAction, &QAction::triggered, this, &DrawingArea::cutMultiSelectedShapes);
        connect(deleteAction, &QAction::triggered, this, [this]() {
            cutMultiSelectedShapes(); 
        });
        layerMenu->setEnabled(false);
    } else {
        connect(copyAction, &QAction::triggered, this, &DrawingArea::copySelectedShape);
        connect(cutAction, &QAction::triggered, this, &DrawingArea::cutSelectedShape);
        connect(deleteAction, &QAction::triggered, this, &DrawingArea::deleteSelectedShape);
        layerMenu->setEnabled(true);
    }
    m_shapeContextMenu->exec(mapToGlobal(pos));
}
void DrawingArea::showCanvasContextMenu(const QPoint &pos)
{
    if (!m_canvasContextMenu) {
        createContextMenus();
    }
    QPoint canvasPos = pos;
    QAction *pasteAction = m_canvasContextMenu->actions().at(0);
    pasteAction->setEnabled(m_copiedShape != nullptr || !m_copiedShapes.isEmpty());
    disconnect(pasteAction, nullptr, this, nullptr);
    connect(pasteAction, &QAction::triggered, this, [this, canvasPos]() {
        this->pasteShape(canvasPos);
    });
    m_canvasContextMenu->exec(mapToGlobal(pos));
}
void DrawingArea::moveShapeUp()
{
    if (!m_selectedShape) {
        qDebug() << "moveShapeUp: No selected graphics";
        return;
    }
    int index = m_shapes.indexOf(m_selectedShape);
    qDebug() << "Moveshapeup: current drawing index= " << index << ", Total number of drawings= " << m_shapes.size();
    if (index < m_shapes.size() - 1) {
        std::swap(m_shapes[index], m_shapes[index + 1]);
        qDebug() << "Moveshapeup: swapped location, new index=" << (index + 1);
        update(); 
        emit shapeSelectionChanged(true);
    } else {
        qDebug() << "Moveshapeup: the shape is already on the top layer";
    }
}
void DrawingArea::moveShapeDown()
{
    if (!m_selectedShape) {
        qDebug() << "Moveshapedown: no selected shapes";
        return;
    }
    int index = m_shapes.indexOf(m_selectedShape);
    qDebug() << "Moveshapedown: current drawing index=" << index << ", total number of drawings=" << m_shapes.size();
    if (index > 0) {
        std::swap(m_shapes[index], m_shapes[index - 1]);
        qDebug() << "Moveshapedown: swapped location, new index=" << (index - 1);
        update(); 
        emit shapeSelectionChanged(true);
    } else {
        qDebug() << "Moveshapedown: the graph is already at the lowest level";
    }
}
void DrawingArea::moveShapeToTop()
{
    if (!m_selectedShape) {
        return;
    }
    int index = m_shapes.indexOf(m_selectedShape);
    if (index >= 0 && index < m_shapes.size() - 1) {
        Shape* shapeToMove = m_selectedShape;
        m_shapes.removeAt(index);
        m_shapes.append(shapeToMove);
        update(); 
        emit shapeSelectionChanged(true);
    } else if (index < 0) {
    } else {
    }
}
void DrawingArea::moveShapeToBottom()
{
    if (!m_selectedShape) {
        return;
    }
    int index = m_shapes.indexOf(m_selectedShape);
    if (index > 0) {
        m_shapes.removeAt(index);
        m_shapes.prepend(m_selectedShape);
        update(); 
        emit shapeSelectionChanged(true);
    } else {
    }
}
void DrawingArea::copySelectedShape()
{
    if (!m_multiSelectedShapes.isEmpty()) {
        copyMultiSelectedShapes();
        return;
    }
    if (!m_selectedShape) return;
    if (m_copiedShape) {
        delete m_copiedShape;
        m_copiedShape = nullptr;
    }
    for (Shape* shape : m_copiedShapes) {
        delete shape;
    }
    m_copiedShapes.clear();
    m_copiedShapesPositions.clear();
    QString type = m_selectedShape->type();
    int basis = m_selectedShape->getRect().width() / 2;
    m_copiedShape = ShapeFactory::instance().createShape(type, basis);
    if (m_copiedShape) {
        m_copiedShape->setRect(m_selectedShape->getRect());
        m_copiedShape->setText(m_selectedShape->text());
        m_copiedShape->setFillColor(m_selectedShape->fillColor());
        m_copiedShape->setLineColor(m_selectedShape->lineColor());
        m_copiedShape->setLineWidth(m_selectedShape->lineWidth());
        m_copiedShape->setLineStyle(m_selectedShape->lineStyle());
        m_copiedShape->setTransparency(m_selectedShape->transparency());
    }
}
void DrawingArea::cutSelectedShape()
{
    if (!m_selectedShape && !m_selectedConnection) return;
    if (m_selectedShape) {
        copySelectedShape();
    }
    deleteSelectedShape();
}
void DrawingArea::pasteShape(const QPoint &pos)
{
    if (!m_copiedShapes.isEmpty() || !m_copiedConnections.isEmpty()) {
        m_selectedShape = nullptr;
        m_multiSelectedShapes.clear();
        m_selectedConnection = nullptr;
        m_multySelectedConnections.clear();
        QPoint pastePos = pos.isNull() ? mapFromGlobal(QCursor::pos()) : pos;
        QPoint scenePos = mapToScene(pastePos);
        for (int i = 0; i < m_copiedShapes.size(); ++i) {
            Shape* sourceShape = m_copiedShapes[i];
            QPoint relativePos = m_copiedShapesPositions[i];
            QString type = sourceShape->type();
            int basis = sourceShape->getRect().width() / 2;
            Shape* newShape = ShapeFactory::instance().createShape(type, basis);
            if (newShape) {
                QRect rect = sourceShape->getRect();
                rect.moveCenter(scenePos + relativePos);
                newShape->setRect(rect);
                newShape->setText(sourceShape->text());
                newShape->setFillColor(sourceShape->fillColor());
                newShape->setLineColor(sourceShape->lineColor());
                newShape->setLineWidth(sourceShape->lineWidth());
                newShape->setLineStyle(sourceShape->lineStyle());
                newShape->setTransparency(sourceShape->transparency());
                m_shapes.append(newShape);
                m_multiSelectedShapes.append(newShape);
            }
        }
        for (int i = 0; i < m_copiedConnections.size(); ++i) {
            Connection* sourceConn = m_copiedConnections[i];
            QPoint relativePos = m_copiedConnectionsPositions[i];
            QPoint origStartPos = sourceConn->getStartPosition();
            QPoint origEndPos = sourceConn->getEndPosition();
            QPoint origCenter = (origStartPos + origEndPos) / 2;
            QPoint startRelativePos = origStartPos - origCenter;
            QPoint endRelativePos = origEndPos - origCenter;
            QPoint newCenter = scenePos + relativePos;
            QPoint newStartPos = newCenter + startRelativePos;
            QPoint newEndPos = newCenter + endRelativePos;
            int startShapeIndex = -1;
            int endShapeIndex = -1;
            ConnectionPoint::Position startPosition = ConnectionPoint::Free;
            ConnectionPoint::Position endPosition = ConnectionPoint::Free;
            for (int j = 0; j < m_copiedConnectionStartShapes.size(); ++j) {
                if (m_copiedConnectionStartShapes[j].first == i) {
                    startShapeIndex = m_copiedConnectionStartShapes[j].second;
                    if (j >= 0 && j < m_copiedConnectionStartPoints.size()) {
                        startPosition = m_copiedConnectionStartPoints[j];
                    }
                    break;
                }
            }
            for (int j = 0; j < m_copiedConnectionEndShapes.size(); ++j) {
                if (m_copiedConnectionEndShapes[j].first == i) {
                    endShapeIndex = m_copiedConnectionEndShapes[j].second;
                    if (j >= 0 && j < m_copiedConnectionEndPoints.size()) {
                        endPosition = m_copiedConnectionEndPoints[j];
                    }
                    break;
                }
            }
            ConnectionPoint* startPoint = nullptr;
            ConnectionPoint* endPoint = nullptr;
            if (startShapeIndex >= 0 && startShapeIndex < m_multiSelectedShapes.size()) {
                Shape* startShape = m_multiSelectedShapes[startShapeIndex];
                QVector<ConnectionPoint*> points = startShape->getConnectionPoints();
                for (ConnectionPoint* point : points) {
                    if (point->getPositionType() == startPosition) {
                        startPoint = point;
                        break;
                    }
                }
            } else {
                startPoint = new ConnectionPoint(newStartPos);
            }
            if (endShapeIndex >= 0 && endShapeIndex < m_multiSelectedShapes.size()) {
                Shape* endShape = m_multiSelectedShapes[endShapeIndex];
                QVector<ConnectionPoint*> points = endShape->getConnectionPoints();
                for (ConnectionPoint* point : points) {
                    if (point->getPositionType() == endPosition) {
                        endPoint = point;
                        break;
                    }
                }
            } else {
                endPoint = new ConnectionPoint(newEndPos);
            }
            ArrowLine* newConnection = new ArrowLine(newStartPos, newEndPos);
            if (startPoint) {
                newConnection->setStartPoint(startPoint);
            }
            if (endPoint) {
                newConnection->setEndPoint(endPoint);
            }
            m_connections.append(newConnection);
            m_multySelectedConnections.append(newConnection);
            newConnection->setSelected(true);
        }
        if (!m_multiSelectedShapes.isEmpty() || !m_multySelectedConnections.isEmpty()) {
            emit multiSelectionChanged(true);
            emit shapesCountChanged(getShapesCount());
            update();
        }
        return;
    }
    if (!m_copiedShape) return;
    QString type = m_copiedShape->type();
    int basis = m_copiedShape->getRect().width() / 2;
    Shape *newShape = ShapeFactory::instance().createShape(type, basis);
    if (newShape) {
        QRect rect = m_copiedShape->getRect();
        if (!pos.isNull()) {
            QPoint scenePos = mapToScene(pos);
            rect.moveCenter(scenePos);
        } else {
            rect.translate(20, 20);
        }
        newShape->setRect(rect);
        newShape->setText(m_copiedShape->text());
        newShape->setFillColor(m_copiedShape->fillColor());
        newShape->setLineColor(m_copiedShape->lineColor());
        newShape->setLineWidth(m_copiedShape->lineWidth());
        newShape->setLineStyle(m_copiedShape->lineStyle());
        newShape->setTransparency(m_copiedShape->transparency());
        m_shapes.append(newShape);
        m_multiSelectedShapes.clear();
        m_multySelectedConnections.clear();
        m_selectedConnection = nullptr;
        m_selectedShape = newShape;
        emit shapeSelectionChanged(true);
        emit multiSelectionChanged(false);
        emit shapesCountChanged(getShapesCount());
        update();
    }
}
void DrawingArea::deleteSelectedShape()
{
    if (m_selectedShape) {
        int index = m_shapes.indexOf(m_selectedShape);
        if (index >= 0) {
            m_shapes.removeAt(index);
        }
        QVector<Connection*> connectionsToRemove;
        for (Connection* connection : m_connections) {
            if ((connection->getStartPoint() && connection->getStartPoint()->getOwner() == m_selectedShape) || 
                (connection->getEndPoint() && connection->getEndPoint()->getOwner() == m_selectedShape)) {
                connectionsToRemove.append(connection);
            }
        }
        for (Connection* connection : connectionsToRemove) {
            m_connections.removeOne(connection);
            delete connection;
        }
        delete m_selectedShape;
        m_selectedShape = nullptr;
        emit shapeSelectionChanged(false);
        emit shapesCountChanged(getShapesCount());
        update();
    } else if (m_selectedConnection) {
        m_connections.removeOne(m_selectedConnection);
        delete m_selectedConnection;
        m_selectedConnection = nullptr;
        emit shapesCountChanged(getShapesCount());
        update();
    }
}
void DrawingArea::selectAllShapes()
{
    clearMultySelection();
    for (int i = 0; i < m_shapes.size(); ++i) {
        m_multiSelectedShapes.append(m_shapes[i]);
    }
    for (int i = 0; i < m_connections.size(); ++i) {
        m_multySelectedConnections.append(m_connections[i]);
        m_connections[i]->setSelected(true);
    }
    if (!m_multiSelectedShapes.isEmpty() || !m_multySelectedConnections.isEmpty()) {
        emit multiSelectionChanged(true);
    }
    update();
}
void DrawingArea::keyPressEvent(QKeyEvent *event)
{
    if (m_textEditor && m_textEditor->isVisible()) {
        QWidget::keyPressEvent(event);
        return;
    }
    if (event->matches(QKeySequence::SelectAll)) {
        selectAllShapes();
    } else if (event->matches(QKeySequence::Copy)) {
        if (!m_multiSelectedShapes.isEmpty()) {
            copyMultiSelectedShapes();
        } else {
            copySelectedShape();
        }
    } else if (event->matches(QKeySequence::Cut)) {
        if (!m_multiSelectedShapes.isEmpty()) {
            cutMultiSelectedShapes();
        } else {
            cutSelectedShape();
        }
    } else if (event->matches(QKeySequence::Paste)) {
        pasteShape(mapFromGlobal(QCursor::pos()));
    } else if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        if (!m_multiSelectedShapes.isEmpty()) {
            cutMultiSelectedShapes(); 
        } else {
            deleteSelectedShape();
        }
    } else {
        QWidget::keyPressEvent(event);
    }
}
void DrawingArea::createArrowLine(const QPoint& startPoint, const QPoint& endPoint)
{
    ArrowLine* arrowLine = new ArrowLine(startPoint, endPoint);
    m_connections.append(arrowLine);
    selectConnection(arrowLine);
    update();
}
void DrawingArea::selectConnection(Connection* connection)
{
    m_selectedShape = nullptr;
    if (m_selectedConnection && m_selectedConnection != connection) {
        m_selectedConnection->setSelected(false);
    }
    m_selectedConnection = connection;
    if (m_selectedConnection) {
        m_selectedConnection->setSelected(true);
    }
    update();
}
Shape* DrawingArea::cloneShape(const Shape* sourceShape)
{
    if (!sourceShape) {
        return nullptr;
    }
    Shape* clonedShape = ShapeFactory::instance().createShape(sourceShape->type(), 55);
    if (!clonedShape) {
        return nullptr;
    }
    clonedShape->setRect(sourceShape->getRect());
    clonedShape->setText(sourceShape->text());
    return clonedShape;
}
void DrawingArea::drawConnectionPreview(QPainter* painter, Connection* connection)
{
    if (!connection || !m_movingConnectionPoint || !m_activeConnectionPoint) 
        return;
    QPoint startPos, endPos;
    if (m_activeConnectionPoint == connection->getStartPoint()) {
        startPos = m_connectionDragPoint;
        endPos = connection->getEndPosition();
    } else {
        startPos = connection->getStartPosition();
        endPos = m_connectionDragPoint;
    }
    bool drawArrow = false;
    ArrowLine* arrowLine = dynamic_cast<ArrowLine*>(connection);
    if (arrowLine) {
        drawArrow = true;
    } 
    else if (m_activeConnectionPoint == connection->getEndPoint()) {
        drawArrow = true;
    }
    Connection::drawConnectionLine(painter, startPos, endPos, true, drawArrow);
}
void DrawingArea::drawGrid(QPainter *painter)
{
    if (!m_showGrid) return;
    painter->save();
    QRectF gridRect(0, 0, m_drawingAreaSize.width(), m_drawingAreaSize.height());
    QColor lightColor(245, 245, 245);  
    QColor darkColor(241, 241, 241);   
    for (int y = 0; y <= gridRect.height(); y += m_gridSize) {
        if (y % (m_gridSize * 4) == 0) {
            QPen darkPen(darkColor, m_gridThickness);
            painter->setPen(darkPen);
        } else {
            QPen lightPen(lightColor, m_gridThickness);
            painter->setPen(lightPen);
        }
        painter->drawLine(0, y, gridRect.width(), y);
    }
    for (int x = 0; x <= gridRect.width(); x += m_gridSize) {
        if (x % (m_gridSize * 4) == 0) {
            QPen darkPen(darkColor, m_gridThickness);
            painter->setPen(darkPen);
        } else {
            QPen lightPen(lightColor, m_gridThickness);
            painter->setPen(lightPen);
        }
        painter->drawLine(x, 0, x, gridRect.height());
    }
    painter->restore();
}
void DrawingArea::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    update();
}
void DrawingArea::setPageSize(const QSize &size)
{
    QSize oldSize = m_drawingAreaSize;
    setDrawingAreaSize(size);
    update();
}
void DrawingArea::setShowGrid(bool show)
{
    m_showGrid = show;
    update();
}
void DrawingArea::setGridColor(const QColor &color)
{
    m_gridColor = color;
    update();
}
void DrawingArea::setGridSize(int size)
{
    m_gridSize = size;
    update();
}
void DrawingArea::setGridThickness(int thickness)
{
    m_gridThickness = thickness;
    update();
}
QColor DrawingArea::getBackgroundColor() const
{
    return m_backgroundColor;
}
bool DrawingArea::getShowGrid() const
{
    return m_showGrid;
}
QColor DrawingArea::getGridColor() const
{
    return m_gridColor;
}
int DrawingArea::getGridSize() const
{
    return m_gridSize;
}
int DrawingArea::getGridThickness() const
{
    return m_gridThickness;
}
void DrawingArea::applyPageSettings()
{
    update();
}
void DrawingArea::setScale(qreal scale)
{
    qreal newScale = qBound(MIN_SCALE, scale, MAX_SCALE);
    if (qFuzzyCompare(newScale, m_scale))
        return; 
    QSize oldSize = size();
    qreal oldScale = m_scale;
    QScrollArea* scrollArea = nullptr;
    QWidget* parent = parentWidget();
    while (parent) {
        scrollArea = qobject_cast<QScrollArea*>(parent);
        if (scrollArea)
            break;
        parent = parent->parentWidget();
    }
    QPointF relativePos;
    if (scrollArea) {
        QScrollBar* hBar = scrollArea->horizontalScrollBar();
        QScrollBar* vBar = scrollArea->verticalScrollBar();
        double hRatio = (hBar->maximum() > 0) ? 
                       (double)hBar->value() / hBar->maximum() : 0.5;
        double vRatio = (vBar->maximum() > 0) ? 
                       (double)vBar->value() / vBar->maximum() : 0.5;
        relativePos = QPointF(hRatio, vRatio);
    }
    m_scale = newScale;
    QSize newSize(m_drawingAreaSize.width() * 3 * m_scale, 
                 m_drawingAreaSize.height() * 3 * m_scale);
    setFixedSize(newSize);  
    emit scaleChanged(m_scale);
    update();
    if (scrollArea) {
        scrollArea->updateGeometry();
        QScrollBar* hBar = scrollArea->horizontalScrollBar();
        QScrollBar* vBar = scrollArea->verticalScrollBar();
        hBar->setValue(qRound(relativePos.x() * hBar->maximum()));
        vBar->setValue(qRound(relativePos.y() * vBar->maximum()));
    }
}
void DrawingArea::zoomInOrOut(const qreal& factor)
{
    setScale(m_scale * factor);  
}
void DrawingArea::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        QPoint mousePos = event->position().toPoint();
        qreal factor = 1.0;
        if (event->angleDelta().y() > 0) {
            factor = 1.2;  
        } else {
            factor = 1.0 / 1.2;  
        }
        if(factor == 1.0) return;
        else{
            zoomInOrOut(factor);
        }
        update();
        event->accept();
    } 
    else if (event->modifiers() & Qt::ShiftModifier) {
        int delta = event->angleDelta().y();
        QPoint scrollDelta;
        if (event->angleDelta().x() != 0) {
            scrollDelta.setX(event->angleDelta().x() / 8);
        } else {
            scrollDelta.setX(delta / 3); 
        }
        QScrollBar* hBar = findParentScrollBar(Qt::Horizontal);
        if (hBar) {
            hBar->setValue(hBar->value() - scrollDelta.x());
        } else {
            m_viewOffset.setX(m_viewOffset.x() - scrollDelta.x());
            update();
        }
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}
QScrollBar* DrawingArea::findParentScrollBar(Qt::Orientation orientation) const
{
    QWidget* parent = this->parentWidget();
    while (parent) {
        QScrollArea* scrollArea = qobject_cast<QScrollArea*>(parent);
        if (scrollArea) {
            return orientation == Qt::Horizontal ? 
                   scrollArea->horizontalScrollBar() : 
                   scrollArea->verticalScrollBar();
        }
        parent = parent->parentWidget();
    }
    return nullptr;
}
QPoint DrawingArea::mapToScene(const QPoint& viewPoint) const
{
    QPoint drawingAreaTopLeft(
        (width() - m_drawingAreaSize.width() * m_scale) / 2,
        (height() - m_drawingAreaSize.height() * m_scale) / 2
    );
    QPoint relativePos = viewPoint - drawingAreaTopLeft;
    QPoint scaledPos(
        relativePos.x() / m_scale,
        relativePos.y() / m_scale
    );
    QPoint scenePos = scaledPos + m_viewOffset;
    return scenePos;
}
QPoint DrawingArea::mapFromScene(const QPoint& scenePoint) const
{
    QPoint pointWithoutOffset = scenePoint - m_viewOffset;
    QPoint scaledPoint(
        pointWithoutOffset.x() * m_scale,
        pointWithoutOffset.y() * m_scale
    );
    QPoint drawingAreaTopLeft(
        (width() - m_drawingAreaSize.width() * m_scale) / 2,
        (height() - m_drawingAreaSize.height() * m_scale) / 2
    );
    QPoint viewPoint = scaledPoint + drawingAreaTopLeft;
    return viewPoint;
}
void DrawingArea::setSelectedShapeFontFamily(const QString& family)
{
    if (m_selectedShape) {
        m_selectedShape->setFontFamily(family);
        QFont font = m_selectedShape->getFont();
        m_textEditor->setFont(font);
        QTextDocument *doc = m_textEditor->document();
        QFont docFont = doc->defaultFont();
        docFont.setFamily(family);
        doc->setDefaultFont(docFont);
        QColor fillColor = m_selectedShape->fillColor();
        QColor transparentFillColor = fillColor; 
        transparentFillColor.setAlphaF(0.0); 
        QString styleSheet = QString("background-color: rgba(%1, %2, %3, %4); border: none; font-family: '%5';")
                                .arg(transparentFillColor.red())
                                .arg(transparentFillColor.green())
                                .arg(transparentFillColor.blue())
                                .arg(transparentFillColor.alphaF())
                                .arg(family);
        m_textEditor->setStyleSheet(styleSheet);
        update();  
    }
}
void DrawingArea::setSelectedShapeFontSize(int size)
{
    if (m_selectedShape) {
        m_selectedShape->setFontSize(size);
        QFont font = m_selectedShape->getFont();
        m_textEditor->setFont(font);
        QTextDocument *doc = m_textEditor->document();
        QFont docFont = doc->defaultFont();
        docFont.setPointSize(size);
        doc->setDefaultFont(docFont);
        QColor fillColor = m_selectedShape->fillColor();
        QColor transparentFillColor = fillColor; 
        transparentFillColor.setAlphaF(0.0); 
        QString styleSheet = QString("background-color: rgba(%1, %2, %3, %4); border: none; font-size: %5pt;")
                                .arg(transparentFillColor.red())
                                .arg(transparentFillColor.green())
                                .arg(transparentFillColor.blue())
                                .arg(transparentFillColor.alphaF())
                                .arg(size);
        m_textEditor->setStyleSheet(styleSheet);
        update();  
    }
}
void DrawingArea::setSelectedShapeFontBold(bool bold)
{
    if (m_selectedShape) {
        m_selectedShape->setFontBold(bold);
        QFont font = m_selectedShape->getFont();
        m_textEditor->setFont(font);  
        update();  
    }
}
void DrawingArea::setSelectedShapeFontItalic(bool italic)
{
    if (m_selectedShape) {
        m_selectedShape->setFontItalic(italic);
        QFont font = m_selectedShape->getFont();
        m_textEditor->setFont(font);  
        update();  
    }
}
void DrawingArea::setSelectedShapeFontUnderline(bool underline)
{
    if (m_selectedShape) {
        m_selectedShape->setFontUnderline(underline);
        QFont font = m_selectedShape->getFont();
        m_textEditor->setFont(font);  
        update();  
    }
}
void DrawingArea::setSelectedShapeFontColor(const QColor& color)
{
    if (m_selectedShape) {
        m_selectedShape->setFontColor(color);
        m_textEditor->setTextColor(color);  
        emit fontColorChanged(color);       
        update();  
    }
}
void DrawingArea::setSelectedShapeTextAlignment(Qt::Alignment alignment)
{
    if (m_selectedShape) {
        m_selectedShape->setTextAlignment(alignment);
        m_textEditor->setAlignment(alignment);  
        update();  
    }
}
int DrawingArea::getShapesCount() const
{
    return m_shapes.size() + m_connections.size();
}
void DrawingArea::setDrawingAreaSize(const QSize &size)
{
    if (size == m_drawingAreaSize)
        return;
    QScrollArea* scrollArea = nullptr;
    QWidget* parent = parentWidget();
    while (parent) {
        scrollArea = qobject_cast<QScrollArea*>(parent);
        if (scrollArea)
            break;
        parent = parent->parentWidget();
    }
    QPointF relativePos;
    if (scrollArea) {
        QScrollBar* hBar = scrollArea->horizontalScrollBar();
        QScrollBar* vBar = scrollArea->verticalScrollBar();
        double hRatio = (hBar->maximum() > 0) ? 
                       (double)hBar->value() / hBar->maximum() : 0.5;
        double vRatio = (vBar->maximum() > 0) ? 
                       (double)vBar->value() / vBar->maximum() : 0.5;
        relativePos = QPointF(hRatio, vRatio);
    }
    m_drawingAreaSize = size;
    QSize newWidgetSize(m_drawingAreaSize.width() * 3 * m_scale, 
                       m_drawingAreaSize.height() * 3 * m_scale);
    setFixedSize(newWidgetSize);
    if (scrollArea) {
        scrollArea->updateGeometry();
        QScrollBar* hBar = scrollArea->horizontalScrollBar();
        QScrollBar* vBar = scrollArea->verticalScrollBar();
        hBar->setValue(qRound(relativePos.x() * hBar->maximum()));
        vBar->setValue(qRound(relativePos.y() * vBar->maximum()));
    }
}
void DrawingArea::centerDrawingArea()
{
    QScrollArea* scrollArea = qobject_cast<QScrollArea*>(parentWidget());
    if (!scrollArea) {
        QWidget* parent = parentWidget();
        while (parent) {
            scrollArea = qobject_cast<QScrollArea*>(parent);
            if (scrollArea) {
                break;
            }
            parent = parent->parentWidget();
        }
    }
    if (scrollArea) {
        QScrollBar* hBar = scrollArea->horizontalScrollBar();
        QScrollBar* vBar = scrollArea->verticalScrollBar();
        if (hBar && vBar) {
            QSize totalSize = size();
            QSize viewportSize = scrollArea->viewport()->size();
            int hValue = (totalSize.width() - viewportSize.width()) / 2;
            int vValue = (totalSize.height() - viewportSize.height()) / 2;
            hValue = qMax(0, qMin(hValue, hBar->maximum()));
            vValue = qMax(0, qMin(vValue, vBar->maximum()));
            hBar->setValue(hValue);
            vBar->setValue(vValue);
        }
    }
    update();
}
bool DrawingArea::exportToPng(const QString &filePath)
{
    QImage image(m_drawingAreaSize, QImage::Format_ARGB32);
    image.fill(m_backgroundColor); 
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    Shape* tempSelectedShape = m_selectedShape;
    Connection* tempSelectedConnection = m_selectedConnection;
    m_selectedShape = nullptr;
    m_selectedConnection = nullptr;
    /*
    if (m_showGrid) {
        drawGrid(&painter);
    }
    */
    for (Shape* shape : m_shapes) {
        shape->paint(&painter);
    }
    for (Connection* connection : m_connections) {
        connection->paint(&painter);
    }
    m_selectedShape = tempSelectedShape;
    m_selectedConnection = tempSelectedConnection;
    bool success = image.save(filePath, "PNG");
    update();
    return success;
}
bool DrawingArea::exportToSvg(const QString &filePath)
{
    QSvgGenerator generator;
    generator.setFileName(filePath);
    generator.setSize(m_drawingAreaSize);
    generator.setViewBox(QRect(0, 0, m_drawingAreaSize.width(), m_drawingAreaSize.height()));
    generator.setTitle(tr("flow chart"));
    generator.setDescription(tr("SVG file generated by flowchart designer"));
    generator.setResolution(96); 
    QString shapesMetadata = "<flowchart:shapes xmlns:flowchart=\"http://flowchart.zeqi.com/ns\">";
    for (int i = 0; i < m_shapes.size(); ++i) {
        Shape* shape = m_shapes[i];
        QRect rect = shape->getRect();
        shapesMetadata += QString("<flowchart:shape id=\"%1\" type=\"%2\" x=\"%3\" y=\"%4\" width=\"%5\" height=\"%6\"")
                               .arg(i)
                               .arg(shape->type())
                               .arg(rect.x())
                               .arg(rect.y())
                               .arg(rect.width())
                               .arg(rect.height());
        shapesMetadata += QString(" text=\"%1\"").arg(shape->text().toHtmlEscaped());
        shapesMetadata += QString(" fontFamily=\"%1\" fontSize=\"%2\" fontBold=\"%3\" fontItalic=\"%4\" fontColor=\"%5\" fontUnderline=\"%6\"")
                               .arg(shape->fontFamily())
                               .arg(shape->fontSize())
                               .arg(shape->isFontBold() ? "true" : "false")
                               .arg(shape->isFontItalic() ? "true" : "false")
                               .arg(shape->fontColor().name())
                               .arg(shape->isFontUnderline() ? "true" : "false");
        shapesMetadata += " />";
    }
    shapesMetadata += "<flowchart:connections>";
    for (int i = 0; i < m_connections.size(); ++i) {
        Connection* conn = m_connections[i];
        QPoint startPos = conn->getStartPosition();
        QPoint endPos = conn->getEndPosition();
        bool isArrow = (dynamic_cast<ArrowLine*>(conn) != nullptr);
        int startShapeIndex = -1;
        int startConnectionPointIndex = -1;
        int endShapeIndex = -1;
        int endConnectionPointIndex = -1;
        if (conn->getStartPoint() && conn->getStartPoint()->getOwner()) {
            Shape* startShape = conn->getStartPoint()->getOwner();
            startShapeIndex = m_shapes.indexOf(startShape);
            ConnectionPoint* startCP = conn->getStartPoint();
            ConnectionPoint::Position startPosition = startCP->getPositionType();
            if (startPosition == ConnectionPoint::Top) startConnectionPointIndex = 0;
            else if (startPosition == ConnectionPoint::Right) startConnectionPointIndex = 1;
            else if (startPosition == ConnectionPoint::Bottom) startConnectionPointIndex = 2;
            else if (startPosition == ConnectionPoint::Left) startConnectionPointIndex = 3;
        }
        if (conn->getEndPoint() && conn->getEndPoint()->getOwner()) {
            Shape* endShape = conn->getEndPoint()->getOwner();
            endShapeIndex = m_shapes.indexOf(endShape);
            ConnectionPoint* endCP = conn->getEndPoint();
            ConnectionPoint::Position endPosition = endCP->getPositionType();
            if (endPosition == ConnectionPoint::Top) endConnectionPointIndex = 0;
            else if (endPosition == ConnectionPoint::Right) endConnectionPointIndex = 1;
            else if (endPosition == ConnectionPoint::Bottom) endConnectionPointIndex = 2;
            else if (endPosition == ConnectionPoint::Left) endConnectionPointIndex = 3;
        }
        shapesMetadata += QString("<flowchart:connection id=\"%1\" startX=\"%2\" startY=\"%3\" endX=\"%4\" endY=\"%5\" isArrow=\"%6\" "
                               "startShapeIndex=\"%7\" startConnectionPointIndex=\"%8\" "
                               "endShapeIndex=\"%9\" endConnectionPointIndex=\"%10\" />")
                               .arg(i)
                               .arg(startPos.x())
                               .arg(startPos.y())
                               .arg(endPos.x())
                               .arg(endPos.y())
                               .arg(isArrow ? "true" : "false")
                               .arg(startShapeIndex)
                               .arg(startConnectionPointIndex)
                               .arg(endShapeIndex)
                               .arg(endConnectionPointIndex);
    }
    shapesMetadata += "</flowchart:connections>";
    shapesMetadata += "</flowchart:shapes>";
    QString metadata = QString(
        "<metadata>"
        "<flowchart:settings xmlns:flowchart=\"http://flowchart.zeqi.com/ns\">"
        "<flowchart:drawingAreaWidth>%1</flowchart:drawingAreaWidth>"
        "<flowchart:drawingAreaHeight>%2</flowchart:drawingAreaHeight>"
        "<flowchart:backgroundColor>%3</flowchart:backgroundColor>"
        "</flowchart:settings>"
        "%4"
        "</metadata>"
    ).arg(m_drawingAreaSize.width())
     .arg(m_drawingAreaSize.height())
     .arg(m_backgroundColor.name())
     .arg(shapesMetadata);
    QPainter painter;
    painter.begin(&generator);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    Shape* tempSelectedShape = m_selectedShape;
    Connection* tempSelectedConnection = m_selectedConnection;
    m_selectedShape = nullptr;
    m_selectedConnection = nullptr;
    painter.fillRect(QRect(0, 0, m_drawingAreaSize.width(), m_drawingAreaSize.height()), m_backgroundColor);
    for (Shape* shape : m_shapes) {
        shape->paint(&painter);
    }
    for (Connection* connection : m_connections) {
        connection->paint(&painter);
    }
    m_selectedShape = tempSelectedShape;
    m_selectedConnection = tempSelectedConnection;
    painter.end();
    update();
    QFile file(filePath);
    if (file.open(QIODevice::ReadWrite)) {
        QByteArray svgData = file.readAll();
        QString svgText = QString::fromUtf8(svgData);
        int defsEndPos = svgText.indexOf("</defs>");
        if (defsEndPos != -1) {
            svgText.insert(defsEndPos + 7, metadata);
            file.seek(0);
            file.write(svgText.toUtf8());
            file.resize(file.pos());
        }
        file.close();
    }
    return true;
}
bool DrawingArea::importFromSvg(const QString &filePath)
{
    qDeleteAll(m_shapes);
    m_shapes.clear();
    qDeleteAll(m_connections);
    m_connections.clear();
    m_selectedShape = nullptr;
    m_selectedConnection = nullptr;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    QByteArray svgData = file.readAll();
    file.close();
    QDomDocument domDocument;
    if (!domDocument.setContent(svgData))
        return false;
    QDomElement metadataElement = domDocument.elementsByTagName("metadata").item(0).toElement();
    if (!metadataElement.isNull()) {
        QDomElement settingsElement = metadataElement.firstChildElement("flowchart:settings");
        if (!settingsElement.isNull()) {
            QDomElement widthElement = settingsElement.firstChildElement("flowchart:drawingAreaWidth");
            QDomElement heightElement = settingsElement.firstChildElement("flowchart:drawingAreaHeight");
            QDomElement bgColorElement = settingsElement.firstChildElement("flowchart:backgroundColor");
            if (!widthElement.isNull() && !heightElement.isNull()) {
                int width = widthElement.text().toInt();
                int height = heightElement.text().toInt();
                m_drawingAreaSize = QSize(width, height);
            }
            if (!bgColorElement.isNull()) {
                m_backgroundColor = QColor(bgColorElement.text());
            }
        }
        QDomElement shapesElement = metadataElement.firstChildElement("flowchart:shapes");
        if (!shapesElement.isNull()) {
            QDomNodeList shapeNodes = shapesElement.elementsByTagName("flowchart:shape");
            QMap<int, Shape*> shapeIdMap;
            for (int i = 0; i < shapeNodes.count(); ++i) {
                QDomElement shapeElement = shapeNodes.at(i).toElement();
                int id = shapeElement.attribute("id").toInt();
                QString type = shapeElement.attribute("type");
                int x = shapeElement.attribute("x").toInt();
                int y = shapeElement.attribute("y").toInt();
                int width = shapeElement.attribute("width").toInt();
                int height = shapeElement.attribute("height").toInt();
                QString text = shapeElement.attribute("text");
                int basis = qMin(width, height) / 2;
                basis = qMax(basis, 30); 
                Shape* newShape = ShapeFactory::instance().createShape(type, basis);
                if (newShape) {
                    QRect rect(x, y, width, height);
                    newShape->setRect(rect);
                    newShape->setText(text);
                    QString fontFamily = shapeElement.attribute("fontFamily");
                    if (!fontFamily.isEmpty()) {
                        newShape->setFontFamily(fontFamily);
                    }
                    int fontSize = shapeElement.attribute("fontSize").toInt();
                    if (fontSize > 0) {
                        newShape->setFontSize(fontSize);
                    }
                    bool isBold = (shapeElement.attribute("fontBold") == "true");
                    newShape->setFontBold(isBold);
                    bool isItalic = (shapeElement.attribute("fontItalic") == "true");
                    newShape->setFontItalic(isItalic);
                    bool isUnderline = (shapeElement.attribute("fontUnderline") == "true");
                    newShape->setFontUnderline(isUnderline);
                    QString fontColor = shapeElement.attribute("fontColor");
                    if (!fontColor.isEmpty()) {
                        newShape->setFontColor(QColor(fontColor));
                    }
                    m_shapes.append(newShape);
                    shapeIdMap[id] = newShape;
                }
            }
            QDomNodeList connectionNodes = shapesElement.elementsByTagName("flowchart:connection");
            for (int i = 0; i < connectionNodes.count(); ++i) {
                QDomElement connectionElement = connectionNodes.at(i).toElement();
                int startX = connectionElement.attribute("startX").toInt();
                int startY = connectionElement.attribute("startY").toInt();
                int endX = connectionElement.attribute("endX").toInt();
                int endY = connectionElement.attribute("endY").toInt();
                bool isArrow = (connectionElement.attribute("isArrow") == "true");
                int startShapeIndex = connectionElement.attribute("startShapeIndex").toInt();
                int startConnectionPointIndex = connectionElement.attribute("startConnectionPointIndex").toInt();
                int endShapeIndex = connectionElement.attribute("endShapeIndex").toInt();
                int endConnectionPointIndex = connectionElement.attribute("endConnectionPointIndex").toInt();
                
                ArrowLine* arrowLine = new ArrowLine(QPoint(startX, startY), QPoint(endX, endY));
                bool startConnected = false;
                bool endConnected = false;
                if (startShapeIndex >= 0 && startShapeIndex < m_shapes.size() && 
                    startConnectionPointIndex >= 0 && startConnectionPointIndex <= 3) {
                    Shape* startShape = m_shapes[startShapeIndex];
                    QVector<ConnectionPoint*> startPoints = startShape->getConnectionPoints();
                    if (startConnectionPointIndex < startPoints.size()) {
                        arrowLine->setStartPoint(startPoints[startConnectionPointIndex]);
                        startConnected = true;
                    }
                }
                if (endShapeIndex >= 0 && endShapeIndex < m_shapes.size() && 
                    endConnectionPointIndex >= 0 && endConnectionPointIndex <= 3) {
                    Shape* endShape = m_shapes[endShapeIndex];
                    QVector<ConnectionPoint*> endPoints = endShape->getConnectionPoints();
                    if (endConnectionPointIndex < endPoints.size()) {
                        arrowLine->setEndPoint(endPoints[endConnectionPointIndex]);
                        endConnected = true;
                    }
                }
                m_connections.append(arrowLine);
            }
        }
    }
    setScale(1.0);
    update();
    emit shapesCountChanged(getShapesCount());
    emit selectionChanged();
    return true;
}
void DrawingArea::setSelectedShapeFillColor(const QColor& color)
{
    if (m_selectedShape) {
        m_selectedShape->setFillColor(color);
        update();  
        emit fillColorChanged(color);
    }
}
void DrawingArea::setSelectedShapeLineColor(const QColor& color)
{
    if (m_selectedShape) {
        m_selectedShape->setLineColor(color);
        update();  
        emit lineColorChanged(color);
    }
}
void DrawingArea::setSelectedShapeTransparency(int transparency)
{
    if (m_selectedShape) {
        m_selectedShape->setTransparency(transparency);
        update();
    }
}
void DrawingArea::setSelectedShapeLineWidth(qreal width)
{
    if (m_selectedShape) {
        m_selectedShape->setLineWidth(width);
        update();
    }
}
void DrawingArea::setSelectedShapeLineStyle(int style)
{
    if (m_selectedShape) {
        m_selectedShape->setLineStyle(style);
        update();
    }
}
void DrawingArea::startRectMultiSelection(const QPoint& point)
{
    m_isMultiRectSelecting = true;
    m_multiSelectionStart = mapToScene(point);
    m_multiSelectionRect = QRect(m_multiSelectionStart, QSize(0, 0));
    update();
}
void DrawingArea::updateRectMultiSelection(const QPoint& point)
{
    if (!m_isMultiRectSelecting)
        return;
    QPoint currentPos = mapToScene(point);
    m_multiSelectionRect = QRect(m_multiSelectionStart, currentPos).normalized();
    update();
}
void DrawingArea::finishRectMultiSelection()
{
    if (!m_isMultiRectSelecting)
        return;
    selectMultiShapesInRect(m_multiSelectionRect);
    m_isMultiRectSelecting = false;
}
bool DrawingArea::isShapeCompletelyInRect(Shape* shape, const QRect& rect) const
{
    QRect shapeRect = shape->getRect();
    return rect.contains(shapeRect);
}
void DrawingArea::selectMultiShapesInRect(const QRect& rect)
{
    clearMultySelection();
    for (int i = 0; i < m_shapes.size(); ++i) {
        if (isShapeCompletelyInRect(m_shapes[i], rect)) {
            m_multiSelectedShapes.append(m_shapes[i]);
        }
    }
    for (int i = 0; i < m_connections.size(); ++i) {
        Connection* conn = m_connections[i];
        QPoint startPos = conn->getStartPosition();
        QPoint endPos = conn->getEndPosition();
        if (rect.contains(startPos) && rect.contains(endPos)) {
            m_multySelectedConnections.append(conn);
            conn->setSelected(true);
        }
    }
    if (m_multiSelectedShapes.size() == 1) {
        m_selectedShape = m_multiSelectedShapes.first();
        m_multiSelectedShapes.clear();
        emit shapeSelectionChanged(true);
    } else if (m_multiSelectedShapes.size() > 1) {
        m_selectedShape = nullptr;
        emit multiSelectionChanged(true);
    } else if (m_multySelectedConnections.size() == 1) {
        selectConnection(m_multySelectedConnections.first());
        m_multySelectedConnections.clear();
    } else if (m_multiSelectedShapes.isEmpty() && m_multySelectedConnections.isEmpty()) {
        emit shapeSelectionChanged(false);
        emit multiSelectionChanged(false);
    } else {
        emit multiSelectionChanged(true);
    }
}
void DrawingArea::clearMultySelection()
{
    m_selectedShape = nullptr;
    m_multiSelectedShapes.clear();
    for (Connection* conn : m_multySelectedConnections) {
        conn->setSelected(false);
    }
    m_multySelectedConnections.clear();
    if (m_selectedConnection) {
        m_selectedConnection->setSelected(false);
        m_selectedConnection = nullptr;
    }
    emit shapeSelectionChanged(false);
    emit multiSelectionChanged(false);
}
void DrawingArea::drawMultiSelectionRect(QPainter* painter)
{
    if (!m_isMultiRectSelecting)
        return;
    QColor fillColor(0, 120, 215, 40);
    QColor borderColor(0, 120, 215);
    painter->save();
    painter->setBrush(fillColor);
    painter->setPen(QPen(borderColor, 1, Qt::DashLine));
    QRect viewRect(mapFromScene(m_multiSelectionRect.topLeft()), 
                  mapFromScene(m_multiSelectionRect.bottomRight()));
    painter->drawRect(viewRect);
    painter->restore();
}
void DrawingArea::copyMultiSelectedShapes()
{
    if (m_multiSelectedShapes.isEmpty() && m_multySelectedConnections.isEmpty()) {
        return;
    }
    for (Shape* shape : m_copiedShapes) {
        delete shape;
    }
    m_copiedShapes.clear();
    m_copiedShapesPositions.clear();
    for (Connection* conn : m_copiedConnections) {
        delete conn;
    }
    m_copiedConnections.clear();
    m_copiedConnectionsPositions.clear();
    m_copiedConnectionStartShapes.clear();
    m_copiedConnectionEndShapes.clear();
    m_copiedConnectionStartPoints.clear();
    m_copiedConnectionEndPoints.clear();
    QPoint centerPoint(0, 0);
    int totalElements = 0;
    for (Shape* shape : m_multiSelectedShapes) {
        centerPoint += shape->getRect().center();
        totalElements++;
    }
    for (Connection* conn : m_multySelectedConnections) {
        QPoint startPos = conn->getStartPosition();
        QPoint endPos = conn->getEndPosition();
        centerPoint += (startPos + endPos) / 2;
        totalElements++;
    }
    if (totalElements > 0) {
        centerPoint /= totalElements;
    }
    for (Shape* shape : m_multiSelectedShapes) {
        QString type = shape->type();
        int basis = shape->getRect().width() / 2;
        Shape* copiedShape = ShapeFactory::instance().createShape(type, basis);
        if (copiedShape) {
            copiedShape->setRect(shape->getRect());
            copiedShape->setText(shape->text());
            copiedShape->setFillColor(shape->fillColor());
            copiedShape->setLineColor(shape->lineColor());
            copiedShape->setLineWidth(shape->lineWidth());
            copiedShape->setLineStyle(shape->lineStyle());
            copiedShape->setTransparency(shape->transparency());
            QPoint relativePos = shape->getRect().center() - centerPoint;
            m_copiedShapes.append(copiedShape);
            m_copiedShapesPositions.append(relativePos);
        }
    }
    for (int i = 0; i < m_multySelectedConnections.size(); ++i) {
        Connection* conn = m_multySelectedConnections[i];
        QPoint startPos = conn->getStartPosition();
        QPoint endPos = conn->getEndPosition();
        ArrowLine* copiedConnection = new ArrowLine(startPos, endPos);
        if (copiedConnection) {
            QPoint connCenter = (startPos + endPos) / 2;
            QPoint relativePos = connCenter - centerPoint;
            m_copiedConnections.append(copiedConnection);
            m_copiedConnectionsPositions.append(relativePos);
            int startShapeIndex = -1;
            int endShapeIndex = -1;
            ConnectionPoint::Position startPosition = ConnectionPoint::Free;
            ConnectionPoint::Position endPosition = ConnectionPoint::Free;
            if (conn->getStartPoint() && conn->getStartPoint()->getOwner()) {
                Shape* startShape = conn->getStartPoint()->getOwner();
                startShapeIndex = m_multiSelectedShapes.indexOf(startShape);
                startPosition = conn->getStartPoint()->getPositionType();
            }
            if (conn->getEndPoint() && conn->getEndPoint()->getOwner()) {
                Shape* endShape = conn->getEndPoint()->getOwner();
                endShapeIndex = m_multiSelectedShapes.indexOf(endShape);
                endPosition = conn->getEndPoint()->getPositionType();
            }
            m_copiedConnectionStartShapes.append(qMakePair(i, startShapeIndex));
            m_copiedConnectionEndShapes.append(qMakePair(i, endShapeIndex));
            m_copiedConnectionStartPoints.append(startPosition);
            m_copiedConnectionEndPoints.append(endPosition);
        }
    }
}
void DrawingArea::cutMultiSelectedShapes()
{
    if (m_multiSelectedShapes.isEmpty()) {
        return;
    }
    copyMultiSelectedShapes();
    QVector<Connection*> connectionsToRemove;
    for (Shape* shape : m_multiSelectedShapes) {
        for (Connection* connection : m_connections) {
            if ((connection->getStartPoint() && connection->getStartPoint()->getOwner() == shape) || 
                (connection->getEndPoint() && connection->getEndPoint()->getOwner() == shape)) {
                if (!connectionsToRemove.contains(connection)) {
                    connectionsToRemove.append(connection);
                }
            }
        }
    }
    for (Connection* connection : connectionsToRemove) {
        m_connections.removeOne(connection);
        delete connection;
    }
    for (Shape* shape : m_multiSelectedShapes) {
        m_shapes.removeOne(shape);
        delete shape;
    }
    m_multiSelectedShapes.clear();
    m_selectedShape = nullptr;
    emit shapeSelectionChanged(false);
    emit multiSelectionChanged(false);
    emit shapesCountChanged(getShapesCount());
    update();
}
void DrawingArea::setSelectedShapeX(int x)
{
    if (!m_selectedShape) return;
    QRect rect = m_selectedShape->getRect();
    rect.moveLeft(x);
    m_selectedShape->setRect(rect);
    update();
    emit shapePositionChanged(rect.topLeft());
}
void DrawingArea::setSelectedShapeY(int y)
{
    if (!m_selectedShape) return;
    QRect rect = m_selectedShape->getRect();
    rect.moveTop(y);
    m_selectedShape->setRect(rect);
    update();
    emit shapePositionChanged(rect.topLeft());
}
void DrawingArea::setSelectedShapeWidth(int width)
{
    if (!m_selectedShape) return;
    width = qMax(1, width);
    QRect rect = m_selectedShape->getRect();
    rect.setWidth(width);
    m_selectedShape->setRect(rect);
    update();
    emit shapeSizeChanged(rect.size());
}
void DrawingArea::setSelectedShapeHeight(int height)
{
    if (!m_selectedShape) return;
    height = qMax(1, height);
    QRect rect = m_selectedShape->getRect();
    rect.setHeight(height);
    m_selectedShape->setRect(rect);
    update();
    emit shapeSizeChanged(rect.size());
}
