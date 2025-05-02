#include "drawingarea.h"
#include <QPainter>
#include <QMimeData>
#include "shape/shapefactory.h"
#include <QKeyEvent>
#include "customtextedit.h"

DrawingArea::DrawingArea(QWidget *parent)
    : QWidget(parent), m_selectedShape(nullptr), m_dragging(false),
      m_activeHandle(Shape::None), m_resizing(false), m_textEditor(nullptr),
      m_currentConnection(nullptr), m_hoveredShape(nullptr)
{
    // 设置接受拖放 (Set accept drops)
    setAcceptDrops(true);

    // 设置背景色为白色 (Set background color to white)
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setAutoFillBackground(true);
    setMouseTracking(true);
    setPalette(pal);
}

DrawingArea::~DrawingArea()
{
    // 清理所有形状 (Clean up all shapes)
    qDeleteAll(m_shapes);
    m_shapes.clear();
    
    // 清理所有连线
    qDeleteAll(m_connections);
    m_connections.clear();
    
    if (m_textEditor) {
        delete m_textEditor;
    }
}

void DrawingArea::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制所有连线
    for (Connection *connection : m_connections) {
        connection->paint(&painter);
    }
    
    // 绘制正在创建的连线
    if (m_currentConnection) {
        m_currentConnection->paint(&painter);
    }
    
    // 绘制所有形状
    for (Shape *shape : m_shapes) {
        shape->paint(&painter);
        
        // 绘制连接点（当鼠标悬停时）
        shape->drawConnectionPoints(&painter);
        
        // 如果是当前选中的形状，绘制一个选中框和调整大小的手柄
        if (shape == m_selectedShape) {
            // 临时禁用抗锯齿以获得更细的线条
            painter.setRenderHint(QPainter::Antialiasing, false);
            
            QPen dashPen(Qt::blue, 0); // 使用0宽度的笔，会被渲染为设备支持的最细线条
            QVector<qreal> dashes;
            dashes << 2 << 2;
            dashPen.setDashPattern(dashes);
            dashPen.setStyle(Qt::CustomDashLine); // 确保使用自定义虚线样式
            painter.setPen(dashPen);
            painter.setBrush(Qt::NoBrush);
            
            // 只绘制一次矩形，避免双线效果
            painter.drawRect(shape->getRect());
            
            // 重新启用抗锯齿以绘制手柄
            painter.setRenderHint(QPainter::Antialiasing, true);
            
            // 绘制调整大小的手柄
            shape->drawResizeHandles(&painter);
        }
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
        // 获取形状类型
        QString type = event->mimeData()->text();
        
        // 使用工厂创建形状对象
        int basis = 55; // 基准值设为55
        Shape *newShape = ShapeFactory::instance().createShape(type, basis);
        
        // 设置形状位置
        if (newShape) {
            //设置新创建形状在绘图区域中的位置
            QRect shapeRect = newShape->getRect();
            shapeRect.moveCenter(event->pos());
            newShape->setRect(shapeRect);
            m_selectedShape = newShape; // 设置为当前选中形状
            // 添加形状到列表
            m_shapes.append(newShape);
            update();
        }
        
        event->acceptProposedAction();
    }
}

void DrawingArea::mouseMoveEvent(QMouseEvent *event)
{
    // 更新正在创建的连线
    if (m_currentConnection) {
        m_currentConnection->setTemporaryEndPoint(event->pos());
        
        // 检测鼠标是否悬停在某个形状上（用于显示目标形状的连接点）
        Shape* targetShape = nullptr;
        for (int i = m_shapes.size() - 1; i >= 0; --i) {
            if (m_shapes[i]->contains(event->pos())) {
                targetShape = m_shapes[i];
                break;
            }
        }
        
        // 更新悬停状态
        if (m_hoveredShape != targetShape) {
            if (m_hoveredShape) {
                m_hoveredShape->setHovered(false);
            }
            
            m_hoveredShape = targetShape;
            
            if (m_hoveredShape) {
                m_hoveredShape->setHovered(true);
            }
        }
        
        update();
        return;
    }
    
    // 调整大小
    if (m_resizing && m_selectedShape) {
        QPoint delta = event->pos() - m_dragStart;
        m_selectedShape->resize(m_activeHandle, delta);
        m_dragStart = event->pos();
        update();
        return;
    }
    
    // 移动形状
    if (m_dragging && m_selectedShape) {
        // 计算拖动偏移
        QPoint delta = event->pos() - m_dragStart;

        // 更新形状位置
        QRect newRect = m_selectedShape->getRect();
        newRect.moveTo(m_shapeStart + delta);
        m_selectedShape->setRect(newRect);

        // 重绘
        update();
        return;
    }

    // 更新鼠标光标样式和形状悬停状态
    
    // 首先检查鼠标是否位于调整大小的手柄上（对于选中的形状）
    if (m_selectedShape) {
        Shape::HandlePosition handle = m_selectedShape->hitHandle(event->pos());
        
        if (handle != Shape::None) {
            // 根据手柄位置设置对应的光标样式
            switch (handle) {
                case Shape::TopLeft:
                case Shape::BottomRight:
                    setCursor(Qt::SizeFDiagCursor); // 斜向双向箭头 ↘↖
                    return;
                case Shape::TopRight:
                case Shape::BottomLeft:
                    setCursor(Qt::SizeBDiagCursor); // 斜向双向箭头 ↗↙
                    return;
                case Shape::Top:
                case Shape::Bottom:
                    setCursor(Qt::SizeVerCursor); // 垂直双向箭头 ↕
                    return;
                case Shape::Left:
                case Shape::Right:
                    setCursor(Qt::SizeHorCursor); // 水平双向箭头 ↔
                    return;
            }
        }
        
        // 检查是否点击了连接点
        ConnectionPoint* cp = m_selectedShape->hitConnectionPoint(event->pos());
        if (cp) {
            setCursor(Qt::CrossCursor); // 十字光标表示可以开始连线
            return;
        }
    }
    
    // 检查鼠标是否位于任何形状上方，并更新悬停状态
    Shape* hoveredShape = nullptr;
    
    for (int i = m_shapes.size() - 1; i >= 0; --i) {
        if (m_shapes[i]->contains(event->pos())) {
            hoveredShape = m_shapes[i];
            setCursor(Qt::SizeAllCursor); // 四向箭头
            break;
        }
    }
    
    // 更新悬停状态
    if (m_hoveredShape != hoveredShape) {
        if (m_hoveredShape) {
            m_hoveredShape->setHovered(false);
        }
        
        m_hoveredShape = hoveredShape;
        
        if (m_hoveredShape) {
            m_hoveredShape->setHovered(true);
        }
        
        update(); // 重绘以更新连接点显示
    }
    
    // 如果鼠标不在任何形状或手柄上，设置为标准光标
    if (!hoveredShape) {
        setCursor(Qt::ArrowCursor);
    }
}

void DrawingArea::mousePressEvent(QMouseEvent *event)
{
    // 如果正在编辑文本，并且点击了编辑区域外，则结束编辑 (If editing text and clicked outside the editor, finish editing)
    if (m_textEditor && m_textEditor->isVisible()) {
        QRect editorRect = m_textEditor->geometry();
        if (!editorRect.contains(event->pos())) {
            finishTextEditing();
        }
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        // 如果有一个形状被悬停且点击了连接点，开始创建连线
        if (m_hoveredShape) {
            ConnectionPoint* cp = m_hoveredShape->hitConnectionPoint(event->pos());
            if (cp) {
                startConnection(cp);
                return;
            }
        }
        
        // 如果已选中形状，检查是否点击了调整手柄
        if (m_selectedShape) {
            Shape::HandlePosition handle = m_selectedShape->hitHandle(event->pos());
            if (handle != Shape::None) {
                m_activeHandle = handle;
                m_resizing = true;
                m_dragStart = event->pos();
                // 光标已经在mouseMoveEvent中设置，这里不需要再设置
                return;
            }
            
            // 检查是否点击了连接点
            ConnectionPoint* cp = m_selectedShape->hitConnectionPoint(event->pos());
            if (cp) {
                startConnection(cp);
                return;
            }
        }
        
        // 检查是否点击了现有的形状
        for (int i = m_shapes.size() - 1; i >= 0; --i) {
            if (m_shapes[i]->contains(event->pos())) {
                // 选中形状并准备拖动
                m_selectedShape = m_shapes[i];
                m_dragging = true;
                m_dragStart = event->pos();
                m_shapeStart = m_selectedShape->getRect().topLeft();
                
                // 将选中的形状移到最前面
                if (i < m_shapes.size() - 1) {
                    m_shapes.removeAt(i);
                    m_shapes.append(m_selectedShape);
                }
                
                update();
                return;
            }
        }
        
        // 如果点击空白区域，取消选中当前形状
        m_selectedShape = nullptr;
        update();
    }
}

void DrawingArea::mouseReleaseEvent(QMouseEvent *event)
{
    // 完成连接创建
    if (m_currentConnection && event->button() == Qt::LeftButton) {
        if (m_hoveredShape) {
            // 寻找最近的连接点
            ConnectionPoint* nearestPoint = findNearestConnectionPoint(m_hoveredShape, event->pos());
            if (nearestPoint) {
                completeConnection(nearestPoint);
            } else {
                cancelConnection();
            }
        } else {
            cancelConnection();
        }
        update();
        return;
    }
    
    // 如果正在调整大小，结束调整
    if (m_resizing) {
        m_resizing = false;
        m_activeHandle = Shape::None;
        return;
    }
    
    // 如果正在拖动，结束拖动
    if (m_dragging) {
        m_dragging = false;
        return;
    }
}

void DrawingArea::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 检查是否双击了某个形状 (Check if double-clicked on a shape)
    if (m_textEditor && m_textEditor->isVisible()) {
        return;  // 如果已经在编辑，忽略双击 (If already editing, ignore double-click)
    }
    
    // 查找点击的形状 (Find clicked shape)
    Shape* clickedShape = nullptr;
    for (int i = m_shapes.size() - 1; i >= 0; i--) {
        if (m_shapes.at(i)->contains(event->pos())) {
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
        m_textEditor->setFrameStyle(QFrame::Box);  // 设置边框样式为方框
        m_textEditor->setStyleSheet("border: 1px solid black;");  // 设置黑色边框
        m_textEditor->installEventFilter(this);
        m_textEditor->hide();
    }
}

void DrawingArea::startTextEditing()
{
    if (!m_selectedShape) return;
    
    createTextEditor();
    
    // 设置文本编辑器的位置和大小
    QRect textRect = m_selectedShape->textRect();
    m_textEditor->setGeometry(textRect);
    
    // 设置文本编辑器的内容
    m_textEditor->setPlainText(m_selectedShape->text());
    
    // 标记形状为编辑状态
    m_selectedShape->setEditing(true);
    
    // 显示文本编辑器并设置焦点
    m_textEditor->show();
    m_textEditor->setFocus();
}

void DrawingArea::finishTextEditing()
{
    if (!m_textEditor || !m_selectedShape) return;
    
    // 更新形状的文本内容
    m_selectedShape->setText(m_textEditor->toPlainText());
    
    // 结束编辑状态
    m_selectedShape->setEditing(false);
    m_textEditor->hide();
    
    // 更新绘图区域
    update();
}

void DrawingArea::cancelTextEditing()
{
    if (!m_textEditor || !m_selectedShape) return;
    
    // 不更新文本，仅结束编辑状态
    m_selectedShape->setEditing(false);
    m_textEditor->hide();
    
    // 更新绘图区域
    update();
}

bool DrawingArea::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_textEditor) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                // 按下ESC键取消编辑
                cancelTextEditing();
                return true;
            } else if (keyEvent->key() == Qt::Key_Return && 
                      (keyEvent->modifiers() & Qt::ControlModifier)) {
                // 按下Ctrl+Enter完成编辑
                finishTextEditing();
                return true;
            }
        } else if (event->type() == QEvent::FocusOut) {
            // 当文本编辑器失去焦点时完成编辑
            finishTextEditing();
            return true;
        }
    }
    
    return QWidget::eventFilter(watched, event);
}

// 连线相关方法实现
void DrawingArea::startConnection(ConnectionPoint* startPoint)
{
    // 创建新的连线
    m_currentConnection = new Connection(startPoint);
    
    // 设置初始临时终点为鼠标位置
    m_currentConnection->setTemporaryEndPoint(QCursor::pos());
    
    update();
}

void DrawingArea::completeConnection(ConnectionPoint* endPoint)
{
    if (!m_currentConnection) {
        return;
    }
    
    // 设置连线的终点
    m_currentConnection->setEndPoint(endPoint);
    
    // 如果是完整的连线，添加到连线列表
    if (m_currentConnection->isComplete()) {
        m_connections.append(m_currentConnection);
    } else {
        delete m_currentConnection;
    }
    
    m_currentConnection = nullptr;
    
    // 重置悬停状态
    if (m_hoveredShape) {
        m_hoveredShape->setHovered(false);
        m_hoveredShape = nullptr;
    }
}

void DrawingArea::cancelConnection()
{
    if (m_currentConnection) {
        delete m_currentConnection;
        m_currentConnection = nullptr;
    }
    
    // 重置悬停状态
    if (m_hoveredShape) {
        m_hoveredShape->setHovered(false);
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
    
    // 查找最近的连接点
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
    
    // 设置一个最大距离阈值，如果太远就不连接
    const double MAX_DISTANCE = 50.0;
    if (minDistance > MAX_DISTANCE) {
        return nullptr;
    }
    
    return nearest;
}
