#include "drawingarea.h"
#include <QPainter>
#include <QMimeData>
#include "shape/shapefactory.h"
#include <QKeyEvent>
#include "customtextedit.h"

DrawingArea::DrawingArea(QWidget *parent)
    : QWidget(parent), m_selectedShape(nullptr), m_dragging(false),
      m_activeHandle(Shape::None), m_resizing(false), m_textEditor(nullptr)
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
    
    if (m_textEditor) {
        delete m_textEditor;
    }
}

void DrawingArea::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制所有形状
    for (Shape *shape : m_shapes) {
        shape->paint(&painter);
        
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

    // 更新鼠标光标样式
    
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
    }
    
    // 检查鼠标是否位于任何形状上方
    for (int i = m_shapes.size() - 1; i >= 0; --i) {
        if (m_shapes[i]->contains(event->pos())) {
            setCursor(Qt::SizeAllCursor); // 四向箭头
            return;
        }
    }
    
    // 如果鼠标不在任何形状或手柄上，设置为标准光标
    setCursor(Qt::ArrowCursor);
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
        }
        
        // 查找点击的是哪个形状
        Shape *clickedShape = nullptr;

        // 从后向前遍历，因为后添加的形状显示在上层
        for (int i = m_shapes.size() - 1; i >= 0; --i) {
            if (m_shapes[i]->contains(event->pos())) {
                clickedShape = m_shapes[i];
                break;
            }
        }

        if (clickedShape) {
            // 选中形状，并准备拖动
            m_selectedShape = clickedShape;
            m_dragging = true;
            m_dragStart = event->pos();
            m_shapeStart = m_selectedShape->getRect().topLeft();
            setCursor(Qt::ClosedHandCursor);
        } else {
            // 点击空白区域，取消选择
            m_selectedShape = nullptr;
        }
        
        // 重绘以更新选中状态
        update();
    }
}

void DrawingArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 结束调整大小
        if (m_resizing) {
            m_resizing = false;
            m_activeHandle = Shape::None;
            // 光标会在下一次mouseMoveEvent中重新设置
        }
        
        // 结束拖动
        if (m_dragging) {
            m_dragging = false;
            // 光标会在下一次mouseMoveEvent中重新设置
        }
        
        // 触发mouseMoveEvent来更新光标
        QMouseEvent fakeEvent(QEvent::MouseMove, event->pos(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
        mouseMoveEvent(&fakeEvent);
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
