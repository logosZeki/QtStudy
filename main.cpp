#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QDir>
#include <QDebug>
#include <QFile>
#include "chart/shape.h"
QString getQmlPath()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QDir currentDir(appDir);
    QString qmlFilePath = QDir::cleanPath(currentDir.absolutePath() + QDir::separator() +
        QString("mui") + QDir::separator() +
        QString("zh_CN") + QDir::separator() +
        QString("lessoncode.qm")
    );
    if (QFile::exists(qmlFilePath))
    {
        return qmlFilePath;
    }
    return QString();
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTranslator translator;
    if (translator.load(getQmlPath()))
    {
        a.installTranslator(&translator);
    }

    // // Set up translator
    // QTranslator translator;
    // // Load translations based on system locale
    // const QStringList uiLanguages = QLocale::system().uiLanguages();
    // for (const QString &locale : uiLanguages) {
    //     const QString baseName = "StepByStepFlowChart_" + QLocale(locale).name();
    //     if (translator.load(":/i18n/" + baseName)) {
    //         a.installTranslator(&translator);
    //         break;
    //     }
    // }

    MainWindow w;
    w.show();
    
    RectangleShape::registerShape();
    CircleShape::registerShape();
    PentagonShape::registerShape();
    EllipseShape::registerShape();
    RoundedRectangleShape::registerShape();
    DiamondShape::registerShape();
    HexagonShape::registerShape();
    OctagonShape::registerShape();
    CloudShape::registerShape();
    return a.exec();
}
