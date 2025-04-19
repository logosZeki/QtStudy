#include "mainwindow.h"
#include "flowchartview.h"
#include "componentlistwidget.h" // 包含组件列表头文件

#include <QGraphicsScene>
#include <QToolBar>
#include <QAction>
#include <QDockWidget>
#include <QMessageBox> // 用于占位符消息
#include <QIcon>       // 用于动作图标
#include <QStyle>      // 用于获取标准图标

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    createCentralWidget(); // 1. 创建编辑区 (场景和视图)
    createActions();       // 2. 创建动作
    createToolbars();      // 3. 创建工具栏并添加动作
    createDocks();         // 4. 创建组件库停靠窗口

    setWindowTitle(tr("流程图设计器")); // 设置窗口标题
    resize(1024, 768); // 设置默认窗口大小
}

MainWindow::~MainWindow()
{
    // scene 会被 view 管理 (如果 view 先析构) 或者需要手动 delete
    // Qt 的父子对象关系通常会自动处理内存，但显式 delete scene 也可以
    // delete scene; // 如果没有父对象或者需要确保删除顺序
}

void MainWindow::createCentralWidget()
{
    scene = new QGraphicsScene(this); // 创建场景，指定父对象为 MainWindow
    scene->setSceneRect(-2000, -1500, 4000, 3000); // 设置一个较大的场景范围

    view = new FlowchartGraphicsView(scene, this); // 创建视图，关联场景
    setCentralWidget(view); // 将视图设置为主窗口的中心部件
}

void MainWindow::createActions()
{
    // 获取标准图标
    const QIcon saveIcon = QIcon::fromTheme("document-save", style()->standardIcon(QStyle::SP_DialogSaveButton));
    const QIcon loadIcon = QIcon::fromTheme("document-open", style()->standardIcon(QStyle::SP_DialogOpenButton));

    saveAction = new QAction(saveIcon, tr("保存 (&S)"), this);
    saveAction->setShortcut(QKeySequence::Save); // 设置快捷键 Ctrl+S
    saveAction->setStatusTip(tr("保存流程图到文件")); // 设置状态栏提示
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFlowchart);

    loadAction = new QAction(loadIcon, tr("加载 (&L)"), this);
    loadAction->setShortcut(QKeySequence::Open); // 设置快捷键 Ctrl+O
    loadAction->setStatusTip(tr("从文件加载流程图"));
    connect(loadAction, &QAction::triggered, this, &MainWindow::loadFlowchart);
}

void MainWindow::createToolbars()
{
    fileToolBar = addToolBar(tr("文件")); // 创建文件工具栏
    fileToolBar->addAction(loadAction);  // 添加加载动作
    fileToolBar->addAction(saveAction);  // 添加保存动作
}

void MainWindow::createDocks()
{
    componentDock = new QDockWidget(tr("组件库"), this); // 创建停靠窗口
    componentDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea); // 允许左右停靠

    componentList = new ComponentListWidget(componentDock); // 创建组件列表作为停靠窗口的内容
    // 添加示例组件
    componentList->addComponent(tr("矩形框"), "rectangle", QIcon(":/icons/rectangle.png")); // 假设有资源文件
    componentList->addComponent(tr("椭圆框"), "ellipse", QIcon(":/icons/ellipse.png"));
    // componentList->addComponent(tr("菱形框"), "diamond", QIcon(":/icons/diamond.png"));

    componentDock->setWidget(componentList); // 将列表设置给停靠窗口
    addDockWidget(Qt::LeftDockWidgetArea, componentDock); // 将停靠窗口添加到主窗口左侧
}

// --- 槽函数占位符 ---

void MainWindow::saveFlowchart()
{
    QMessageBox::information(this, tr("保存"), tr("保存功能待实现。"));
    // 这里将来需要实现文件对话框选择路径，并将 scene 内容序列化保存
}

void MainWindow::loadFlowchart()
{
     QMessageBox::information(this, tr("加载"), tr("加载功能待实现。"));
    // 这里将来需要实现文件对话框选择文件，清空 scene，然后反序列化加载内容
}
