#ifndef FLOWCHARTVIEW_H
#define FLOWCHARTVIEW_H



#include <QGraphicsView>
#include <QKeyEvent>
#include <QWheelEvent> // 需要包含滚轮事件头文件
#include <QMimeData>   // 需要包含 MIME 数据头文件
#include <QDragEnterEvent>
#include <QDropEvent>

// 前向声明
class QGraphicsScene;
class QGraphicsItem;

class FlowchartGraphicsView : public QGraphicsView
{
    Q_OBJECT // 需要 Q_OBJECT 宏以支持信号和槽

public:
    FlowchartGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr);

protected:
    // 重写 keyPressEvent 处理删除操作 (来自之前的 MyGraphicsView)
    void keyPressEvent(QKeyEvent *event) override;

    // 重写滚轮事件处理缩放
    void wheelEvent(QWheelEvent *event) override;

    // 重写鼠标事件处理平移 (使用中键或特定模式)
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // 重写背景绘制事件添加网格
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    // 重写拖放事件以接收组件
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    bool isPanning;       // 标记是否正在平移
    QPoint lastPanPoint;  // 记录上一次平移鼠标位置
};

#endif // FLOWCHARTVIEW_H
