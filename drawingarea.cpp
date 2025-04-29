#include "drawingarea.h"
#include <QPainter>
#include <QMimeData>
#include "shape/shapefactory.h"

DrawingArea::DrawingArea(QWidget *parent)
    : QWidget(parent), m_selectedShape(nullptr), m_dragging(false),
      m_activeHandle(Shape::None), m_resizing(false)
{
    // 设置接受拖放
    setAcceptDrops(true);

    // 设置背景色为白色
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);
}

DrawingArea::~DrawingArea()
{
    // 清理所有形状
    qDeleteAll(m_shapes);
    m_shapes.clear();
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
            painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(shape->getRect().adjusted(-2, -2, 2, 2));//扩大轮廓
            
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


void DrawingArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 如果已选中形状，检查是否点击了调整手柄
        if (m_selectedShape) {
            Shape::HandlePosition handle = m_selectedShape->hitHandle(event->pos());
            if (handle != Shape::None) {
                m_activeHandle = handle;
                m_resizing = true;
                m_dragStart = event->pos();
                setCursor(Qt::SizeAllCursor); // 设置调整大小的光标
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
    }
}

void DrawingArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 结束调整大小
        if (m_resizing) {
            m_resizing = false;
            m_activeHandle = Shape::None;
            setCursor(Qt::ArrowCursor);
        }
        
        // 结束拖动
        if (m_dragging) {
            m_dragging = false;
            setCursor(Qt::ArrowCursor);
        }
    }
}
