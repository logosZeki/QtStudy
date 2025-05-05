#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPolygon>
#include <QTabBar>
#include <QLineEdit>
#include <QScrollArea>
#include <QPushButton>
#include <QGridLayout>
#include "chart/shape.h" // 包含shape.h获取ShapeTypes命名空间

// 工具栏上的形状项
class ShapeItem : public QWidget
{
    Q_OBJECT

public:
    ShapeItem(const QString& type, QWidget* parent = nullptr);

    QString type() const { return m_type; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QString m_type;
    QRect m_shapeRect;
};

// 形状类别组件
class ShapeCategory : public QWidget
{
    Q_OBJECT
    
public:
    ShapeCategory(const QString& title, QWidget* parent = nullptr);
    void addShape(const QString& type);
    
private:
    QString m_title;
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QGridLayout* m_shapesLayout;
    int m_currentRow;
    int m_currentColumn;
};

// 工具栏
class ToolBar : public QWidget
{
    Q_OBJECT

public:
    ToolBar(QWidget* parent = nullptr);

private slots:
    void onTabChanged(int index);
    
private:
    void setupUI();
    void createGraphicsLibTab();
    void createStylesTab();
    
private:
    QVBoxLayout* m_layout;
    QTabBar* m_tabBar;
    QLineEdit* m_searchBox;
    QWidget* m_libraryWidget;
    QWidget* m_stylesWidget;
    QScrollArea* m_scrollArea;
};

#endif // TOOLBAR_H
