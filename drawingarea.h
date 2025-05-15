#ifndef DRAWINGAREA_H
#define DRAWINGAREA_H

#include <QWidget>
#include <QVector>
#include <QMouseEvent>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QScrollBar>


#include "chart/shape.h" //因为要用到Shape里的枚举
#include "util/Utils.h"

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
    // 缩放范围常量
    const qreal MAX_SCALE = Utils::MAX_SCALE;      // 最大缩放比例
    const qreal MIN_SCALE = Utils::MIN_SCALE;     // 最小缩放比例

    // 定义默认尺寸（像素）
    const int Default_WIDTH = Utils::Default_WIDTH; 
    const int Default_HEIGHT = Utils::Default_HEIGHT;
    
    DrawingArea(QWidget *parent = nullptr);
    ~DrawingArea();
    
    // 页面设置相关方法
    void setBackgroundColor(const QColor &color);
    void setPageSize(const QSize &size);
    void setDrawingAreaSize(const QSize &size);
    void setShowGrid(bool show);
    void setGridColor(const QColor &color);
    void setGridSize(int size);
    void setGridThickness(int thickness);
    
    // 获取当前页面设置
    QColor getBackgroundColor() const;
    QSize getPageSize() const;
    QSize getDrawingAreaSize() const { return m_drawingAreaSize; }
    bool getShowGrid() const;
    QColor getGridColor() const;
    int getGridSize() const;
    int getGridThickness() const;
    
    // 缩放相关方法
    void setScale(qreal scale);
    qreal getScale() const { return m_scale; }
    void zoomInOrOut(const qreal& factor);                    // 放大或缩小
    
    // 图形数量相关方法
    int getShapesCount() const;
    
    // 坐标转换方法
    //视图坐标系：用户在屏幕上看到和交互的坐标
    // 场景坐标系：实际存储图形和连线的物理坐标
    QPoint mapToScene(const QPoint& viewPoint) const;    // 将视图坐标转换为场景坐标
    QPoint mapFromScene(const QPoint& scenePoint) const; // 将场景坐标转换为视图坐标
    
    // 获取当前选中的图形
    Shape* getSelectedShape() const { return m_selectedShape; }

    // 图层管理相关方法
    void moveShapeUp();
    void moveShapeDown();
    void moveShapeToTop();
    void moveShapeToBottom();
    
    // 字体设置方法
    void setSelectedShapeFontFamily(const QString& family);
    void setSelectedShapeFontSize(int size);
    void setSelectedShapeFontBold(bool bold);
    void setSelectedShapeFontItalic(bool italic);
    void setSelectedShapeFontUnderline(bool underline);
    void setSelectedShapeFontColor(const QColor& color);
    void setSelectedShapeTextAlignment(Qt::Alignment alignment);
    
    // 图形颜色设置方法
    void setSelectedShapeFillColor(const QColor& color);
    void setSelectedShapeLineColor(const QColor& color);
    
    // 图形透明度、线条粗细和线条样式设置方法
    void setSelectedShapeTransparency(int transparency);
    void setSelectedShapeLineWidth(qreal width);
    void setSelectedShapeLineStyle(int style);
    
    // 图形位置和尺寸设置方法
    void setSelectedShapeX(int x);
    void setSelectedShapeY(int y);
    void setSelectedShapeWidth(int width);
    void setSelectedShapeHeight(int height);
    
    // 导出功能
    bool exportToPng(const QString &filePath);
    // SVG导出与导入功能
    bool exportToSvg(const QString &filePath);
    bool importFromSvg(const QString &filePath);
    
signals:
    // 图形选择状态改变的信号
    void selectionChanged();
    void shapeSelectionChanged(bool hasSelection);
    void fontColorChanged(const QColor& color);
    void fillColorChanged(const QColor& color);
    void lineColorChanged(const QColor& color);
    void scaleChanged(qreal scale);  // 新增的缩放比例变化信号
    void shapesCountChanged(int count);  // 新增的图形数量变化信号
    void multiSelectionChanged(bool hasMultiSelection); // 新增的多选状态变化信号
    void shapePositionChanged(const QPoint& topLeft);
    void shapeSizeChanged(const QSize& size);
    
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
    void wheelEvent(QWheelEvent *event) override;  // 处理鼠标滚轮事件
    
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

    // 尝试将连接线连接到最近的图形连接点
    void tryConnectLineToShapes(Connection* connection);

    // 绘制连接线拖动预览
    void drawConnectionPreview(QPainter* painter, Connection* connection);

    void updateCursor(QMouseEvent *event);
    
    // 右键菜单相关方法
    void createContextMenus();
    void showShapeContextMenu(const QPoint &pos);
    void showCanvasContextMenu(const QPoint &pos);
    

    
    // 剪切板操作相关方法
    void copySelectedShape();               // 复制选中的图形
    void cutSelectedShape();                // 剪切选中的图形或连接线
    void pasteShape(const QPoint &pos = QPoint());  // 粘贴图形
    void deleteSelectedShape();             // 删除选中的图形或连接线
    void selectAllShapes();                 // 全选图形
    Shape* cloneShape(const Shape* sourceShape);  // 克隆图形
    
    // 新增复制粘贴多选图形的方法
    void copyMultiSelectedShapes();         // 复制多个选中的图形
    void cutMultiSelectedShapes();          // 剪切多个选中的图形
    
    // ArrowLine相关方法
    void createArrowLine(const QPoint& startPoint, const QPoint& endPoint);
    void selectConnection(Connection* connection);
    
    // 绘制网格
    void drawGrid(QPainter *painter);
    
    // 居中显示绘图区域
    void centerDrawingArea();
    
    // 获取滚动视图的滚动条
    QScrollBar* findParentScrollBar(Qt::Orientation orientation = Qt::Horizontal) const;

    // 多选功能相关方法
    void startRectMultiSelection(const QPoint& point);
    void updateRectMultiSelection(const QPoint& point);
    void finishRectMultiSelection();
    void selectMultiShapesInRect(const QRect& rect);
    void clearMultySelection();
    void drawMultiSelectionRect(QPainter* painter);
    bool isShapeCompletelyInRect(Shape* shape, const QRect& rect) const;
    
private:
    QVector<Shape*> m_shapes;
    Shape* m_selectedShape;
    bool m_dragging;
    QPoint m_dragStart;
    QPoint m_shapeStart;
    
    // 缩放相关变量
    qreal m_scale;                    // 当前缩放比例
    QPoint m_lastMousePos;            // 上次鼠标位置
    bool m_isPanning;                 // 是否正在平移
    
    // 滚动偏移相关变量
    QPoint m_viewOffset;              // 视图偏移量
    
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
    QVector<Shape*> m_copiedShapes;      // 存储多个复制的图形
    QVector<QPoint> m_copiedShapesPositions; // 存储多个复制图形的相对位置
    QVector<Connection*> m_copiedConnections; // 存储复制的连接线
    QVector<QPoint> m_copiedConnectionsPositions; // 存储复制连接线的相对位置
    QVector<QPair<int, int>> m_copiedConnectionStartShapes; // 连接线起点与图形的绑定关系
    QVector<QPair<int, int>> m_copiedConnectionEndShapes; // 连接线终点与图形的绑定关系
    QVector<ConnectionPoint::Position> m_copiedConnectionStartPoints; // 连接线起点的连接点位置
    QVector<ConnectionPoint::Position> m_copiedConnectionEndPoints; // 连接线终点的连接点位置
    
    bool m_movingConnectionPoint;          // 是否正在移动连接线端点
    ConnectionPoint* m_activeConnectionPoint; // 当前活动的连接点
    QPoint m_connectionDragPoint;          // 拖动连接线端点时的临时位置
    
    // 页面设置相关变量
    QColor m_backgroundColor;              // 页面背景颜色
    QSize m_drawingAreaSize;               // 绘图区域尺寸，新增
    bool m_showGrid;                       // 是否显示网格
    QColor m_gridColor;                    // 网格颜色
    int m_gridSize;                        // 网格大小
    int m_gridThickness;                   // 网格线条粗细

    // 多选相关变量
    QVector<Shape*> m_multiSelectedShapes;      // 存储多选的图形
    bool m_isMultiRectSelecting;                // 是否正在框选
    QRect m_multiSelectionRect;                 // 框选矩形
    QPoint m_multiSelectionStart;               // 框选起点
    QVector<QPoint> m_multyShapesStartPos;      // 批量移动时记录每个图形的起始位置
    QVector<Connection*> m_multySelectedConnections; // 存储选中的连接线
};

#endif // DRAWINGAREA_H
