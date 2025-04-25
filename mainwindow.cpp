#include "mainwindow.h"



MainWindow::MainWindow(QWidget *parent) : QWidget(parent),
    m_dragging(false)
{
    // 设置窗口背景为白色
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);

    // 设置初始矩形位置
    m_rect = QRect(100, 100, 200, 150);

    // 允许鼠标追踪，以便实现平滑的拖动效果
    setMouseTracking(true);//启用鼠标跟踪，即使没有按下任何按钮，wideget也会接收鼠标移动事件。默认情况不启用鼠标跟踪，按下按钮才会接收鼠标移动事件
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // 创建绘图器
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 设置矩形样式与颜色
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(QColor(1, 162, 232, 128)));

    // 绘制矩形
    painter.drawRect(m_rect);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // 检查鼠标点击是否在矩形内
    if (event->button() == Qt::LeftButton && m_rect.contains(event->pos()))
    {
        m_dragging = true;
        m_dragStart = event->pos();
        m_rectStart = m_rect.topLeft();
        setCursor(Qt::ClosedHandCursor);//把鼠标光标变为一个握紧的手的图标
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    // 如果正在拖动，更新矩形位置
    if (m_dragging)
    {
        QPoint delta = event->pos() - m_dragStart;
        m_rect.moveTo(m_rectStart + delta);

        // 重绘窗口以更新矩形位置
        update();
    }
    else if (m_rect.contains(event->pos()))
    {
        // 当鼠标悬停在矩形上时，改变光标形状
        setCursor(Qt::OpenHandCursor);//把鼠标光标变为一个张开的手的图标
    }
    else
    {
        // 鼠标不在矩形上时，使用默认光标
        setCursor(Qt::ArrowCursor);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    // 如果释放左键，结束拖动状态
    if (event->button() == Qt::LeftButton && m_dragging)
    {
        m_dragging = false;
        if (m_rect.contains(event->pos()))
        {
            setCursor(Qt::OpenHandCursor);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
        }
    }
}
