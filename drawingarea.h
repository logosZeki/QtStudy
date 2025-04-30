#ifndef DRAWINGAREA_H
#define DRAWINGAREA_H

#include <QWidget>
#include <QVector>
#include <QMouseEvent>
#include <QLineEdit>
#include "shape/shape.h"
#include "customtextedit.h"

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
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    
private:
    void createTextEditor();
    void startTextEditing();
    void finishTextEditing();
    void cancelTextEditing();
    
private:
    QVector<Shape*> m_shapes;
    Shape* m_selectedShape;
    bool m_dragging;
    QPoint m_dragStart;
    QPoint m_shapeStart;
    
    // 调整大小相关变量
    Shape::HandlePosition m_activeHandle;
    bool m_resizing;
    
    // 文本编辑相关
    CustomTextEdit* m_textEditor;
};

#endif // DRAWINGAREA_H
