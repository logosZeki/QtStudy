#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    // 设置窗口大小
    w.resize(800, 600);

    // 设置窗口标题
    w.setWindowTitle("可拖动矩形示例");
    w.show();
    return a.exec();
}

