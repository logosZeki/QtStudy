#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QRect>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    // 重写绘制事件
    void paintEvent(QPaintEvent *event) override;
    // 重写鼠标按下事件
    void mousePressEvent(QMouseEvent *event) override;
    // 重写鼠标移动事件
    void mouseMoveEvent(QMouseEvent *event) override;
    // 重写鼠标释放事件
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QRect m_rect;          // 矩形
    bool m_dragging;       // 是否正在拖动
    QPoint m_dragStart;    // 拖动开始点
    QPoint m_rectStart;    // 矩形原始位置
};
#endif // MAINWINDOW_H
