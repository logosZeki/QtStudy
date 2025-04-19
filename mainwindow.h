#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// 前向声明
class QGraphicsScene;
class FlowchartGraphicsView;
class ComponentListWidget;
class QToolBar;
class QAction;
class QDockWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void saveFlowchart(); // 保存按钮的槽函数 (待实现)
    void loadFlowchart(); // 加载按钮的槽函数 (待实现)

private:
    void createActions();      // 创建动作 (Save/Load)
    void createToolbars();     // 创建工具栏
    void createDocks();        // 创建组件库停靠窗口
    void createCentralWidget();// 创建编辑区

    QGraphicsScene *scene;          // 图形场景
    FlowchartGraphicsView *view;    // 图形视图 (编辑区)
    ComponentListWidget *componentList; // 组件库列表
    QDockWidget *componentDock;     // 组件库停靠窗口

    QToolBar *fileToolBar;       // 文件工具栏
    QAction *saveAction;         // 保存动作
    QAction *loadAction;         // 加载动作
};
#endif // MAINWINDOW_H
