#include "drawingarea.h"
#include "util/command.h"
#include "util/shapecommands.h"

// 以下代码需要添加到原有的mouseReleaseEvent方法中
// 在鼠标释放事件的方法中添加命令模式支持
void DrawingArea::mouseReleaseEvent_CommandSupport(QMouseEvent *event)
{
    // 以下代码添加到mouseReleaseEvent方法中的适当位置

    // 结束调整大小操作，创建调整大小命令
    if (m_resizing && event->button() == Qt::LeftButton && m_selectedShape) {
        // 创建调整大小命令
        Command* resizeCmd = new ResizeShapeCommand(
            this, 
            m_selectedShape, 
            m_resizeStartRect, 
            m_selectedShape->getRect()
        );
        CommandManager::instance()->executeCommand(resizeCmd);
        
        m_resizing = false;
        m_activeHandle = Shape::None;
        setCursor(Qt::ArrowCursor);
        
        // 更新撤销/重做状态
        emit undoAvailable(canUndo());
        emit redoAvailable(canRedo());
        update();
        return;
    }
    
    // 结束拖动操作，创建移动命令
    if (m_dragging && event->button() == Qt::LeftButton) {
        if (m_selectedShape) {
            // 创建移动命令
            Command* moveCmd = new MoveShapeCommand(
                this, 
                m_selectedShape, 
                m_shapeStart, 
                m_selectedShape->getRect().topLeft()
            );
            CommandManager::instance()->executeCommand(moveCmd);
        } 
        else if (!m_multiSelectedShapes.isEmpty()) {
            // 创建批量移动命令
            BatchMoveCommand* batchMoveCmd = new BatchMoveCommand(
                this,
                m_multiSelectedShapes,
                m_multyShapesStartPos,
                QVector<QPoint>() // 当前位置将在命令中获取
            );
            
            // 获取当前位置
            QVector<QPoint> currentPositions;
            for (Shape* shape : m_multiSelectedShapes) {
                currentPositions.append(shape->getRect().topLeft());
            }
            
            // 更新命令中的当前位置
            batchMoveCmd->setNewPositions(currentPositions);
            
            // 执行命令
            CommandManager::instance()->executeCommand(batchMoveCmd);
        }
        
        // 更新撤销/重做状态
        emit undoAvailable(canUndo());
        emit redoAvailable(canRedo());
        
        m_dragging = false;
        m_multyShapesStartPos.clear();
        setCursor(Qt::ArrowCursor);
        update();
        return;
    }
}

// 添加到鼠标按下事件中
void DrawingArea::mousePressEvent_CommandSupport(QMouseEvent *event)
{
    // 以下代码添加到mousePressEvent方法中的适当位置
    
    // 如果将要调整形状大小，记录初始矩形
    if (m_selectedShape) {
        Shape::HandlePosition handle = m_selectedShape->hitHandle(mapToScene(event->pos()));
        if (handle != Shape::None) {
            // 记录调整大小前的矩形，用于撤销
            m_resizeStartRect = m_selectedShape->getRect();
        }
    }
    
    // 如果将要拖动形状，记录初始位置
    if (event->button() == Qt::LeftButton) {
        QPoint scenePos = mapToScene(event->pos());
        
        // 记录用于拖动的起始位置
        if (m_selectedShape && m_selectedShape->contains(scenePos)) {
            m_moveStartPos = m_selectedShape->getRect().topLeft();
        }
    }
}

// 添加到finishTextEditing方法中
void DrawingArea::finishTextEditing_CommandSupport()
{
    // 以下代码添加到finishTextEditing方法中的适当位置
    
    // 确保有选中的形状和文本编辑器
    if (!m_textEditor || !m_selectedShape) return;
    
    // 获取原始文本和新文本
    QString oldText = m_selectedShape->text();
    QString newText = m_textEditor->toPlainText();
    
    // 如果文本有变化，创建文本修改命令
    if (oldText != newText) {
        Command* textChangeCmd = new ChangeTextCommand(
            this,
            m_selectedShape,
            oldText,
            newText
        );
        CommandManager::instance()->executeCommand(textChangeCmd);
        
        // 更新撤销/重做状态
        emit undoAvailable(canUndo());
        emit redoAvailable(canRedo());
    }
}

// 修改deleteSelectedShape方法
void DrawingArea::deleteSelectedShape_CommandSupport()
{
    // 以下代码替换原有的deleteSelectedShape方法
    
    // 删除选中的形状
    if (m_selectedShape) {
        // 创建删除形状命令
        Command* deleteCmd = new DeleteShapeCommand(this, m_selectedShape);
        CommandManager::instance()->executeCommand(deleteCmd);
        
        // 清除选中状态（命令中会移除形状）
        m_selectedShape = nullptr;
        
        // 更新撤销/重做状态
        emit undoAvailable(canUndo());
        emit redoAvailable(canRedo());
        emit shapeSelectionChanged(false);
        update();
    }
    // 删除选中的连接线
    else if (m_selectedConnection) {
        // 这里需要添加删除连接线的命令
        // ...
        
        // 临时实现：直接删除
        m_connections.removeOne(m_selectedConnection);
        delete m_selectedConnection;
        m_selectedConnection = nullptr;
        
        update();
    }
}

// 为BatchMoveCommand类添加设置新位置的方法
// 需要添加到shapecommands.h中
void BatchMoveCommand::setNewPositions(const QVector<QPoint>& newPositions)
{
    m_newPositions = newPositions;
} 