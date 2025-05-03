#ifndef DRAWINGAREA_H
#define DRAWINGAREA_H

#include <QWidget>
#include <QVector>
#include <QMouseEvent>
#include <QLineEdit>
#include "chart/shape.h" //因为要用到Shape里的枚举

// 添加前向声明
class Shape;
class Connection;
class ConnectionPoint;
class CustomTextEdit;

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
    
    // 流程图连线相关方法
    void startConnection(ConnectionPoint* startPoint);
    void completeConnection(ConnectionPoint* endPoint);
    void cancelConnection();
    
    // 查找特定图形下最近的连接点
    ConnectionPoint* findNearestConnectionPoint(Shape* shape, const QPoint& pos);

    void updateCursor(QMouseEvent *event);
    
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
    
    // 连线相关变量
    QVector<Connection*> m_connections;  // 所有连线
    Connection* m_currentConnection;     // 正在创建的连线
    Shape* m_hoveredShape;               // 鼠标悬停的形状
};

#endif // DRAWINGAREA_H
