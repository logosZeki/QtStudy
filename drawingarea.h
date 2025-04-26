#ifndef DRAWINGAREA_H
#define DRAWINGAREA_H

#include <QWidget>
#include <QList>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include "shape.h"

class DrawingArea : public QWidget
{
    Q_OBJECT

public:
    DrawingArea(QWidget *parent = nullptr);
    ~DrawingArea();

protected:
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QList<Shape*> m_shapes;
    Shape* m_selectedShape;
    bool m_dragging;
    QPoint m_dragStart;
    QPoint m_shapeStart;
};

#endif // DRAWINGAREA_H
