#include "drawingarea.h"
#include <QPainter>
#include <QMimeData>

DrawingArea::DrawingArea(QWidget *parent)
    : QWidget(parent), m_selectedShape(nullptr), m_dragging(false)
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
        
        // 如果是当前选中的形状，绘制一个选中框
        if (shape == m_selectedShape) {
            painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(shape->rect().adjusted(-2, -2, 2, 2));//扩大轮廓
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
        bool ok;
        int typeValue = event->mimeData()->text().toInt(&ok);
        
        if (ok) {
            ShapeType type = static_cast<ShapeType>(typeValue);
            
            
            // 根据类型创建形状对象
            Shape *newShape = nullptr;
            int basis = 55; // 基准值设为55

            switch (type) {
            case Rectangle:
                newShape = new RectangleShape(basis);
                break;
            case Circle:
                newShape = new CircleShape(basis);
                break;
            case Pentagon:
                newShape = new PentagonShape(basis);
                break;
            case Ellipse:
                newShape = new EllipseShape(basis);
                break;
            }
            
            // 设置形状位置
            if (newShape) {
                QRect shapeRect = newShape->rect();
                shapeRect.moveCenter(event->pos());
                newShape->setRect(shapeRect);
                m_selectedShape = newShape; // 设置为当前选中形状
                // 添加形状到列表
                m_shapes.append(newShape);
                update();
            }
        }
        
        event->acceptProposedAction();
    }
}


void DrawingArea::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
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
            m_shapeStart = m_selectedShape->rect().topLeft();
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
    if (m_dragging && m_selectedShape) {
        // 计算拖动偏移
        QPoint delta = event->pos() - m_dragStart;

        // 更新形状位置
        QRect newRect = m_selectedShape->rect();
        newRect.moveTo(m_shapeStart + delta);
        m_selectedShape->setRect(newRect);

        // 重绘
        update();
    }
}

void DrawingArea::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
    }
}
