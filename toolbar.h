#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPolygon>


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

// 工具栏
class ToolBar : public QWidget
{
    Q_OBJECT

public:
    ToolBar(QWidget* parent = nullptr);

private:
    QVBoxLayout* m_layout;
    QLabel* m_titleLabel;
};

#endif // TOOLBAR_H
