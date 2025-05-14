#include "drawingarea.h"
#include "util/command.h"
#include "util/shapecommands.h"

// 这个文件是用来帮助开发者理解如何将命令模式集成到现有代码中的
// 以下是一些需要修改的关键方法的示例代码

/* 
修改mouseReleaseEvent方法示例:

void DrawingArea::mouseReleaseEvent(QMouseEvent *event)
{
    // ... 现有代码 ...
    
    // 结束调整大小操作
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
    
    // 结束拖动操作
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
    
    // ... 其余现有代码 ...
}
*/

/*
修改mousePressEvent方法示例:

void DrawingArea::mousePressEvent(QMouseEvent *event)
{
    // ... 现有代码 ...
    
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
        
        for (int i = m_shapes.size() - 1; i >= 0; --i) {
            if (m_shapes[i]->contains(scenePos)) {
                // 记录起始位置，用于创建移动命令
                if (m_shapes[i] == m_selectedShape) {
                    m_shapeStart = m_selectedShape->getRect().topLeft();
                }
                // ... 其余现有代码 ...
            }
        }
    }
    
    // ... 其余现有代码 ...
}
*/

/*
修改finishTextEditing方法示例:

void DrawingArea::finishTextEditing()
{
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
    
    // 结束编辑状态
    m_selectedShape->setEditing(false);
    m_textEditor->hide();
    
    update();
}
*/

/*
修改删除形状方法示例:

void DrawingArea::deleteSelectedShape()
{
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
        // ... 现有代码 ...
    }
}
*/

/*
在各种属性修改方法中添加命令模式示例:

void DrawingArea::setSelectedShapeFillColor(const QColor& color)
{
    if (m_selectedShape) {
        QColor oldColor = m_selectedShape->fillColor();
        
        // 如果颜色有变化，创建样式修改命令
        if (oldColor != color) {
            Command* styleCmd = new ChangeStyleCommand(
                this,
                m_selectedShape,
                ChangeStyleCommand::FillColor,
                oldColor,
                color
            );
            CommandManager::instance()->executeCommand(styleCmd);
            
            // 更新撤销/重做状态
            emit undoAvailable(canUndo());
            emit redoAvailable(canRedo());
        }
        
        emit fillColorChanged(color);
    }
}
*/ 