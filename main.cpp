#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // Register all shapes with factory
    RectangleShape::registerShape();
    CircleShape::registerShape();
    PentagonShape::registerShape();
    EllipseShape::registerShape();

    return a.exec();
}
