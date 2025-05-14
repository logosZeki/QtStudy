#ifndef SHAPECOMMANDS_H
#define SHAPECOMMANDS_H

#include "command.h"
#include "../chart/shape.h"
#include "../drawingarea.h"
#include <QVector>

// 添加形状命令
class AddShapeCommand : public Command {
public:
    AddShapeCommand(DrawingArea* drawingArea, Shape* shape);
    ~AddShapeCommand() override;
    
    void execute() override;
    void undo() override;
    QString description() const override;
    
private:
    DrawingArea* m_drawingArea;
    Shape* m_shape;
    bool m_shapeOwned;  // 是否拥有形状的所有权
};

// 删除形状命令
class DeleteShapeCommand : public Command {
public:
    DeleteShapeCommand(DrawingArea* drawingArea, Shape* shape);
    ~DeleteShapeCommand() override;
    
    void execute() override;
    void undo() override;
    QString description() const override;
    
private:
    DrawingArea* m_drawingArea;
    Shape* m_shape;
    bool m_executed;  // 是否已执行删除
};

// 移动形状命令
class MoveShapeCommand : public Command {
public:
    MoveShapeCommand(DrawingArea* drawingArea, Shape* shape, 
                    const QPoint& oldPos, const QPoint& newPos);
    
    void execute() override;
    void undo() override;
    QString description() const override;
    
private:
    DrawingArea* m_drawingArea;
    Shape* m_shape;
    QPoint m_oldPos;
    QPoint m_newPos;
};

// 调整形状大小命令
class ResizeShapeCommand : public Command {
public:
    ResizeShapeCommand(DrawingArea* drawingArea, Shape* shape, 
                      const QRect& oldRect, const QRect& newRect);
    
    void execute() override;
    void undo() override;
    QString description() const override;
    
private:
    DrawingArea* m_drawingArea;
    Shape* m_shape;
    QRect m_oldRect;
    QRect m_newRect;
};

// 修改形状文本命令
class ChangeTextCommand : public Command {
public:
    ChangeTextCommand(DrawingArea* drawingArea, Shape* shape, 
                    const QString& oldText, const QString& newText);
    
    void execute() override;
    void undo() override;
    QString description() const override;
    
private:
    DrawingArea* m_drawingArea;
    Shape* m_shape;
    QString m_oldText;
    QString m_newText;
};

// 修改形状样式属性命令
class ChangeStyleCommand : public Command {
public:
    enum StyleProperty {
        FillColor,
        LineColor,
        Transparency,
        LineWidth,
        LineStyle,
        FontFamily,
        FontSize,
        FontBold,
        FontItalic,
        FontUnderline,
        FontColor,
        TextAlignment
    };
    
    ChangeStyleCommand(DrawingArea* drawingArea, Shape* shape, 
                     StyleProperty property, const QVariant& oldValue, 
                     const QVariant& newValue);
    
    void execute() override;
    void undo() override;
    QString description() const override;
    
private:
    DrawingArea* m_drawingArea;
    Shape* m_shape;
    StyleProperty m_property;
    QVariant m_oldValue;
    QVariant m_newValue;
    
    void applyProperty(const QVariant& value);
};

// 批量操作命令
class BatchCommand : public Command {
public:
    BatchCommand(const QString& description);
    ~BatchCommand() override;
    
    void addCommand(Command* command);
    
    void execute() override;
    void undo() override;
    QString description() const override;
    
private:
    QVector<Command*> m_commands;
    QString m_description;
};

// 批量移动命令
class BatchMoveCommand : public Command {
public:
    BatchMoveCommand(DrawingArea* drawingArea, 
                    const QVector<Shape*>& shapes,
                    const QVector<QPoint>& oldPositions, 
                    const QVector<QPoint>& newPositions);
    
    void execute() override;
    void undo() override;
    QString description() const override;
    
    // 设置新位置
    void setNewPositions(const QVector<QPoint>& newPositions);
    
private:
    DrawingArea* m_drawingArea;
    QVector<Shape*> m_shapes;
    QVector<QPoint> m_oldPositions;
    QVector<QPoint> m_newPositions;
};

#endif // SHAPECOMMANDS_H 