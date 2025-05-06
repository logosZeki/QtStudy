#ifndef DRAWINGAREA_H
#define DRAWINGAREA_H

#include <QWidget>
#include <QVector>
#include <QMouseEvent>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include "chart/shape.h" //因为要用到Shape里的枚举

// 添加前向声明
class Shape;
class Connection;
class ArrowLine;
class ConnectionPoint;
class CustomTextEdit;

class DrawingArea : public QWidget
{
    Q_OBJECT
    
public:
    DrawingArea(QWidget *parent = nullptr);
    ~DrawingArea();
    
    // 页面设置相关方法
    void setBackgroundColor(const QColor &color);
    void setPageSize(const QSize &size);
    void setShowGrid(bool show);
    void setGridColor(const QColor &color);
    void setGridSize(int size);
    void setGridThickness(int thickness);
    
    // 获取当前页面设置
    QColor getBackgroundColor() const;
    QSize getPageSize() const;
    bool getShowGrid() const;
    QColor getGridColor() const;
    int getGridSize() const;
    int getGridThickness() const;
    
public slots:
    // 应用页面设置
    void applyPageSettings();
    
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
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    
private:
    void createTextEditor();
    void startTextEditing();
    void finishTextEditing();
    void cancelTextEditing();
    
    // 流程图连线相关方法
    void startConnection(ConnectionPoint* startPoint);
    void completeConnection(ConnectionPoint* endPoint = nullptr);
    void cancelConnection();
    
    // 查找特定图形下最近的连接点
    ConnectionPoint* findNearestConnectionPoint(Shape* shape, const QPoint& pos);

    // 绘制连接线拖动预览
    void drawConnectionPreview(QPainter* painter, Connection* connection);

    void updateCursor(QMouseEvent *event);
    
    // 右键菜单相关方法
    void createContextMenus();
    void showShapeContextMenu(const QPoint &pos);
    void showCanvasContextMenu(const QPoint &pos);
    
    // 图层管理相关方法
    void moveShapeUp();
    void moveShapeDown();
    void moveShapeToTop();
    void moveShapeToBottom();
    
    // 剪切板操作相关方法
    void copySelectedShape();               // 复制选中的图形
    void cutSelectedShape();                // 剪切选中的图形或连接线
    void pasteShape(const QPoint &pos = QPoint());  // 粘贴图形
    void deleteSelectedShape();             // 删除选中的图形或连接线
    void selectAllShapes();                 // 全选图形
    Shape* cloneShape(const Shape* sourceShape);  // 克隆图形
    
    // ArrowLine相关方法
    void createArrowLine(const QPoint& startPoint, const QPoint& endPoint);
    void selectConnection(Connection* connection);
    
    // 绘制网格
    void drawGrid(QPainter *painter);
    
    // 检查并自动扩展绘图区域
    void checkAndExpandDrawingArea();
    
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
    Connection* m_selectedConnection;    // 当前选中的连线
    QPoint m_temporaryEndPoint;          // 临时端点位置
    
    // 右键菜单
    QMenu* m_shapeContextMenu;           // 图形右键菜单
    QMenu* m_canvasContextMenu;          // 画布右键菜单
    
    // 剪切板数据
    Shape* m_copiedShape;                // 复制的图形
    
    bool m_movingConnectionPoint;          // 是否正在移动连接线端点
    ConnectionPoint* m_activeConnectionPoint; // 当前活动的连接点
    QPoint m_connectionDragPoint;          // 拖动连接线端点时的临时位置
    
    // 页面设置相关变量
    QColor m_backgroundColor;              // 页面背景颜色
    QSize m_pageSize;                      // 页面尺寸
    bool m_showGrid;                       // 是否显示网格
    QColor m_gridColor;                    // 网格颜色
    int m_gridSize;                        // 网格大小
    int m_gridThickness;                   // 网格线条粗细
};

#endif // DRAWINGAREA_H
