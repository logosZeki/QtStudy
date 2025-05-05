#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTabBar>
#include <QToolBar>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>

class ToolBar;
class DrawingArea;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTabBarClicked(int index);

private:
    void createTopToolbar();
    void createMainToolbar();
    void createArrangeToolbar();
    void createExportToolbar();
    void setupUi();
    
private:
    ToolBar *m_toolBar;
    DrawingArea *m_drawingArea;
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_contentLayout;
    
    // 顶部工具栏
    QToolBar *m_topToolbar;
    QTabBar *m_tabBar;
    
    // 主工具栏 (对应"开始"选项卡)
    QToolBar *m_mainToolbar;
    
    // 排列工具栏 (对应"排列"选项卡)
    QToolBar *m_arrangeToolbar;
    
    // 导出工具栏 (对应"导出"选项卡)
    QToolBar *m_exportToolbar;
};

#endif // MAINWINDOW_H
