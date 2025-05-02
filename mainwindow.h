#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>

class ToolBar;
class DrawingArea;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    ToolBar *m_toolBar;
    DrawingArea *m_drawingArea;
    QWidget *m_centralWidget;
    QHBoxLayout *m_layout;
};

#endif // MAINWINDOW_H
