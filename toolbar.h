#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include "shape.h"

// 工具栏上的形状项
class ShapeItem : public QWidget
{
    Q_OBJECT

public:
    ShapeItem(ShapeType type, const QString &name, QWidget *parent = nullptr);

    ShapeType type() const { return m_type; }

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    ShapeType m_type;
    QString m_name;
    QRect m_shapeRect;
};

// 工具栏
class ToolBar : public QWidget
{
    Q_OBJECT

public:
    ToolBar(QWidget *parent = nullptr);

private:
    QVBoxLayout *m_layout;
    QLabel *m_titleLabel;
};

#endif // TOOLBAR_H
