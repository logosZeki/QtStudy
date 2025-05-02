#include "mainwindow.h"
#include "toolbar.h"
#include "drawingarea.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 创建中央部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // 创建水平布局
    m_layout = new QHBoxLayout(m_centralWidget);

    // 创建工具栏和绘图区域
    m_toolBar = new ToolBar(this);
    m_drawingArea = new DrawingArea(this);

    // 设置工具栏固定宽度
    m_toolBar->setFixedWidth(150);

    // 添加到布局
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_drawingArea);

    // 设置窗口标题和大小
    setWindowTitle(tr("Simple flowchart Designer"));
    resize(800, 600);
}

MainWindow::~MainWindow()
{
}
