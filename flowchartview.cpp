#include "flowchartview.h"
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem> // 如果需要其他形状
#include <QDebug>
#include <QApplication> // for QApplication::keyboardModifiers()
#include <QScrollBar>   // for scrollbar panning
#include <QPainter>     // for drawBackground
#include <cmath>

// 定义拖放时使用的 MIME 类型
const QString FLOWCHART_ITEM_MIME_TYPE = "application/x-flowchart-item";

FlowchartGraphicsView::FlowchartGraphicsView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent), isPanning(false)
{
    setRenderHint(QPainter::Antialiasing); // 抗锯齿
    setDragMode(QGraphicsView::RubberBandDrag); // 默认启用框选

    // 启用对拖放操作的接收
    setAcceptDrops(true);

    // 更好的平移方式: 按住空格键 + 鼠标左键拖动
    // 或者使用中键 setDragMode(QGraphicsView::ScrollHandDrag);
}

// --- 事件处理 ---

void FlowchartGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Backspace)
    {
        QGraphicsScene *currentScene = scene();
        if (!currentScene) return;

        QList<QGraphicsItem *> selected = currentScene->selectedItems();
        for (QGraphicsItem *item : selected) {
            // 这里假设所有可删除的项都是我们添加的流程框
            // 可以增加更严格的类型检查，比如检查自定义类型ID
            currentScene->removeItem(item);
            delete item;
        }
    } else if (event->key() == Qt::Key_Space) {
         // 按下空格键时，临时切换到抓手模式用于平移
         setDragMode(QGraphicsView::ScrollHandDrag);
         QGraphicsView::keyPressEvent(event); // 确保基类也能处理
    }
    else {
        QGraphicsView::keyPressEvent(event); // 其他按键交给基类处理
    }
}

// 在 keyReleaseEvent 中恢复之前的 DragMode (如果需要的话)
// void FlowchartGraphicsView::keyReleaseEvent(QKeyEvent *event)
// {
//     if (event->key() == Qt::Key_Space) {
//         setDragMode(QGraphicsView::RubberBandDrag); // 恢复框选模式
//     }
//     QGraphicsView::keyReleaseEvent(event);
// }


void FlowchartGraphicsView::wheelEvent(QWheelEvent *event)
{
    // 以鼠标指针位置为中心进行缩放
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    double scaleFactor = 1.15; // 缩放因子
    if (event->angleDelta().y() > 0) {
        // 向上滚动，放大
        scale(scaleFactor, scaleFactor);
    } else {
        // 向下滚动，缩小
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
    // 重置锚点为默认值（视图中心）
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
}

void FlowchartGraphicsView::mousePressEvent(QMouseEvent *event)
{
    // 使用中键平移的简单实现
    if (event->button() == Qt::MiddleButton) {
        isPanning = true;
        lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor); // 设置拖动鼠标样式
        event->accept();
        return;
    }
    // 如果启用了 ScrollHandDrag 模式，基类会处理左键拖动
    QGraphicsView::mousePressEvent(event);
}

void FlowchartGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (isPanning) {
        // 计算鼠标移动的距离
        QPoint delta = event->pos() - lastPanPoint;
        lastPanPoint = event->pos();

        // 移动滚动条来实现平移
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void FlowchartGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        isPanning = false;
        setCursor(Qt::ArrowCursor); // 恢复默认鼠标样式
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

// 绘制网格背景
void FlowchartGraphicsView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect); // 先绘制默认背景 (如果需要)

    // 设置网格画笔
    QPen pen(Qt::lightGray, 0); // 细线
    pen.setStyle(Qt::DotLine); // 点线样式
    painter->setPen(pen);

    // 获取视图当前的变换矩阵的逆矩阵，用于将视图坐标转换为场景坐标
    QTransform viewTransform = transform().inverted();
    // 计算在视图坐标 rect 内对应的场景坐标范围
    QRectF sceneRect = viewTransform.mapRect(rect);

    // 网格间距 (场景坐标系)
    qreal gridSize = 20.0;

    // 计算需要绘制的网格线的起始和结束位置 (场景坐标)
    qreal left = floor(sceneRect.left() / gridSize) * gridSize;
    qreal top = floor(sceneRect.top() / gridSize) * gridSize;
    qreal right = ceil(sceneRect.right() / gridSize) * gridSize;
    qreal bottom = ceil(sceneRect.bottom() / gridSize) * gridSize;

    // 绘制垂直线
    for (qreal x = left; x <= right; x += gridSize) {
        painter->drawLine(QPointF(x, top), QPointF(x, bottom));
    }

    // 绘制水平线
    for (qreal y = top; y <= bottom; y += gridSize) {
        painter->drawLine(QPointF(left, y), QPointF(right, y));
    }
}

// --- 拖放处理 ---

void FlowchartGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    // 检查拖入的数据是否包含我们定义的 MIME 类型
    if (event->mimeData()->hasFormat(FLOWCHART_ITEM_MIME_TYPE)) {
        event->acceptProposedAction(); // 接受拖入操作
    } else {
        event->ignore(); // 忽略不相关的拖放
    }
}

void FlowchartGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    // 可以在这里根据鼠标位置决定是否接受，但通常直接接受
    if (event->mimeData()->hasFormat(FLOWCHART_ITEM_MIME_TYPE)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void FlowchartGraphicsView::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(FLOWCHART_ITEM_MIME_TYPE)) {
        // 获取拖放数据中包含的组件类型信息
        QByteArray itemData = event->mimeData()->data(FLOWCHART_ITEM_MIME_TYPE);
        QString itemType = QString::fromUtf8(itemData); // 将数据转回字符串

        // 将放置点的视图坐标转换为场景坐标
        QPointF scenePos = mapToScene(event->pos());

        // 根据组件类型创建相应的图形项
        QGraphicsItem *newItem = nullptr;
        if (itemType == "rectangle") {
            QGraphicsRectItem *rectItem = new QGraphicsRectItem(0, 0, 100, 60); // 默认大小
            rectItem->setBrush(Qt::yellow); // 给个默认颜色
            newItem = rectItem;
        } else if (itemType == "ellipse") {
            QGraphicsEllipseItem *ellipseItem = new QGraphicsEllipseItem(0, 0, 100, 60);
            ellipseItem->setBrush(Qt::cyan);
            newItem = ellipseItem;
        }
        // ... 可以添加更多类型，如菱形 (需要 QGraphicsPolygonItem)

        if (newItem) {
            newItem->setPos(scenePos); // 设置在场景中的位置
            // 设置标志，使其可交互
            newItem->setFlag(QGraphicsItem::ItemIsMovable);
            newItem->setFlag(QGraphicsItem::ItemIsSelectable);
            newItem->setFlag(QGraphicsItem::ItemIsFocusable);
            //newItem->setPen(QPen(Qt::black, 1)); // 默认边框

            scene()->addItem(newItem); // 添加到场景中
            event->acceptProposedAction();
        } else {
             event->ignore();
        }

    } else {
        event->ignore();
    }
}
