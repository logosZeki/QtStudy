#ifndef COMPONENTLISTWIDGET_H
#define COMPONENTLISTWIDGET_H

#include <QListWidget>
#include <QDrag>       // 需要包含拖动操作头文件
#include <QMimeData>
#include <QMouseEvent>

const QString FLOWCHART_ITEM_MIME_TYPE = "application/x-flowchart-item";
class ComponentListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit ComponentListWidget(QWidget *parent = nullptr);
    void addComponent(const QString &name, const QString &typeId, const QIcon &icon = QIcon());

protected:
    // 重写鼠标按下事件以准备拖动
    void mousePressEvent(QMouseEvent *event) override;
    // 重写鼠标移动事件以启动拖动
    void mouseMoveEvent(QMouseEvent *event) override;
    // (可选) 重写 startDrag 自定义拖动行为
    // void startDrag(Qt::DropActions supportedActions) override;

private:
    QPoint startDragPos; // 记录拖动开始时的鼠标位置
};

#endif // COMPONENTLISTWIDGET_H
