#include "drawingarea.h"
#include "util/command.h"
#include "util/shapecommands.h"

// 实现撤销功能
bool DrawingArea::canUndo() const
{
    return CommandManager::instance()->canUndo();
}

void DrawingArea::undo()
{
    if (canUndo()) {
        CommandManager::instance()->undo();
        update();
        emit undoAvailable(canUndo());
        emit redoAvailable(canRedo());
    }
}

// 实现重做功能
bool DrawingArea::canRedo() const
{
    return CommandManager::instance()->canRedo();
}

void DrawingArea::redo()
{
    if (canRedo()) {
        CommandManager::instance()->redo();
        update();
        emit undoAvailable(canUndo());
        emit redoAvailable(canRedo());
    }
}

// 实现添加形状方法（供命令类调用）
void DrawingArea::addShape(Shape* shape)
{
    if (shape) {
        m_shapes.append(shape);
        emit shapesCountChanged(m_shapes.size());
        update();
    }
}

// 实现删除形状方法（供命令类调用）
void DrawingArea::removeShape(Shape* shape)
{
    if (shape) {
        // 处理选中状态
        if (shape == m_selectedShape) {
            m_selectedShape = nullptr;
            emit shapeSelectionChanged(false);
        }
        
        // 从多选列表中移除
        if (m_multiSelectedShapes.contains(shape)) {
            m_multiSelectedShapes.removeOne(shape);
            if (m_multiSelectedShapes.isEmpty()) {
                emit multiSelectionChanged(false);
            }
        }
        
        // 从形状列表中移除
        m_shapes.removeOne(shape);
        emit shapesCountChanged(m_shapes.size());
        update();
    }
} 