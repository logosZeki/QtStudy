#include "drawingarea.h"
#include <QPainter>
#include <QMimeData>
#include <QKeyEvent>
#include <QApplication>
#include <QContextMenuEvent>
#include <QScrollArea>
#include <algorithm>  // 添加此行以使用std::swap
#include <QFontMetrics>
#include <QTextCharFormat>
#include <QTimer>
#include <QSvgGenerator>
#include <QDomDocument>
#include <QFile>
#include <QSvgRenderer>

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
      m_gridSize(20),
      m_gridThickness(1),
      m_multiSelectedShapes(),
      m_multySelectedConnections(),
      m_multyShapesStartPos(),
      m_isMultiRectSelecting(false),
      m_multiSelectionStart(),
      m_multiSelectionRect()
{
    // 启用接收拖放 (Enable accepting drops)
    setAcceptDrops(true);
    
    // 启用鼠标追踪以便处理悬停事件 (Enable mouse tracking to handle hover events)
    setMouseTracking(true);
    
    // 初始化页面设置相关变量
    m_backgroundColor = Qt::white;
    m_drawingAreaSize = QSize(Default_WIDTH, Default_HEIGHT); // 初始化绘图区域尺寸
    m_showGrid = true;
    m_gridColor = QColor(220, 220, 220);
    m_gridSize = 20;
    m_gridThickness = 1;
    
    // 设置绘图区域背景色为白色，更符合现代UI风格
    setStyleSheet("DrawingArea { background-color: white; }");
    
    // 设置焦点策略以捕获键盘事件 (Set focus policy to capture keyboard events)
    setFocusPolicy(Qt::StrongFocus);
    
    // 创建文本编辑器 (Create text editor)
    createTextEditor();
    
    // 创建右键菜单 (Create context menus)
    createContextMenus();
    
    // 设置初始大小，将在setScale方法中适当调整
    // 设置Widget的大小为绘图区域的3倍
    setMinimumSize(m_drawingAreaSize.width() * 3, m_drawingAreaSize.height() * 3);
    
    // 调用一次setScale以正确初始化大小
    QTimer::singleShot(0, this, [this]() {
        setScale(m_scale);
    });
    
    // 添加一个定时器，确保在界面完全加载后居中显示绘图区域
    QTimer::singleShot(100, this, [this]() {
        centerDrawingArea();
    });
}

DrawingArea::~DrawingArea()
{
    // 清理形状列表
    for (Shape* shape : m_shapes) {
        delete shape;
    }
    
    // 清理连接线列表
    for (Connection* connection : m_connections) {
        delete connection;
    }
    
    // 清理当前正在创建的连接线
    if (m_currentConnection) {
        delete m_currentConnection;
    }
    
    // 清理复制的形状
    if (m_copiedShape) {
        delete m_copiedShape;
    }
    
    // 清理复制的多选形状
    for (Shape* shape : m_copiedShapes) {
        delete shape;
    }
    
    // 清理复制的连接线
    for (Connection* conn : m_copiedConnections) {
        delete conn;
    }
    
    // 清理右键菜单
    if (m_shapeContextMenu) {
        delete m_shapeContextMenu;
    }
    
    if (m_canvasContextMenu) {
        delete m_canvasContextMenu;
    }
    
    // 清理文本编辑器
    if (m_textEditor) {
        m_textEditor->deleteLater();
    }
}

void DrawingArea::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 首先填充整个Widget为灰色
    painter.fillRect(rect(), QColor(230, 230, 230));
    // 保存当前变换状态
    painter.save();
    

    // 计算绘图区域在Widget中的位置（居中）,且固定左上角的顶点
    QRectF drawingRect(
        (width() - m_drawingAreaSize.width() * m_scale) / 2,
        (height() - m_drawingAreaSize.height() * m_scale) / 2,
        m_drawingAreaSize.width() * m_scale,
        m_drawingAreaSize.height() * m_scale
    );
    

    painter.translate(drawingRect.topLeft());
    
    // 应用缩放变换
    painter.scale(m_scale, m_scale);
    
    // 应用视图偏移
    painter.translate(-m_viewOffset);
    
    // 绘制背景（绘图区域）
    QRectF bgRect(0, 0, m_drawingAreaSize.width(), m_drawingAreaSize.height());
    painter.fillRect(bgRect, m_backgroundColor);
    
    // 在绘图区域绘制网格
    if (m_showGrid) {
        painter.setClipRect(bgRect);
        drawGrid(&painter);
        painter.setClipping(false);
    }
    
    // 绘制所有形状
    for (Shape *shape : m_shapes) {
        shape->paint(&painter);//包括绘制文字和形状
        
        // 绘制连接点（当鼠标悬停时）
        if(m_hoveredShape==shape){
            shape->drawConnectionPoints(&painter);
        }
        
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
        
        // 绘制多选状态下被选中的图形
        if (m_multiSelectedShapes.contains(shape)) {
            // 临时禁用抗锯齿以获得更细的线条
            painter.setRenderHint(QPainter::Antialiasing, false);
            
            QPen dashPen(Qt::blue, 0); // 使用0宽度的笔
            QVector<qreal> dashes;
            dashes << 2 << 2;
            dashPen.setDashPattern(dashes);
            dashPen.setStyle(Qt::CustomDashLine);
            painter.setPen(dashPen);
            painter.setBrush(Qt::NoBrush);
            
            // 绘制选中框，不绘制调整大小的手柄
            painter.drawRect(shape->getRect());
            
            // 重新启用抗锯齿
            painter.setRenderHint(QPainter::Antialiasing, true);
        }
    }
    
    // 绘制所有连线 - 移到形状绘制之后，确保连接线始终显示在最上层
    for (Connection *connection : m_connections) {
        // 当拖动端点时，不绘制被拖动的连接线，而是绘制预览
        if (m_movingConnectionPoint && connection == m_selectedConnection) {
            drawConnectionPreview(&painter, connection);
        } else {
            connection->paint(&painter);
        }
    }
    
    // 绘制正在创建的连线
    if (m_currentConnection) {
        m_currentConnection->paint(&painter);
    }
    
    // 恢复变换状态
    painter.restore();
    
    // 绘制选择框（如果正在框选）
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
        // 获取形状类型
        QString type = event->mimeData()->text();
        
        // 将拖放位置从视图坐标转换为场景坐标
        QPoint scenePos = mapToScene(event->pos());
        
        // 处理ArrowLine类型
        if (type == ShapeTypes::ArrowLine) {
            // 创建初始位置相同的临时直线，之后用户可以调整端点
            QPoint center = scenePos;
            QPoint startPoint = center - QPoint(40, 0);
            QPoint endPoint = center + QPoint(40, 0);
            createArrowLine(startPoint, endPoint);
            event->acceptProposedAction();
            
            // 发送图形数量变化信号
            emit shapesCountChanged(getShapesCount());
            
            return;
        }
        
        // 使用工厂创建其他形状对象
        int basis = 85; // 基准值设为85
        Shape *newShape = ShapeFactory::instance().createShape(type, basis);
        
        // 设置形状位置
        if (newShape) {
            //设置新创建形状在绘图区域中的位置
            QRect shapeRect = newShape->getRect();
            shapeRect.moveCenter(scenePos);
            newShape->setRect(shapeRect);
            m_selectedShape = newShape; // 设置为当前选中形状
            // 添加形状到列表
            m_shapes.append(newShape);
            
            // 发送图形数量变化信号
            emit shapesCountChanged(getShapesCount());
            
            update();
        }
        
        event->acceptProposedAction();
    }
}

void DrawingArea::updateCursor(QMouseEvent *event){

}
void DrawingArea::mouseMoveEvent(QMouseEvent *event)
{
    // 将鼠标坐标转换为场景坐标
    QPoint scenePos = mapToScene(event->pos());
    
    // 更新鼠标位置
    m_lastMousePos = event->pos();
    
    // 处理框选更新
    if (m_isMultiRectSelecting) {
        updateRectMultiSelection(event->pos());
        return;
    }
    
    // 拉住拉线拖动时的操作
    if (m_currentConnection) {
        m_temporaryEndPoint = scenePos;
        m_currentConnection->setTemporaryEndPoint(scenePos);
        bool isOverShape = false;
        for (int i = m_shapes.size() - 1; i >= 0; --i) {
            ConnectionPoint* cp = m_shapes[i]->hitConnectionPoint(scenePos, false);
            if(cp || m_shapes[i]->contains(scenePos)) {
                m_hoveredShape = m_shapes[i];
                setCursor(Qt::ArrowCursor); // 普通箭头
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
    
    // 移动连接线端点
    if (m_movingConnectionPoint && m_activeConnectionPoint) {
        // 更新拖动预览点
        m_connectionDragPoint = scenePos;
        
        // 检查是否悬停在某个形状上
        bool isOverShape = false;
        for (int i = m_shapes.size() - 1; i >= 0; --i) {
            ConnectionPoint* cp = m_shapes[i]->hitConnectionPoint(scenePos, false);
            if (cp) {
                // 悬停在连接点上，准备重新连接
                m_hoveredShape = m_shapes[i];
                setCursor(Qt::CrossCursor);
                isOverShape = true;
                break;
            } else if (m_shapes[i]->contains(scenePos)) {
                // 悬停在形状上，但不在连接点上
                m_hoveredShape = m_shapes[i];
                setCursor(Qt::ArrowCursor);
                isOverShape = true;
                break;
            }
        }
        
        if (!isOverShape) {
            // 不在任何形状上，直接移动到鼠标位置
            if (m_activeConnectionPoint->getOwner() == nullptr) {
                m_activeConnectionPoint->setPosition(scenePos);
            }
            m_hoveredShape = nullptr;
        }
        
        update();
        return;
    }
    
    // 调整大小
    if (m_resizing && m_selectedShape) {
        QPoint delta = event->pos() - m_dragStart;
        // 将delta转换为场景坐标
        QPoint sceneDelta = mapToScene(delta) - mapToScene(QPoint(0, 0));
        m_selectedShape->resize(m_activeHandle, sceneDelta);
        m_dragStart = event->pos();
        
        update();
        return;
    }
    
    // 移动形状或整条线或多个选中的图形
    if (m_dragging) {
        // 计算拖动偏移
        QPoint delta = event->pos() - m_dragStart;
        // 将delta转换为场景坐标
        QPoint sceneDelta = mapToScene(delta) - mapToScene(QPoint(0, 0));
        
        if (m_selectedShape) {
            // 移动单个图形
            QRect newRect = m_selectedShape->getRect();
            newRect.moveTo(m_shapeStart + sceneDelta);
            m_selectedShape->setRect(newRect);
        } else if (!m_multiSelectedShapes.isEmpty()) {
            // 批量移动多个图形
            for (int i = 0; i < m_multiSelectedShapes.size(); ++i) {
                QRect newRect = m_multiSelectedShapes[i]->getRect();
                newRect.moveTo(m_multyShapesStartPos[i] + sceneDelta);
                m_multiSelectedShapes[i]->setRect(newRect);
            }
        } else if (m_selectedConnection) {
            // 移动整条线
            Connection* conn = m_selectedConnection;
            if (conn->getStartPoint() && conn->getEndPoint() && 
                conn->getStartPoint()->getOwner() == nullptr && 
                conn->getEndPoint()->getOwner() == nullptr) {
                
                // 获取当前端点位置
                QPoint startPos = conn->getStartPosition();
                QPoint endPos = conn->getEndPosition();
                
                // 计算新位置
                QPoint newStartPos = startPos + sceneDelta;
                QPoint newEndPos = endPos + sceneDelta;
                
                // 更新端点位置
                conn->getStartPoint()->setPosition(newStartPos);
                conn->getEndPoint()->setPosition(newEndPos);
                
                // 更新拖动起点
                m_dragStart = event->pos();
            }
        }
        
        update();
        return;
    }
    
    // 检查鼠标是否位于连接线端点附近
    for (int i = m_connections.size() - 1; i >= 0; --i) {
        Connection* conn = m_connections[i];
        
        // 检查端点是否可以拖动（不在形状连接点附近）
        if (conn->isNearStartPoint(scenePos, 20)) {
            if (conn->getStartPoint()->getOwner() == nullptr || 
                !conn->getStartPoint()->getOwner()->hitConnectionPoint(scenePos, true)) {
                setCursor(Qt::SizeAllCursor); // 四向箭头
                return;
            }
        } else if (conn->isNearEndPoint(scenePos, 20)) {
            if (conn->getEndPoint()->getOwner() == nullptr || 
                !conn->getEndPoint()->getOwner()->hitConnectionPoint(scenePos, true)) {
                setCursor(Qt::SizeAllCursor); // 四向箭头
                return;
            }
        } else if (conn->contains(scenePos)) {
            // 在线条上
            if (conn->getStartPoint()->getOwner() == nullptr && 
                conn->getEndPoint()->getOwner() == nullptr) {
                // 独立线条可以拖动整条线
                setCursor(Qt::SizeAllCursor);
            } else {
                // 普通线条只能选中
                setCursor(Qt::PointingHandCursor);
            }
            return;
        }
    }
    
    // 更新鼠标光标样式和形状悬停状态
    // 首先检查鼠标是否位于调整大小的手柄上（对于选中的形状）
    if (m_selectedShape) {
        Shape::HandlePosition handle = m_selectedShape->hitHandle(scenePos);
        
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
    }
    
    // 检查鼠标是否位于任何形状上方，并更新悬停状态
    for (int i = m_shapes.size() - 1; i >= 0; --i) {
        ConnectionPoint* cp = m_shapes[i]->hitConnectionPoint(scenePos, true);
        if (cp && m_selectedShape != m_shapes[i]) {
            setCursor(Utils::getCrossCursor()); // 十字无箭头光标表示可以开始连线
            if (m_hoveredShape != m_shapes[i]) {
                m_hoveredShape = m_shapes[i];
                update(); // 重绘以更新连接点显示
            }
            return;
        } else if (m_shapes[i]->contains(scenePos)) {
            if (m_hoveredShape != m_shapes[i]) {
                m_hoveredShape = m_shapes[i];
                update(); // 重绘以更新连接点显示
            }
            setCursor(Qt::SizeAllCursor); // 四向箭头
            return;
        }
    }
    
    // 如果鼠标不在任何形状或手柄上，设置为标准光标
    setCursor(Qt::ArrowCursor);
        
    // 如果之前有悬停的形状，现在鼠标已经移出，需要清除悬停状态
    if (m_hoveredShape) {
        m_hoveredShape = nullptr;
        update();
    }
}

void DrawingArea::mousePressEvent(QMouseEvent *event)
{
    // 确保获得焦点以接收键盘事件
    setFocus();
    
    // 如果正在编辑文本，并且点击了编辑区域外，则结束编辑
    if (m_textEditor && m_textEditor->isVisible()) {
        QRect editorRect = m_textEditor->geometry();
        if (!editorRect.contains(event->pos())) {
            finishTextEditing();
        }
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        // 将鼠标坐标转换为场景坐标
        QPoint scenePos = mapToScene(event->pos());
        
        // 首先检查是否点击了图形的连接点
        if (m_hoveredShape && m_hoveredShape!=m_selectedShape){
            ConnectionPoint* cp = m_hoveredShape->hitConnectionPoint(scenePos, true);
            if (cp) {
                startConnection(cp);
                return;
            }
        }
        
        // 然后检查是否点击了线条端点
        for (int i = m_connections.size() - 1; i >= 0; --i) {
            Connection* conn = m_connections[i];
            if (conn->isNearStartPoint(scenePos, 20)) {
                // 如果端点离连接点足够远，才可以拖动端点
                if (conn->getStartPoint()->getOwner() == nullptr || 
                    !conn->getStartPoint()->getOwner()->hitConnectionPoint(scenePos, true)) {
                    // 选中连接线
                    selectConnection(conn);
                    m_movingConnectionPoint = true;
                    m_activeConnectionPoint = conn->getStartPoint();
                    m_dragStart = event->pos();
                    m_connectionDragPoint = scenePos;
                    setCursor(Qt::SizeAllCursor); // 四向箭头
                    return;
                }
            } else if (conn->isNearEndPoint(scenePos, 20)) {
                // 如果端点离连接点足够远，才可以拖动端点
                if (conn->getEndPoint()->getOwner() == nullptr || 
                    !conn->getEndPoint()->getOwner()->hitConnectionPoint(scenePos, true)) {
                    // 选中连接线
                    selectConnection(conn);
                    m_movingConnectionPoint = true;
                    m_activeConnectionPoint = conn->getEndPoint();
                    m_dragStart = event->pos();
                    m_connectionDragPoint = scenePos;
                    setCursor(Qt::SizeAllCursor); // 四向箭头
                    return;
                }
            } else if (conn->contains(scenePos)) {
                // 点击了线条中间
                // 检查是否是独立的线条（两端都未连接到图形）
                bool isIndependentLine = 
                    (conn->getStartPoint()->getOwner() == nullptr && 
                     conn->getEndPoint()->getOwner() == nullptr);
                
                selectConnection(conn);
                
                if (isIndependentLine) {
                    // 准备拖动整条线
                    m_dragging = true;
                    m_dragStart = event->pos();
                }
                return;
            }
        }
    
        // 然后检查是否点击了形状的调整手柄
        if (m_selectedShape) {
            Shape::HandlePosition handle = m_selectedShape->hitHandle(scenePos);
            if (handle != Shape::None) {
                m_activeHandle = handle;
                m_resizing = true;
                m_dragStart = event->pos();
                return;
            }
        }
        
        // 检查是否点击了多选中的图形之一
        if (!m_multiSelectedShapes.isEmpty()) {
            bool clickedSelectedShape = false;
            
            for (int i = 0; i < m_multiSelectedShapes.size(); ++i) {
                if (m_multiSelectedShapes[i]->contains(scenePos)) {
                    // 准备拖动所有选中的图形
                    m_dragging = true;
                    m_dragStart = event->pos();
                    
                    // 记录所有图形的起始位置，用于计算拖动偏移
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
        
        // 检查是否点击了现有的形状
        Shape* oldSelectedShape = m_selectedShape;  // 记录旧的选中图形
        
        for (int i = m_shapes.size() - 1; i >= 0; --i) {
            if (m_shapes[i]->contains(scenePos)) {
                // 如果按住Ctrl键，则进行多选
                if (event->modifiers() & Qt::ControlModifier) {
                    // 如果点击已选中的图形，则取消其选择
                    if (m_shapes[i] == m_selectedShape) {
                        m_selectedShape = nullptr;
                        emit shapeSelectionChanged(false);
                    } else if (m_multiSelectedShapes.contains(m_shapes[i])) {
                        m_multiSelectedShapes.removeOne(m_shapes[i]);
                        if (m_multiSelectedShapes.isEmpty()) {
                            emit multiSelectionChanged(false);
                        }
                    } else {
                        // 如果有单选中的图形，先转换到多选模式
                        if (m_selectedShape) {
                            m_multiSelectedShapes.append(m_selectedShape);
                            m_selectedShape = nullptr;
                        }
                        
                        // 添加到多选列表
                        m_multiSelectedShapes.append(m_shapes[i]);
                        emit multiSelectionChanged(true);
                    }
                } else {
                    // 取消当前选中的连接线
                    if (m_selectedConnection) {
                        m_selectedConnection->setSelected(false);
                        m_selectedConnection = nullptr;
                    }
                    
                    // 清除多选状态
                    if (!m_multiSelectedShapes.isEmpty()) {
                        m_multiSelectedShapes.clear();
                        emit multiSelectionChanged(false);
                    }
                    
                    // 选中单个形状并准备拖动
                    m_selectedShape = m_shapes[i];
                    m_dragging = true;
                    m_dragStart = event->pos();
                    m_shapeStart = m_selectedShape->getRect().topLeft();
                    
                    // 如果选中的图形改变，发出信号
                    if (oldSelectedShape != m_selectedShape) {
                        emit shapeSelectionChanged(true);
                    }
                }
                
                update();
                return;
            }
        }
        
        // 如果点击空白区域且没有按Ctrl键，则清除当前选择
        if (!(event->modifiers() & Qt::ControlModifier)) {
            // 清除单选
            if (m_selectedShape) {
                m_selectedShape = nullptr;
                update();
                emit shapeSelectionChanged(false);
            }
            
            // 清除多选
            if (!m_multiSelectedShapes.isEmpty()) {
                m_multiSelectedShapes.clear();
                update();
                emit multiSelectionChanged(false);
            }
            
            // 清除选中的连接线
            if (m_selectedConnection) {
                m_selectedConnection->setSelected(false);
                m_selectedConnection = nullptr;
                update();
            }
            
            // 开始框选
            startRectMultiSelection(event->pos());
        }
    }

}

void DrawingArea::mouseReleaseEvent(QMouseEvent *event)
{
    // 将鼠标坐标转换为场景坐标
    QPoint scenePos = mapToScene(event->pos());
    
    // 结束框选过程
    if (m_isMultiRectSelecting && event->button() == Qt::LeftButton) {
        finishRectMultiSelection();
		update();
        return;
    }
    
    // 完成连接创建
    if (m_currentConnection && event->button() == Qt::LeftButton) {
        if (m_hoveredShape) {
            // 寻找最近的连接点
            ConnectionPoint* nearestPoint = findNearestConnectionPoint(m_hoveredShape, scenePos);
            // 如果找到连接点且不是起点，完成连接
            if (nearestPoint && (!m_currentConnection->getStartPoint()->equalTo(nearestPoint))) {
                completeConnection(nearestPoint);
            } else {
                // 如果没有找到连接点或者是同一个点，尝试连接到空白区域
                completeConnection(nullptr);
            }
        } else {
            // 直接连接到空白区域
            completeConnection(nullptr);
        }
        update();
        
        // 发出选择状态变化信号
        emit shapeSelectionChanged(m_selectedShape != nullptr);
        
        return;
    }
    
    // 如果正在移动连接线端点
    if (m_movingConnectionPoint && event->button() == Qt::LeftButton) {
        if (m_hoveredShape) {
            // 尝试连接到悬停形状的连接点
            ConnectionPoint* nearestPoint = findNearestConnectionPoint(m_hoveredShape, scenePos);
            if (nearestPoint) {
                // 检查是否是同一个连接线的另一个端点
                Connection* conn = m_selectedConnection;
                bool isStartPoint = (m_activeConnectionPoint == conn->getStartPoint());
                
                if (isStartPoint) {
                    // 检查是否试图连接到自己的终点
                    if (conn->getEndPoint() && !nearestPoint->equalTo(conn->getEndPoint())) {
                        // 替换起点
                        conn->setStartPoint(nearestPoint);
                    }
                } else {
                    // 检查是否试图连接到自己的起点
                    if (conn->getStartPoint() && !nearestPoint->equalTo(conn->getStartPoint())) {
                        // 替换终点
                        conn->setEndPoint(nearestPoint);
                    }
                }
            } else {
                // 没找到连接点，保持端点位置不变
            }
        } else {
            // 悬停在空白区域，创建一个自由位置的连接点
            ConnectionPoint* freePoint = new ConnectionPoint(scenePos);
            
            // 更新连接线端点
            Connection* conn = m_selectedConnection;
            bool isStartPoint = (m_activeConnectionPoint == conn->getStartPoint());
            
            if (isStartPoint) {
                conn->setStartPoint(freePoint);
            } else {
                conn->setEndPoint(freePoint);
            }
        }
        
        // 完成拖动操作
        m_movingConnectionPoint = false;
        m_activeConnectionPoint = nullptr;
        setCursor(Qt::ArrowCursor);
        update();
        return;
    }
    
    // 结束调整大小操作
    if (m_resizing && event->button() == Qt::LeftButton) {
        m_resizing = false;
        m_activeHandle = Shape::None;
        setCursor(Qt::ArrowCursor);
        update();
        return;
    }
    
    // 结束拖动操作
    if (m_dragging && event->button() == Qt::LeftButton) {
        m_dragging = false;
        m_multyShapesStartPos.clear(); // 清除批量移动时记录的起始位置
        setCursor(Qt::ArrowCursor);
        update();
        return;
    }
}

void DrawingArea::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 检查是否双击了某个形状 (Check if double-clicked on a shape)
    if (m_textEditor && m_textEditor->isVisible()) {
        return;  // 如果已经在编辑，忽略双击 (If already editing, ignore double-click)
    }
    
    // 将鼠标坐标转换为场景坐标
    QPoint scenePos = mapToScene(event->pos());
    
    // 查找点击的形状 (Find clicked shape)
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
        m_textEditor->setFrameStyle(QFrame::NoFrame);  // 设置为无边框
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
    
    // 将场景坐标转换为窗口坐标
    QPoint topLeft = mapFromScene(textRect.topLeft());
    QPoint bottomRight = mapFromScene(textRect.bottomRight());
    QRect viewRect(topLeft, bottomRight);
    
    m_textEditor->setGeometry(viewRect);
    
    // 设置文本编辑器的内容
    m_textEditor->setPlainText(m_selectedShape->text());
    
    // 获取图形的填充颜色并设置为编辑器的背景色
    QColor fillColor = m_selectedShape->fillColor();
    QColor transparentFillColor = fillColor; 
    double presentTransparency=m_selectedShape->transparency()/100.0; // 获取透明度
    if(presentTransparency==1.0){
        transparentFillColor.setAlphaF(1); 
    }else{//当透明度不为1时，设置编辑框背景全透明
        transparentFillColor.setAlphaF(0.0); // 编辑框背景全透明
    }
    

    // 设置编辑器样式，使用带有透明度的背景色
    // 使用 rgba() 格式，其中 %4 为 alpha 值
    QString styleSheet = QString("background-color: rgba(%1, %2, %3, %4); border: none;")
                            .arg(transparentFillColor.red())
                            .arg(transparentFillColor.green())
                            .arg(transparentFillColor.blue())
                            .arg(transparentFillColor.alphaF()); // 使用alphaF获取0.0-1.0的alpha值
    m_textEditor->setStyleSheet(styleSheet);
    
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
    
    // 设置初始临时终点为鼠标在绘图区域的位置，而不是屏幕坐标
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
        // 常规情况：连接到某个图形的连接点
        m_currentConnection->setEndPoint(endPoint);
    } else {
        // 连接到空白区域
        // 创建一个自由位置的连接点
        QPoint tempPos = m_currentConnection->getEndPosition(); // 使用当前连接线的临时终点位置
        //判断终点离起点的距离，太近就不连接
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
    
    // 如果是完整的连线，添加到连线列表并选中它
    if (m_currentConnection->isComplete()) {
        m_connections.append(m_currentConnection);
        selectConnection(m_currentConnection); // 选中新创建的连接线
        
        // 发送图形数量变化信号
        emit shapesCountChanged(getShapesCount());
    } else {
        delete m_currentConnection;
        m_currentConnection = nullptr;
    }
    
    m_currentConnection = nullptr;
    m_selectedShape = nullptr;
    
    // 重置悬停状态
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
    
    // 重置悬停状态
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
    
    
    return nearest;
}

void DrawingArea::contextMenuEvent(QContextMenuEvent *event)
{
    // 获取右键点击的场景坐标
    QPoint pos = event->pos();
    QPoint scenePos = mapToScene(pos);
    
    // 检查是否有多选图形
    if (!m_multiSelectedShapes.isEmpty()) {
        // 检查是否点击在多选图形上
        bool clickedOnSelectedShape = false;
        for (Shape* shape : m_multiSelectedShapes) {
            if (shape->contains(scenePos)) {
                clickedOnSelectedShape = true;
                break;
            }
        }
        
        if (clickedOnSelectedShape) {
            // 显示图形右键菜单
            showShapeContextMenu(pos);
            event->accept();
            return;
        }
    } 
    // 检查是否点击在单选图形上
    else if (m_selectedShape && m_selectedShape->contains(scenePos)) {
        showShapeContextMenu(pos);
        event->accept();
        return;
    }
    // 如果点击在选中的连接线上
    else if (m_selectedConnection) {
        for (Connection* connection : m_connections) {
            if (connection == m_selectedConnection && connection->contains(scenePos)) {
                showShapeContextMenu(pos);
                event->accept();
                return;
            }
        }
    }
    
    // 如果没有点击在图形或连接线上，显示画布右键菜单
    showCanvasContextMenu(pos);
    event->accept();
}

// 创建右键菜单
void DrawingArea::createContextMenus()
{
    // 创建图形右键菜单
    m_shapeContextMenu = new QMenu(this);
    
    // 添加复制、剪切和删除操作
    QAction *copyAction = m_shapeContextMenu->addAction(tr("Copy"));
    copyAction->setShortcut(QKeySequence::Copy);  // 设置Ctrl+C快捷键
    copyAction->setShortcutVisibleInContextMenu(true);  // 在右键菜单中显示快捷键
    connect(copyAction, &QAction::triggered, this, &DrawingArea::copySelectedShape);
    
    QAction *cutAction = m_shapeContextMenu->addAction(tr("Cut"));
    cutAction->setShortcut(QKeySequence::Cut);  // 设置Ctrl+X快捷键
    cutAction->setShortcutVisibleInContextMenu(true);  // 在右键菜单中显示快捷键
    connect(cutAction, &QAction::triggered, this, &DrawingArea::cutSelectedShape);
    
    QAction *deleteAction = m_shapeContextMenu->addAction(tr("Delete"));
    deleteAction->setShortcut(QKeySequence(Qt::Key_Delete));  // 设置Delete快捷键
    deleteAction->setShortcutVisibleInContextMenu(true);  // 在右键菜单中显示快捷键
    connect(deleteAction, &QAction::triggered, this, &DrawingArea::deleteSelectedShape);
    
    // 添加分隔线
    m_shapeContextMenu->addSeparator();
    
    // 添加图层管理操作
    QMenu *layerMenu = m_shapeContextMenu->addMenu(tr("Layer"));
    
    QAction *bringToFrontAction = layerMenu->addAction(tr("Move Shape To Top"));
    connect(bringToFrontAction, &QAction::triggered, this, &DrawingArea::moveShapeToTop);
    
    QAction *sendToBackAction = layerMenu->addAction(tr("Move Shape To Bottom"));
    connect(sendToBackAction, &QAction::triggered, this, &DrawingArea::moveShapeToBottom);
    
    QAction *bringForwardAction = layerMenu->addAction(tr("Move Shape Up"));
    connect(bringForwardAction, &QAction::triggered, this, &DrawingArea::moveShapeUp);
    
    QAction *sendBackwardAction = layerMenu->addAction(tr("Move Shape Down"));
    connect(sendBackwardAction, &QAction::triggered, this, &DrawingArea::moveShapeDown);
    
    // 创建画布右键菜单
    m_canvasContextMenu = new QMenu(this);
    
    // 添加粘贴操作
    QAction *pasteAction = m_canvasContextMenu->addAction(tr("Paste"));
    pasteAction->setShortcut(QKeySequence::Paste);  // 设置Ctrl+V快捷键
    pasteAction->setShortcutVisibleInContextMenu(true);  // 在右键菜单中显示快捷键
    // 注意：连接在showCanvasContextMenu中进行，因为需要获取右键点击位置
    
    // 添加全选操作
    QAction *selectAllAction = m_canvasContextMenu->addAction(tr("Select All"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);  // 设置Ctrl+A快捷键
    selectAllAction->setShortcutVisibleInContextMenu(true);  // 在右键菜单中显示快捷键
    connect(selectAllAction, &QAction::triggered, this, &DrawingArea::selectAllShapes);
}

// 显示图形右键菜单
void DrawingArea::showShapeContextMenu(const QPoint &pos)
{
    if (!m_shapeContextMenu) {
        createContextMenus();
    }
    
    QAction *copyAction = m_shapeContextMenu->actions().at(0);
    QAction *cutAction = m_shapeContextMenu->actions().at(1);
    QAction *deleteAction = m_shapeContextMenu->actions().at(2);
    QMenu *layerMenu = m_shapeContextMenu->actions().at(4)->menu();
    
    // 断开旧的连接
    disconnect(copyAction, nullptr, this, nullptr);
    disconnect(cutAction, nullptr, this, nullptr);
    disconnect(deleteAction, nullptr, this, nullptr);
    
    // 检查是否有多选状态
    if (!m_multiSelectedShapes.isEmpty()) {
        // 多选状态下的菜单操作
        connect(copyAction, &QAction::triggered, this, &DrawingArea::copyMultiSelectedShapes);
        connect(cutAction, &QAction::triggered, this, &DrawingArea::cutMultiSelectedShapes);
        connect(deleteAction, &QAction::triggered, this, [this]() {
            cutMultiSelectedShapes(); // 使用cut但不复制
        });
        
        // 多选状态下禁用图层操作
        layerMenu->setEnabled(false);
    } else {
        // 单选状态下的菜单操作
        connect(copyAction, &QAction::triggered, this, &DrawingArea::copySelectedShape);
        connect(cutAction, &QAction::triggered, this, &DrawingArea::cutSelectedShape);
        connect(deleteAction, &QAction::triggered, this, &DrawingArea::deleteSelectedShape);
        
        // 单选状态下启用图层操作
        layerMenu->setEnabled(true);
    }
    
    // 显示右键菜单
    m_shapeContextMenu->exec(mapToGlobal(pos));
}

// 显示画布右键菜单
void DrawingArea::showCanvasContextMenu(const QPoint &pos)
{
    if (!m_canvasContextMenu) {
        createContextMenus();
    }
    
    // 获取右键点击位置（场景坐标）
    QPoint canvasPos = pos;
    
    // 获取粘贴操作
    QAction *pasteAction = m_canvasContextMenu->actions().at(0);
    
    // 根据剪贴板内容设置粘贴操作的启用状态
    pasteAction->setEnabled(m_copiedShape != nullptr || !m_copiedShapes.isEmpty());
    
    // 断开旧的连接
    disconnect(pasteAction, nullptr, this, nullptr);
    
    // 建立新的连接，传递右键点击位置
    connect(pasteAction, &QAction::triggered, this, [this, canvasPos]() {
        this->pasteShape(canvasPos);
    });
    
    // 显示右键菜单
    m_canvasContextMenu->exec(mapToGlobal(pos));
}

// 图层管理方法实现

// 向上移动一层
void DrawingArea::moveShapeUp()
{
    if (!m_selectedShape) return;
    
    int index = m_shapes.indexOf(m_selectedShape);
    if (index < m_shapes.size() - 1) {
        // 使用std::swap替换QVector::swap
        std::swap(m_shapes[index], m_shapes[index + 1]);
        update();
    }
}

// 向下移动一层
void DrawingArea::moveShapeDown()
{
    if (!m_selectedShape) return;
    
    int index = m_shapes.indexOf(m_selectedShape);
    if (index > 0) {
        // 使用std::swap替换QVector::swap
        std::swap(m_shapes[index], m_shapes[index - 1]);
        update();
    }
}

// 移到最顶层
void DrawingArea::moveShapeToTop()
{
    if (!m_selectedShape) return;
    
    int index = m_shapes.indexOf(m_selectedShape);
    if (index < m_shapes.size() - 1) {
        m_shapes.removeAt(index);
        m_shapes.append(m_selectedShape);
        update();
    }
}

// 移到最底层
void DrawingArea::moveShapeToBottom()
{
    if (!m_selectedShape) return;
    
    int index = m_shapes.indexOf(m_selectedShape);
    if (index > 0) {
        m_shapes.removeAt(index);
        m_shapes.prepend(m_selectedShape);
        update();
    }
}

// 剪切板操作方法实现

// 复制选中的图形
void DrawingArea::copySelectedShape()
{
    // 如果有多个图形被选中，使用多选复制方法
    if (!m_multiSelectedShapes.isEmpty()) {
        copyMultiSelectedShapes();
        return;
    }
    
    if (!m_selectedShape) return;
    
    // 如果已有复制的图形，删除它
    if (m_copiedShape) {
        delete m_copiedShape;
        m_copiedShape = nullptr;
    }
    
    // 清空多选复制的数据（因为当前是单选复制）
    for (Shape* shape : m_copiedShapes) {
        delete shape;
    }
    m_copiedShapes.clear();
    m_copiedShapesPositions.clear();
    
    // 使用工厂创建一个同类型的新图形
    QString type = m_selectedShape->type();
    int basis = m_selectedShape->getRect().width() / 2;
    m_copiedShape = ShapeFactory::instance().createShape(type, basis);
    
    if (m_copiedShape) {
        // 复制属性
        m_copiedShape->setRect(m_selectedShape->getRect());
        m_copiedShape->setText(m_selectedShape->text());
        m_copiedShape->setFillColor(m_selectedShape->fillColor());
        m_copiedShape->setLineColor(m_selectedShape->lineColor());
        m_copiedShape->setLineWidth(m_selectedShape->lineWidth());
        m_copiedShape->setLineStyle(m_selectedShape->lineStyle());
        m_copiedShape->setTransparency(m_selectedShape->transparency());
    }
}

// 剪切选中的图形
void DrawingArea::cutSelectedShape()
{
    if (!m_selectedShape && !m_selectedConnection) return;
    
    // 如果选中的是图形
    if (m_selectedShape) {
        // 先复制
        copySelectedShape();
    }
    
    // 删除选中的对象（图形或连接线）
    deleteSelectedShape();
}

// 粘贴图形
void DrawingArea::pasteShape(const QPoint &pos)
{
    // 处理多选复制的粘贴
    if (!m_copiedShapes.isEmpty() || !m_copiedConnections.isEmpty()) {
        // 清除当前选择
        m_selectedShape = nullptr;
        m_multiSelectedShapes.clear();
        m_selectedConnection = nullptr;
        m_multySelectedConnections.clear();
        
        QPoint pastePos = pos.isNull() ? mapFromGlobal(QCursor::pos()) : pos;
        QPoint scenePos = mapToScene(pastePos);
        
        // 创建并粘贴每个复制的图形
        for (int i = 0; i < m_copiedShapes.size(); ++i) {
            Shape* sourceShape = m_copiedShapes[i];
            QPoint relativePos = m_copiedShapesPositions[i];
            
            // 使用工厂创建一个同类型的新图形
            QString type = sourceShape->type();
            int basis = sourceShape->getRect().width() / 2;
            Shape* newShape = ShapeFactory::instance().createShape(type, basis);
            
            if (newShape) {
                // 复制属性
                QRect rect = sourceShape->getRect();
                
                // 计算新位置（中心点 + 相对位置）
                rect.moveCenter(scenePos + relativePos);
                
                newShape->setRect(rect);
                newShape->setText(sourceShape->text());
                newShape->setFillColor(sourceShape->fillColor());
                newShape->setLineColor(sourceShape->lineColor());
                newShape->setLineWidth(sourceShape->lineWidth());
                newShape->setLineStyle(sourceShape->lineStyle());
                newShape->setTransparency(sourceShape->transparency());
                
                // 添加到图形列表和多选列表
                m_shapes.append(newShape);
                m_multiSelectedShapes.append(newShape);
            }
        }
        
        // 创建并粘贴每个复制的连接线
        for (int i = 0; i < m_copiedConnections.size(); ++i) {
            Connection* sourceConn = m_copiedConnections[i];
            QPoint relativePos = m_copiedConnectionsPositions[i];
            
            // 获取原始连接线的起点和终点
            QPoint origStartPos = sourceConn->getStartPosition();
            QPoint origEndPos = sourceConn->getEndPosition();
            
            // 计算起点和终点的相对位置（相对于连接线中心点）
            QPoint origCenter = (origStartPos + origEndPos) / 2;
            QPoint startRelativePos = origStartPos - origCenter;
            QPoint endRelativePos = origEndPos - origCenter;
            
            // 计算新的中心点位置
            QPoint newCenter = scenePos + relativePos;
            
            // 计算新的起点和终点位置
            QPoint newStartPos = newCenter + startRelativePos;
            QPoint newEndPos = newCenter + endRelativePos;
            
            // 查找此连接线的起点和终点连接关系
            int startShapeIndex = -1;
            int endShapeIndex = -1;
            ConnectionPoint::Position startPosition = ConnectionPoint::Free;
            ConnectionPoint::Position endPosition = ConnectionPoint::Free;
            
            // 查找连接线的索引
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
            
            // 创建新的连接点
            ConnectionPoint* startPoint = nullptr;
            ConnectionPoint* endPoint = nullptr;
            
            // 如果起点连接到图形
            if (startShapeIndex >= 0 && startShapeIndex < m_multiSelectedShapes.size()) {
                Shape* startShape = m_multiSelectedShapes[startShapeIndex];
                // 获取对应位置的连接点
                QVector<ConnectionPoint*> points = startShape->getConnectionPoints();
                for (ConnectionPoint* point : points) {
                    if (point->getPositionType() == startPosition) {
                        startPoint = point;
                        break;
                    }
                }
            } else {
                // 如果不连接到图形，创建自由连接点
                startPoint = new ConnectionPoint(newStartPos);
            }
            
            // 如果终点连接到图形
            if (endShapeIndex >= 0 && endShapeIndex < m_multiSelectedShapes.size()) {
                Shape* endShape = m_multiSelectedShapes[endShapeIndex];
                // 获取对应位置的连接点
                QVector<ConnectionPoint*> points = endShape->getConnectionPoints();
                for (ConnectionPoint* point : points) {
                    if (point->getPositionType() == endPosition) {
                        endPoint = point;
                        break;
                    }
                }
            } else {
                // 如果不连接到图形，创建自由连接点
                endPoint = new ConnectionPoint(newEndPos);
            }
            
            // 创建新的连接线
            ArrowLine* newConnection = new ArrowLine(newStartPos, newEndPos);
            
            // 设置连接线的端点
            if (startPoint) {
                newConnection->setStartPoint(startPoint);
            }
            if (endPoint) {
                newConnection->setEndPoint(endPoint);
            }
            
            // 添加到连接线列表和多选列表
            m_connections.append(newConnection);
            m_multySelectedConnections.append(newConnection);
            newConnection->setSelected(true);
        }
        
        // 发送多选状态变化信号
        if (!m_multiSelectedShapes.isEmpty() || !m_multySelectedConnections.isEmpty()) {
            emit multiSelectionChanged(true);
            emit shapesCountChanged(getShapesCount());
            update();
        }
        
        return;
    }
    
    // 单个图形的粘贴处理（原有逻辑）
    if (!m_copiedShape) return;
    
    // 使用工厂创建一个同类型的新图形
    QString type = m_copiedShape->type();
    int basis = m_copiedShape->getRect().width() / 2;
    Shape *newShape = ShapeFactory::instance().createShape(type, basis);
    
    if (newShape) {
        // 复制属性
        QRect rect = m_copiedShape->getRect();
        
        // 如果提供了鼠标位置，则使用鼠标位置
        if (!pos.isNull()) {
            // 使用mapToScene将视图坐标转换为场景坐标
            QPoint scenePos = mapToScene(pos);
            rect.moveCenter(scenePos);
        } else {
            // 否则使用默认偏移（保持向前兼容）
            rect.translate(20, 20);
        }
        
        newShape->setRect(rect);
        newShape->setText(m_copiedShape->text());
        newShape->setFillColor(m_copiedShape->fillColor());
        newShape->setLineColor(m_copiedShape->lineColor());
        newShape->setLineWidth(m_copiedShape->lineWidth());
        newShape->setLineStyle(m_copiedShape->lineStyle());
        newShape->setTransparency(m_copiedShape->transparency());
        
        // 添加到图形列表
        m_shapes.append(newShape);
        
        // 清除多选状态
        m_multiSelectedShapes.clear();
        m_multySelectedConnections.clear();
        m_selectedConnection = nullptr;
        // 选中新粘贴的图形
        m_selectedShape = newShape;
        
        // 发送信号
        emit shapeSelectionChanged(true);
        emit multiSelectionChanged(false);
        emit shapesCountChanged(getShapesCount());
        
        update();
    }
}

// 删除选中的图形或连接线
void DrawingArea::deleteSelectedShape()
{
    if (m_selectedShape) {
        // 将选中的形状从列表中移除
        int index = m_shapes.indexOf(m_selectedShape);
        if (index >= 0) {
            m_shapes.removeAt(index);
        }
        
        // 删除相关联的连接线
        QVector<Connection*> connectionsToRemove;
        
        for (Connection* connection : m_connections) {
            if ((connection->getStartPoint() && connection->getStartPoint()->getOwner() == m_selectedShape) || 
                (connection->getEndPoint() && connection->getEndPoint()->getOwner() == m_selectedShape)) {
                connectionsToRemove.append(connection);
            }
        }
        
        // 从连接线列表中移除并释放内存
        for (Connection* connection : connectionsToRemove) {
            m_connections.removeOne(connection);
            delete connection;
        }
        
        // 释放形状内存
        delete m_selectedShape;
        m_selectedShape = nullptr;
        
        // 发送选中状态变化信号
        emit shapeSelectionChanged(false);
        
        // 发送图形数量变化信号
        emit shapesCountChanged(getShapesCount());
        
        update();
    } else if (m_selectedConnection) {
        // 从连接线列表中移除并释放当前选中的连接线
        m_connections.removeOne(m_selectedConnection);
        delete m_selectedConnection;
        m_selectedConnection = nullptr;
        
        // 发送图形数量变化信号
        emit shapesCountChanged(getShapesCount());
        
        update();
    }
}

// 全选图形
void DrawingArea::selectAllShapes()
{
    // 目前DrawingArea只支持单选一个形状
    // 如果形状列表不为空，选择第一个形状
    if (!m_shapes.isEmpty()) {
        m_selectedShape = m_shapes.first();
        update();
    }
    
    // 注意：如果将来需要支持多选，可以在此处扩展功能
    // 例如添加一个m_multiSelectedShapes向量来存储多个选中的形状
}

// 键盘事件处理
void DrawingArea::keyPressEvent(QKeyEvent *event)
{
    // 如果正在编辑文本，让文本编辑器处理键盘事件
    if (m_textEditor && m_textEditor->isVisible()) {
        // 文本编辑器会通过eventFilter处理键盘事件
        QWidget::keyPressEvent(event);
        return;
    }
    
    // 使用标准快捷键
    if (event->matches(QKeySequence::SelectAll)) {
        // Ctrl+A 全选
        selectAllShapes();
    } else if (event->matches(QKeySequence::Copy)) {
        // Ctrl+C 复制（根据选择状态决定使用单选复制或多选复制）
        if (!m_multiSelectedShapes.isEmpty()) {
            copyMultiSelectedShapes();
        } else {
            copySelectedShape();
        }
    } else if (event->matches(QKeySequence::Cut)) {
        // Ctrl+X 剪切（根据选择状态决定使用单选剪切或多选剪切）
        if (!m_multiSelectedShapes.isEmpty()) {
            cutMultiSelectedShapes();
        } else {
            cutSelectedShape();
        }
    } else if (event->matches(QKeySequence::Paste)) {
        // Ctrl+V 粘贴
        // 使用鼠标当前位置
        pasteShape(mapFromGlobal(QCursor::pos()));
    } else if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        // Delete/Backspace 删除
        if (!m_multiSelectedShapes.isEmpty()) {
            cutMultiSelectedShapes(); // 使用cut但不复制
        } else {
            deleteSelectedShape();
        }
    } else {
        QWidget::keyPressEvent(event);
    }
}

// 创建独立箭头直线
void DrawingArea::createArrowLine(const QPoint& startPoint, const QPoint& endPoint)
{
    ArrowLine* arrowLine = new ArrowLine(startPoint, endPoint);
    m_connections.append(arrowLine);
    selectConnection(arrowLine);
    update();
}

// 选中连接线
void DrawingArea::selectConnection(Connection* connection)
{
    // 取消之前选中的图形
    m_selectedShape = nullptr;
    
    // 取消之前选中的连接线
    if (m_selectedConnection && m_selectedConnection != connection) {
        m_selectedConnection->setSelected(false);
    }
    
    m_selectedConnection = connection;
    
    // 设置新选中的连接线状态
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
    
    // 使用工厂创建相同类型的形状
    Shape* clonedShape = ShapeFactory::instance().createShape(sourceShape->type(), 55);
    if (!clonedShape) {
        return nullptr;
    }
    
    // 复制位置和大小
    clonedShape->setRect(sourceShape->getRect());
    
    // 复制文本
    clonedShape->setText(sourceShape->text());
    
    return clonedShape;
}

// 绘制连接线拖动预览
void DrawingArea::drawConnectionPreview(QPainter* painter, Connection* connection)
{
    if (!connection || !m_movingConnectionPoint || !m_activeConnectionPoint) 
        return;
    
    // 准备起点和终点
    QPoint startPos, endPos;
    
    if (m_activeConnectionPoint == connection->getStartPoint()) {
        // 拖动起点
        startPos = m_connectionDragPoint;
        endPos = connection->getEndPosition();
    } else {
        // 拖动终点
        startPos = connection->getStartPosition();
        endPos = m_connectionDragPoint;
    }
    
    // 确定是否需要绘制箭头
    bool drawArrow = false;
    
    // 如果是 ArrowLine 类型，总是绘制箭头
    ArrowLine* arrowLine = dynamic_cast<ArrowLine*>(connection);
    if (arrowLine) {
        drawArrow = true;
    } 
    // 对于普通 Connection，仅当拖动终点时绘制箭头
    else if (m_activeConnectionPoint == connection->getEndPoint()) {
        drawArrow = true;
    }
    
    // 使用共享绘制函数
    Connection::drawConnectionLine(painter, startPos, endPos, true, drawArrow);
}

// 绘制网格方法
void DrawingArea::drawGrid(QPainter *painter)
{
    if (!m_showGrid) return;
    
    painter->save();
    
    QPen gridPen(m_gridColor, m_gridThickness);
    painter->setPen(gridPen);
    
    // 在当前变换后的坐标系中绘制网格
    QRectF gridRect(0, 0, m_drawingAreaSize.width(), m_drawingAreaSize.height());
    
    // 绘制水平网格线
    for (int y = 0; y <= gridRect.height(); y += m_gridSize) {
        painter->drawLine(0, y, gridRect.width(), y);
    }
    
    // 绘制垂直网格线
    for (int x = 0; x <= gridRect.width(); x += m_gridSize) {
        painter->drawLine(x, 0, x, gridRect.height());
    }
    
    painter->restore();
}

// 页面设置相关方法
void DrawingArea::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    update();
}

void DrawingArea::setPageSize(const QSize &size)
{
    // 保存原来的绘图区域大小
    QSize oldSize = m_drawingAreaSize;
    
    // 设置新的绘图区域大小
    setDrawingAreaSize(size);
    
    // 更新界面
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
    // 此方法将在MainWindow中连接到页面设置对话框的应用信号
    update();
}

// 检查并自动扩展绘图区域
void DrawingArea::setScale(qreal scale)
{
// 限制缩放范围
    qreal newScale = qBound(MIN_SCALE, scale, MAX_SCALE);
    if (qFuzzyCompare(newScale, m_scale))
        return; // 如果缩放比例没有实质性变化，直接返回
    
    // 保存原始大小和缩放比例
    QSize oldSize = size();
    qreal oldScale = m_scale;
    
    // 记录滚动条的相对位置
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
        
        // 计算相对位置(0.0-1.0范围)，表示滚动条的百分比位置
        double hRatio = (hBar->maximum() > 0) ? 
                       (double)hBar->value() / hBar->maximum() : 0.5;
        double vRatio = (vBar->maximum() > 0) ? 
                       (double)vBar->value() / vBar->maximum() : 0.5;
        relativePos = QPointF(hRatio, vRatio);
    }
    
    // 更新缩放比例
    m_scale = newScale;
    
    // 计算新尺寸并调整大小
    // 注意：确保新尺寸始终大于滚动区域的视口尺寸，即使在缩小时
    QSize newSize(m_drawingAreaSize.width() * 3 * m_scale, 
                 m_drawingAreaSize.height() * 3 * m_scale);
    
    // 确保大小不会小于最小尺寸
    setFixedSize(newSize);  // 使用setFixedSize而不是resize，强制更新布局
    
    // 更新界面
    emit scaleChanged(m_scale);
    update();
    
    // 重新设置滚动条位置，保持相对位置不变
    if (scrollArea) {
        // 强制立即更新滚动区域几何布局
        scrollArea->updateGeometry();
        QScrollBar* hBar = scrollArea->horizontalScrollBar();
        QScrollBar* vBar = scrollArea->verticalScrollBar();
        
        // 根据相对位置设置新的滚动条值
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
    // 检查是否按住Ctrl键 - 进行缩放
    if (event->modifiers() & Qt::ControlModifier) {
        // 获取鼠标位置
        QPoint mousePos = event->position().toPoint();
        
        // 计算缩放因子
        qreal factor = 1.0;
        if (event->angleDelta().y() > 0) {
            factor = 1.2;  // 放大
        } else {
            factor = 1.0 / 1.2;  // 缩小
        }

        if(factor == 1.0) return;
        else{
            zoomInOrOut(factor);
        }
        
        
        // 重绘
        update();

        
        // 接受事件
        event->accept();
    } 
    // 检查是否按住Shift键 - 进行平移
    else if (event->modifiers() & Qt::ShiftModifier) {
        // 计算平移距离，使用滚轮垂直移动来水平滚动
        int delta = event->angleDelta().y();
        
        // 水平方向的滚动距离
        QPoint scrollDelta;
        
        // 如果有水平滚动，则使用水平滚动；否则默认使用垂直滚动作为水平移动
        if (event->angleDelta().x() != 0) {
            scrollDelta.setX(event->angleDelta().x() / 8);
        } else {
            scrollDelta.setX(delta / 3); // 调整滚动速度
        }
        
        // 尝试获取父滚动条并调整滚动位置
        QScrollBar* hBar = findParentScrollBar(Qt::Horizontal);
        if (hBar) {
            hBar->setValue(hBar->value() - scrollDelta.x());
        } else {
            // 如果没有找到滚动条，直接移动绘图区域的视图
            m_viewOffset.setX(m_viewOffset.x() - scrollDelta.x());
            update();
        }
        
        // 接受事件
        event->accept();
    } else {
        // 如果没有按修饰键，让父类处理滚轮事件
        QWidget::wheelEvent(event);
    }
}

QScrollBar* DrawingArea::findParentScrollBar(Qt::Orientation orientation) const
{
    // 遍历父窗口层次，查找QScrollArea
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
    // 计算绘图区域在窗口中的位置（居中）
    QPoint drawingAreaTopLeft(
        (width() - m_drawingAreaSize.width() * m_scale) / 2,
        (height() - m_drawingAreaSize.height() * m_scale) / 2
    );
    
    // 计算相对于绘图区域左上角的位置
    QPoint relativePos = viewPoint - drawingAreaTopLeft;
    
    // 应用反向缩放
    QPoint scaledPos(
        relativePos.x() / m_scale,
        relativePos.y() / m_scale
    );
    
    // 应用视图偏移
    QPoint scenePos = scaledPos + m_viewOffset;
    
    return scenePos;
}

QPoint DrawingArea::mapFromScene(const QPoint& scenePoint) const
{
    // 应用反向视图偏移
    QPoint pointWithoutOffset = scenePoint - m_viewOffset;
    
    // 应用缩放
    QPoint scaledPoint(
        pointWithoutOffset.x() * m_scale,
        pointWithoutOffset.y() * m_scale
    );
    
    // 计算绘图区域在窗口中的位置（居中）
    QPoint drawingAreaTopLeft(
        (width() - m_drawingAreaSize.width() * m_scale) / 2,
        (height() - m_drawingAreaSize.height() * m_scale) / 2
    );
    
    // 加上绘图区域的位置偏移，得到窗口坐标
    QPoint viewPoint = scaledPoint + drawingAreaTopLeft;
    
    return viewPoint;
}

void DrawingArea::setSelectedShapeFontFamily(const QString& family)
{
    if (m_selectedShape) {
        m_selectedShape->setFontFamily(family);
        QFont font = m_selectedShape->getFont();
        m_textEditor->setFont(font);// 更新文本编辑器的字体
        //m_textEditor->setFontFamily(family);  
        update();  // 更新绘图区域以显示变化
    }
}

void DrawingArea::setSelectedShapeFontSize(int size)
{
    if (m_selectedShape) {
        m_selectedShape->setFontSize(size);
        QFont font = m_selectedShape->getFont();
        m_textEditor->setFont(font);// 更新文本编辑器的字体
        //m_textEditor->setFontPointSize(size);  // 更新文本编辑器的字体大小
        update();  // 更新绘图区域以显示变化
    }
}

void DrawingArea::setSelectedShapeFontBold(bool bold)
{
    if (m_selectedShape) {
        m_selectedShape->setFontBold(bold);
        QFont font = m_selectedShape->getFont();
        m_textEditor->setFont(font);  // 更新文本编辑器的字体
        update();  // 更新绘图区域以显示变化
    }
}

void DrawingArea::setSelectedShapeFontItalic(bool italic)
{
    if (m_selectedShape) {
        m_selectedShape->setFontItalic(italic);
        QFont font = m_selectedShape->getFont();
        m_textEditor->setFont(font);  // 更新文本编辑器的字体
        update();  // 更新绘图区域以显示变化
    }
}

void DrawingArea::setSelectedShapeFontUnderline(bool underline)
{
    if (m_selectedShape) {
        m_selectedShape->setFontUnderline(underline);
        QFont font = m_selectedShape->getFont();
        m_textEditor->setFont(font);  // 更新文本编辑器的字体
        update();  // 更新绘图区域以显示变化
    }
}

void DrawingArea::setSelectedShapeFontColor(const QColor& color)
{
    if (m_selectedShape) {
        m_selectedShape->setFontColor(color);
        m_textEditor->setTextColor(color);  // 更新文本编辑器的字体颜色
        emit fontColorChanged(color);       // 发出字体颜色已更改的信号
        update();  // 更新绘图区域以显示变化
    }
}

void DrawingArea::setSelectedShapeTextAlignment(Qt::Alignment alignment)
{
    if (m_selectedShape) {
        m_selectedShape->setTextAlignment(alignment);
        m_textEditor->setAlignment(alignment);  // 更新文本编辑器的对齐方式
        update();  // 更新绘图区域以显示变化
    }
}


int DrawingArea::getShapesCount() const
{
    return m_shapes.size() + m_connections.size();
}

void DrawingArea::setDrawingAreaSize(const QSize &size)
{
    // 如果尺寸没有变化，直接返回
    if (size == m_drawingAreaSize)
        return;
    
    // 记录滚动条的相对位置
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
        
        // 计算相对位置(0.0-1.0范围)，表示滚动条的百分比位置
        double hRatio = (hBar->maximum() > 0) ? 
                       (double)hBar->value() / hBar->maximum() : 0.5;
        double vRatio = (vBar->maximum() > 0) ? 
                       (double)vBar->value() / vBar->maximum() : 0.5;
        relativePos = QPointF(hRatio, vRatio);
    }
    
    // 更新绘图区域尺寸
    m_drawingAreaSize = size;
    
    // 根据缩放比例计算新的窗口部件尺寸
    QSize newWidgetSize(m_drawingAreaSize.width() * 3 * m_scale, 
                       m_drawingAreaSize.height() * 3 * m_scale);
    
    // 使用setFixedSize设置新尺寸，强制更新布局
    setFixedSize(newWidgetSize);
    
    // 重新设置滚动条位置，保持相对位置不变
    if (scrollArea) {
        // 强制立即更新滚动区域几何布局
        scrollArea->updateGeometry();
        QScrollBar* hBar = scrollArea->horizontalScrollBar();
        QScrollBar* vBar = scrollArea->verticalScrollBar();
        
        // 根据相对位置设置新的滚动条值
        hBar->setValue(qRound(relativePos.x() * hBar->maximum()));
        vBar->setValue(qRound(relativePos.y() * vBar->maximum()));
    }
}

//第一次执行程序时候居中，调整尺寸的时候不要让滚动条居中
void DrawingArea::centerDrawingArea()
{
    // 查找父滚动区域
    QScrollArea* scrollArea = qobject_cast<QScrollArea*>(parentWidget());
    if (!scrollArea) {
        // 如果没有找到直接父滚动区域，尝试在整个父窗口层次结构中查找
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
        // 获取水平和垂直滚动条
        QScrollBar* hBar = scrollArea->horizontalScrollBar();
        QScrollBar* vBar = scrollArea->verticalScrollBar();
        
        if (hBar && vBar) {
            // 计算绘图区域的总尺寸（Widget尺寸）
            QSize totalSize = size();
            //qDebug() << "totalSize:" << totalSize;
            // 计算滚动区域视口的尺寸
            QSize viewportSize = scrollArea->viewport()->size();
            //qDebug() << "viewportSize:" << viewportSize;
            
            // 计算需要滚动的位置，使绘图区域居中
            // 这是将widget的中心位置与viewport的中心位置对齐所需的滚动值
            int hValue = (totalSize.width() - viewportSize.width()) / 2;
            int vValue = (totalSize.height() - viewportSize.height()) / 2;
            
            // 确保滚动值在有效范围内
            hValue = qMax(0, qMin(hValue, hBar->maximum()));
            vValue = qMax(0, qMin(vValue, vBar->maximum()));
            
            // 设置滚动条位置
            hBar->setValue(hValue);
            vBar->setValue(vValue);
        }
    }
    
    update();
}



// 实现导出为PNG的功能
bool DrawingArea::exportToPng(const QString &filePath)
{
    // 创建一个与绘图区域尺寸相同的图像
    QImage image(m_drawingAreaSize, QImage::Format_ARGB32);
    image.fill(m_backgroundColor); // 填充背景颜色
    
    // 创建QPainter在图像上绘制
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    // 保存当前可能的选中状态，以便稍后恢复
    Shape* tempSelectedShape = m_selectedShape;
    Connection* tempSelectedConnection = m_selectedConnection;
    
    // 临时取消选中状态，避免导出的图像显示选中框和手柄
    m_selectedShape = nullptr;
    m_selectedConnection = nullptr;
    
    // 不导出网格，无论当前设置如何
    // 注释掉或移除原有的网格绘制代码
    /*
    if (m_showGrid) {
        drawGrid(&painter);
    }
    */
    
    // 绘制所有形状
    for (Shape* shape : m_shapes) {
        shape->paint(&painter);
    }
    
    // 绘制所有连线
    for (Connection* connection : m_connections) {
        connection->paint(&painter);
    }
    
    // 恢复选中状态
    m_selectedShape = tempSelectedShape;
    m_selectedConnection = tempSelectedConnection;
    
    // 保存图像为PNG文件
    bool success = image.save(filePath, "PNG");
    
    // 更新视图
    update();
    
    return success;
}

// 实现导出为SVG的功能
bool DrawingArea::exportToSvg(const QString &filePath)
{
    // 创建SVG生成器
    QSvgGenerator generator;
    generator.setFileName(filePath);
    generator.setSize(m_drawingAreaSize);
    generator.setViewBox(QRect(0, 0, m_drawingAreaSize.width(), m_drawingAreaSize.height()));
    generator.setTitle(tr("flow chart"));
    generator.setDescription(tr("SVG file generated by flowchart designer"));
    
    // 保存当前的页面设置为元数据
    generator.setResolution(96); // 标准屏幕DPI
    
    // 准备更详细的元数据信息，包括每个形状和连接线的信息
    QString shapesMetadata = "<flowchart:shapes xmlns:flowchart=\"http://flowchart.zeqi.com/ns\">";
    
    // 添加所有形状的元数据
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
        
        // 添加文本内容
        shapesMetadata += QString(" text=\"%1\"").arg(shape->text().toHtmlEscaped());
        
        // 添加字体属性
        shapesMetadata += QString(" fontFamily=\"%1\" fontSize=\"%2\" fontBold=\"%3\" fontItalic=\"%4\" fontColor=\"%5\" fontUnderline=\"%6\"")
                               .arg(shape->fontFamily())
                               .arg(shape->fontSize())
                               .arg(shape->isFontBold() ? "true" : "false")
                               .arg(shape->isFontItalic() ? "true" : "false")
                               .arg(shape->fontColor().name())
                               .arg(shape->isFontUnderline() ? "true" : "false");
        
        shapesMetadata += " />";
    }
    
    // 添加所有连接线的元数据
    shapesMetadata += "<flowchart:connections>";
    for (int i = 0; i < m_connections.size(); ++i) {
        Connection* conn = m_connections[i];
        
        // 获取连接线的端点信息
        QPoint startPos = conn->getStartPosition();
        QPoint endPos = conn->getEndPosition();
        
        // 检查是否是ArrowLine类型
        bool isArrow = (dynamic_cast<ArrowLine*>(conn) != nullptr);
        
        // 获取端点连接的形状信息
        int startShapeIndex = -1;
        int startConnectionPointIndex = -1;
        int endShapeIndex = -1;
        int endConnectionPointIndex = -1;
        
        // 检查起点是否连接到形状
        if (conn->getStartPoint() && conn->getStartPoint()->getOwner()) {
            Shape* startShape = conn->getStartPoint()->getOwner();
            startShapeIndex = m_shapes.indexOf(startShape);
            
            // 查找连接点索引
            ConnectionPoint* startCP = conn->getStartPoint();
            ConnectionPoint::Position startPosition = startCP->getPositionType();
            if (startPosition == ConnectionPoint::Top) startConnectionPointIndex = 0;
            else if (startPosition == ConnectionPoint::Right) startConnectionPointIndex = 1;
            else if (startPosition == ConnectionPoint::Bottom) startConnectionPointIndex = 2;
            else if (startPosition == ConnectionPoint::Left) startConnectionPointIndex = 3;
        }
        
        // 检查终点是否连接到形状
        if (conn->getEndPoint() && conn->getEndPoint()->getOwner()) {
            Shape* endShape = conn->getEndPoint()->getOwner();
            endShapeIndex = m_shapes.indexOf(endShape);
            
            // 查找连接点索引
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
    
    // 创建完整的元数据字符串
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
    
    // 创建QPainter在SVG上绘制
    QPainter painter;
    painter.begin(&generator);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    // 保存当前可能的选中状态，以便稍后恢复
    Shape* tempSelectedShape = m_selectedShape;
    Connection* tempSelectedConnection = m_selectedConnection;
    
    // 临时取消选中状态，避免导出的SVG显示选中框和手柄
    m_selectedShape = nullptr;
    m_selectedConnection = nullptr;
    
    // 绘制背景
    painter.fillRect(QRect(0, 0, m_drawingAreaSize.width(), m_drawingAreaSize.height()), m_backgroundColor);
    
    // 绘制所有形状
    for (Shape* shape : m_shapes) {
        shape->paint(&painter);
    }
    
    // 绘制所有连线
    for (Connection* connection : m_connections) {
        connection->paint(&painter);
    }
    
    // 恢复选中状态
    m_selectedShape = tempSelectedShape;
    m_selectedConnection = tempSelectedConnection;
    
    // 结束绘制
    painter.end();
    
    // 更新视图
    update();
    
    // 添加元数据到已生成的SVG文件
    QFile file(filePath);
    if (file.open(QIODevice::ReadWrite)) {
        QByteArray svgData = file.readAll();
        QString svgText = QString::fromUtf8(svgData);
        
        // 在文件中寻找</defs>标签，并在其后插入元数据
        int defsEndPos = svgText.indexOf("</defs>");
        if (defsEndPos != -1) {
            svgText.insert(defsEndPos + 7, metadata);
            
            // 回到文件开头并重写内容
            file.seek(0);
            file.write(svgText.toUtf8());
            file.resize(file.pos());
        }
        
        file.close();
    }
    
    return true;
}

// 实现从SVG导入的功能
bool DrawingArea::importFromSvg(const QString &filePath)
{
    // 清除当前的所有图形和连线
    qDeleteAll(m_shapes);
    m_shapes.clear();
    qDeleteAll(m_connections);
    m_connections.clear();
    
    m_selectedShape = nullptr;
    m_selectedConnection = nullptr;
    
    // 尝试加载SVG文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    
    // 读取SVG文件内容
    QByteArray svgData = file.readAll();
    file.close();
    
    // 解析SVG文件内容为DOM
    QDomDocument domDocument;
    if (!domDocument.setContent(svgData))
        return false;
    
    // 首先查找元数据
    QDomElement metadataElement = domDocument.elementsByTagName("metadata").item(0).toElement();
    if (!metadataElement.isNull()) {
        // 解析DrawingArea设置
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
        
        // 找到形状元数据节点
        QDomElement shapesElement = metadataElement.firstChildElement("flowchart:shapes");
        if (!shapesElement.isNull()) {
            // 解析形状信息
            QDomNodeList shapeNodes = shapesElement.elementsByTagName("flowchart:shape");
            
            // 创建一个临时字典，用于存储形状ID与对象的对应关系
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
                
                // 计算一个合适的basis值，使用宽度或高度的一半作为基准
                int basis = qMin(width, height) / 2;
                basis = qMax(basis, 30); // 确保basis至少为30
                
                // 使用ShapeFactory创建相应类型的图形
                Shape* newShape = ShapeFactory::instance().createShape(type, basis);
                
                if (newShape) {
                    // 设置位置和大小
                    QRect rect(x, y, width, height);
                    newShape->setRect(rect);
                    
                    // 设置文本内容
                    newShape->setText(text);
                    
                    // 设置字体属性
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
                    
                    // 添加到图形列表
                    m_shapes.append(newShape);
                    
                    // 在映射中保存ID与形状的对应关系
                    shapeIdMap[id] = newShape;
                }
            }
            
            // 处理连接线元素
            QDomNodeList connectionNodes = shapesElement.elementsByTagName("flowchart:connection");
            for (int i = 0; i < connectionNodes.count(); ++i) {
                QDomElement connectionElement = connectionNodes.at(i).toElement();
                
                int startX = connectionElement.attribute("startX").toInt();
                int startY = connectionElement.attribute("startY").toInt();
                int endX = connectionElement.attribute("endX").toInt();
                int endY = connectionElement.attribute("endY").toInt();
                bool isArrow = (connectionElement.attribute("isArrow") == "true");
                
                // 获取连接的形状索引信息
                int startShapeIndex = connectionElement.attribute("startShapeIndex").toInt();
                int startConnectionPointIndex = connectionElement.attribute("startConnectionPointIndex").toInt();
                int endShapeIndex = connectionElement.attribute("endShapeIndex").toInt();
                int endConnectionPointIndex = connectionElement.attribute("endConnectionPointIndex").toInt();
                
                // 创建连接线
                ArrowLine* arrowLine = new ArrowLine(QPoint(startX, startY), QPoint(endX, endY));
                
                // 尝试建立连接关系，首先通过保存的索引关系
                bool startConnected = false;
                bool endConnected = false;
                
                // 如果有有效的形状索引和连接点索引，直接建立连接关系
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
                
                
                // 添加到连接线列表
                m_connections.append(arrowLine);
            }
        }
    }
    // 强制设置缩放比例为1.0
    setScale(1.0);
    
    // 更新视图
    update();
    
    // 发出图形数量变化信号
    emit shapesCountChanged(getShapesCount());
    emit selectionChanged();
    
    return true;
}

// 设置选中图形的填充颜色
void DrawingArea::setSelectedShapeFillColor(const QColor& color)
{
    if (m_selectedShape) {
        m_selectedShape->setFillColor(color);
        update();  // 更新绘图区域
        emit fillColorChanged(color);
    }
}

// 设置选中图形的线条颜色
void DrawingArea::setSelectedShapeLineColor(const QColor& color)
{
    if (m_selectedShape) {
        m_selectedShape->setLineColor(color);
        update();  // 更新绘图区域
        emit lineColorChanged(color);
    }
}

// 添加透明度、线条粗细和线条样式设置方法的实现
void DrawingArea::setSelectedShapeTransparency(int transparency)
{
    if (m_selectedShape) {
        // 设置透明度并更新视图
        m_selectedShape->setTransparency(transparency);
        update();
    }
}

void DrawingArea::setSelectedShapeLineWidth(qreal width)
{
    if (m_selectedShape) {
        // 设置线条粗细并更新视图
        m_selectedShape->setLineWidth(width);
        update();
    }
}

void DrawingArea::setSelectedShapeLineStyle(int style)
{
    if (m_selectedShape) {
        // 设置线条样式并更新视图
        m_selectedShape->setLineStyle(style);
        update();
    }
}

// 开始框选
void DrawingArea::startRectMultiSelection(const QPoint& point)
{
    m_isMultiRectSelecting = true;
    m_multiSelectionStart = mapToScene(point);
    m_multiSelectionRect = QRect(m_multiSelectionStart, QSize(0, 0));
    update();
}

// 更新框选矩形
void DrawingArea::updateRectMultiSelection(const QPoint& point)
{
    if (!m_isMultiRectSelecting)
        return;
    
    QPoint currentPos = mapToScene(point);
    m_multiSelectionRect = QRect(m_multiSelectionStart, currentPos).normalized();
    update();
}

// 完成框选过程
void DrawingArea::finishRectMultiSelection()
{
    if (!m_isMultiRectSelecting)
        return;
    
    selectMultiShapesInRect(m_multiSelectionRect);
    m_isMultiRectSelecting = false;
}

// 判断图形是否完全包含在选择矩形内
bool DrawingArea::isShapeCompletelyInRect(Shape* shape, const QRect& rect) const
{
    // 使用shape的contains方法来判断矩形的四个角是否都在图形内
    QRect shapeRect = shape->getRect();
    return rect.contains(shapeRect);
}

// 选择矩形内的所有图形
void DrawingArea::selectMultiShapesInRect(const QRect& rect)
{
    // 清除之前的选择
    clearMultySelection();
    
    // 遍历所有图形，检查是否完全在选择框内
    for (int i = 0; i < m_shapes.size(); ++i) {
        if (isShapeCompletelyInRect(m_shapes[i], rect)) {
            m_multiSelectedShapes.append(m_shapes[i]);
        }
    }
    
    // 遍历所有连接线，检查是否完全在选择框内
    for (int i = 0; i < m_connections.size(); ++i) {
        Connection* conn = m_connections[i];
        QPoint startPos = conn->getStartPosition();
        QPoint endPos = conn->getEndPosition();
        
        // 如果连接线的两个端点都在选择框内，就选中它
        if (rect.contains(startPos) && rect.contains(endPos)) {
            m_multySelectedConnections.append(conn);
            conn->setSelected(true);
        }
    }
    
    // 如果只选中了一个图形，则设置为单选
    if (m_multiSelectedShapes.size() == 1) {
        m_selectedShape = m_multiSelectedShapes.first();
        m_multiSelectedShapes.clear();
        emit shapeSelectionChanged(true);
    } else if (m_multiSelectedShapes.size() > 1) {
        // 多选状态
        m_selectedShape = nullptr;
        emit multiSelectionChanged(true);
    } else if (m_multySelectedConnections.size() == 1) {
        // 如果只选中了一条连接线
        selectConnection(m_multySelectedConnections.first());
        m_multySelectedConnections.clear();
    } else if (m_multiSelectedShapes.isEmpty() && m_multySelectedConnections.isEmpty()) {
        // 没有选中任何东西
        emit shapeSelectionChanged(false);
        emit multiSelectionChanged(false);
    } else {
        // 选中了多条连接线或图形和连接线混合
        emit multiSelectionChanged(true);
    }
}

// 清除所有选择状态
void DrawingArea::clearMultySelection()
{
    // 清除单选
    m_selectedShape = nullptr;
    
    // 清除多选的图形
    m_multiSelectedShapes.clear();
    
    // 清除选中的连接线
    for (Connection* conn : m_multySelectedConnections) {
        conn->setSelected(false);
    }
    m_multySelectedConnections.clear();
    
    // 清除当前选中的连接线
    if (m_selectedConnection) {
        m_selectedConnection->setSelected(false);
        m_selectedConnection = nullptr;
    }
    
    // 发送信号
    emit shapeSelectionChanged(false);
    emit multiSelectionChanged(false);
}

// 绘制选择矩形
void DrawingArea::drawMultiSelectionRect(QPainter* painter)
{
    if (!m_isMultiRectSelecting)
        return;
    
    // 设置半透明蓝色填充和边框
    QColor fillColor(0, 120, 215, 40);
    QColor borderColor(0, 120, 215);
    
    painter->save();
    painter->setBrush(fillColor);
    painter->setPen(QPen(borderColor, 1, Qt::DashLine));
    
    // 将场景坐标转换为视图坐标
    QRect viewRect(mapFromScene(m_multiSelectionRect.topLeft()), 
                  mapFromScene(m_multiSelectionRect.bottomRight()));
    
    painter->drawRect(viewRect);
    painter->restore();
}

// 复制多个选中的图形
void DrawingArea::copyMultiSelectedShapes()
{
    if (m_multiSelectedShapes.isEmpty() && m_multySelectedConnections.isEmpty()) {
        return;
    }
    
    // 清理之前复制的图形
    for (Shape* shape : m_copiedShapes) {
        delete shape;
    }
    m_copiedShapes.clear();
    m_copiedShapesPositions.clear();
    
    // 清理之前复制的连接线
    for (Connection* conn : m_copiedConnections) {
        delete conn;
    }
    m_copiedConnections.clear();
    m_copiedConnectionsPositions.clear();
    
    // 清除之前存储的连接关系
    m_copiedConnectionStartShapes.clear();
    m_copiedConnectionEndShapes.clear();
    m_copiedConnectionStartPoints.clear();
    m_copiedConnectionEndPoints.clear();
    
    // 计算多选图形的中心点
    QPoint centerPoint(0, 0);
    int totalElements = 0;
    
    // 计算所有图形的中心点总和
    for (Shape* shape : m_multiSelectedShapes) {
        centerPoint += shape->getRect().center();
        totalElements++;
    }
    
    // 加上所有选中连接线的中心点
    for (Connection* conn : m_multySelectedConnections) {
        QPoint startPos = conn->getStartPosition();
        QPoint endPos = conn->getEndPosition();
        centerPoint += (startPos + endPos) / 2;
        totalElements++;
    }
    
    // 计算总体中心点
    if (totalElements > 0) {
        centerPoint /= totalElements;
    }
    
    // 复制每个选中的图形
    for (Shape* shape : m_multiSelectedShapes) {
        // 使用工厂创建一个同类型的新图形
        QString type = shape->type();
        int basis = shape->getRect().width() / 2;
        Shape* copiedShape = ShapeFactory::instance().createShape(type, basis);
        
        if (copiedShape) {
            // 复制属性
            copiedShape->setRect(shape->getRect());
            copiedShape->setText(shape->text());
            copiedShape->setFillColor(shape->fillColor());
            copiedShape->setLineColor(shape->lineColor());
            copiedShape->setLineWidth(shape->lineWidth());
            copiedShape->setLineStyle(shape->lineStyle());
            copiedShape->setTransparency(shape->transparency());
            
            // 保存相对位置（相对于中心点）
            QPoint relativePos = shape->getRect().center() - centerPoint;
            
            // 添加到复制列表
            m_copiedShapes.append(copiedShape);
            m_copiedShapesPositions.append(relativePos);
        }
    }
    
    // 复制每个选中的连接线
    for (int i = 0; i < m_multySelectedConnections.size(); ++i) {
        Connection* conn = m_multySelectedConnections[i];
        QPoint startPos = conn->getStartPosition();
        QPoint endPos = conn->getEndPosition();
        
        // 创建一个新的连接线对象
        ArrowLine* copiedConnection = new ArrowLine(startPos, endPos);
        
        if (copiedConnection) {
            // 保存连接线的相对位置（中点相对于总中心点）
            QPoint connCenter = (startPos + endPos) / 2;
            QPoint relativePos = connCenter - centerPoint;
            
            // 添加到复制列表
            m_copiedConnections.append(copiedConnection);
            m_copiedConnectionsPositions.append(relativePos);
            
            // 保存连接关系
            int startShapeIndex = -1;
            int endShapeIndex = -1;
            ConnectionPoint::Position startPosition = ConnectionPoint::Free;
            ConnectionPoint::Position endPosition = ConnectionPoint::Free;
            
            // 检查起点是否连接到图形
            if (conn->getStartPoint() && conn->getStartPoint()->getOwner()) {
                Shape* startShape = conn->getStartPoint()->getOwner();
                // 查找起点图形在选中图形中的索引
                startShapeIndex = m_multiSelectedShapes.indexOf(startShape);
                // 记录连接点位置
                startPosition = conn->getStartPoint()->getPositionType();
            }
            
            // 检查终点是否连接到图形
            if (conn->getEndPoint() && conn->getEndPoint()->getOwner()) {
                Shape* endShape = conn->getEndPoint()->getOwner();
                // 查找终点图形在选中图形中的索引
                endShapeIndex = m_multiSelectedShapes.indexOf(endShape);
                // 记录连接点位置
                endPosition = conn->getEndPoint()->getPositionType();
            }
            
            // 存储连接关系
            m_copiedConnectionStartShapes.append(qMakePair(i, startShapeIndex));
            m_copiedConnectionEndShapes.append(qMakePair(i, endShapeIndex));
            m_copiedConnectionStartPoints.append(startPosition);
            m_copiedConnectionEndPoints.append(endPosition);
        }
    }
}

// 剪切多个选中的图形
void DrawingArea::cutMultiSelectedShapes()
{
    if (m_multiSelectedShapes.isEmpty()) {
        return;
    }
    
    // 先复制
    copyMultiSelectedShapes();
    
    // 删除选中的图形及相关的连接线
    QVector<Connection*> connectionsToRemove;
    
    // 收集所有需要删除的连接线
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
    
    // 删除连接线
    for (Connection* connection : connectionsToRemove) {
        m_connections.removeOne(connection);
        delete connection;
    }
    
    // 删除图形
    for (Shape* shape : m_multiSelectedShapes) {
        m_shapes.removeOne(shape);
        delete shape;
    }
    
    // 清除选择
    m_multiSelectedShapes.clear();
    m_selectedShape = nullptr;
    
    // 发送信号
    emit shapeSelectionChanged(false);
    emit multiSelectionChanged(false);
    emit shapesCountChanged(getShapesCount());
    
    update();
}



