#include "shapecommands.h"
#include <QVariant>

//=============================================================================
// AddShapeCommand
//=============================================================================

AddShapeCommand::AddShapeCommand(DrawingArea* drawingArea, Shape* shape)
    : m_drawingArea(drawingArea), m_shape(shape), m_shapeOwned(true) {
}

AddShapeCommand::~AddShapeCommand() {
    if (m_shapeOwned) {
        delete m_shape;
    }
}

void AddShapeCommand::execute() {
    if (m_drawingArea && m_shape) {
        m_drawingArea->addShape(m_shape);
        m_shapeOwned = false;  // 所有权转移到DrawingArea
    }
}

void AddShapeCommand::undo() {
    if (m_drawingArea && m_shape) {
        m_drawingArea->removeShape(m_shape);
        m_shapeOwned = true;  // 重新获得所有权
    }
}

QString AddShapeCommand::description() const {
    return QObject::tr("添加图形");
}

//=============================================================================
// DeleteShapeCommand
//=============================================================================

DeleteShapeCommand::DeleteShapeCommand(DrawingArea* drawingArea, Shape* shape)
    : m_drawingArea(drawingArea), m_shape(shape), m_executed(false) {
}

DeleteShapeCommand::~DeleteShapeCommand() {
    if (m_executed) {
        delete m_shape;
    }
}

void DeleteShapeCommand::execute() {
    if (m_drawingArea && m_shape) {
        m_drawingArea->removeShape(m_shape);
        m_executed = true;
    }
}

void DeleteShapeCommand::undo() {
    if (m_drawingArea && m_shape) {
        m_drawingArea->addShape(m_shape);
        m_executed = false;
    }
}

QString DeleteShapeCommand::description() const {
    return QObject::tr("删除图形");
}

//=============================================================================
// MoveShapeCommand
//=============================================================================

MoveShapeCommand::MoveShapeCommand(DrawingArea* drawingArea, Shape* shape, 
                               const QPoint& oldPos, const QPoint& newPos)
    : m_drawingArea(drawingArea), m_shape(shape), m_oldPos(oldPos), m_newPos(newPos) {
}

void MoveShapeCommand::execute() {
    if (m_drawingArea && m_shape) {
        QRect rect = m_shape->getRect();
        rect.moveTopLeft(m_newPos);
        m_shape->setRect(rect);
        m_drawingArea->update();
    }
}

void MoveShapeCommand::undo() {
    if (m_drawingArea && m_shape) {
        QRect rect = m_shape->getRect();
        rect.moveTopLeft(m_oldPos);
        m_shape->setRect(rect);
        m_drawingArea->update();
    }
}

QString MoveShapeCommand::description() const {
    return QObject::tr("移动图形");
}

//=============================================================================
// ResizeShapeCommand
//=============================================================================

ResizeShapeCommand::ResizeShapeCommand(DrawingArea* drawingArea, Shape* shape, 
                                    const QRect& oldRect, const QRect& newRect)
    : m_drawingArea(drawingArea), m_shape(shape), m_oldRect(oldRect), m_newRect(newRect) {
}

void ResizeShapeCommand::execute() {
    if (m_drawingArea && m_shape) {
        m_shape->setRect(m_newRect);
        m_drawingArea->update();
    }
}

void ResizeShapeCommand::undo() {
    if (m_drawingArea && m_shape) {
        m_shape->setRect(m_oldRect);
        m_drawingArea->update();
    }
}

QString ResizeShapeCommand::description() const {
    return QObject::tr("调整图形大小");
}

//=============================================================================
// ChangeTextCommand
//=============================================================================

ChangeTextCommand::ChangeTextCommand(DrawingArea* drawingArea, Shape* shape, 
                                  const QString& oldText, const QString& newText)
    : m_drawingArea(drawingArea), m_shape(shape), m_oldText(oldText), m_newText(newText) {
}

void ChangeTextCommand::execute() {
    if (m_drawingArea && m_shape) {
        m_shape->setText(m_newText);
        m_drawingArea->update();
    }
}

void ChangeTextCommand::undo() {
    if (m_drawingArea && m_shape) {
        m_shape->setText(m_oldText);
        m_drawingArea->update();
    }
}

QString ChangeTextCommand::description() const {
    return QObject::tr("修改文本");
}

//=============================================================================
// ChangeStyleCommand
//=============================================================================

ChangeStyleCommand::ChangeStyleCommand(DrawingArea* drawingArea, Shape* shape, 
                                    StyleProperty property, 
                                    const QVariant& oldValue, 
                                    const QVariant& newValue)
    : m_drawingArea(drawingArea), m_shape(shape), 
      m_property(property), m_oldValue(oldValue), m_newValue(newValue) {
}

void ChangeStyleCommand::execute() {
    if (m_drawingArea && m_shape) {
        applyProperty(m_newValue);
        m_drawingArea->update();
    }
}

void ChangeStyleCommand::undo() {
    if (m_drawingArea && m_shape) {
        applyProperty(m_oldValue);
        m_drawingArea->update();
    }
}

QString ChangeStyleCommand::description() const {
    switch (m_property) {
        case FillColor: return QObject::tr("修改填充颜色");
        case LineColor: return QObject::tr("修改线条颜色");
        case Transparency: return QObject::tr("修改透明度");
        case LineWidth: return QObject::tr("修改线条粗细");
        case LineStyle: return QObject::tr("修改线条样式");
        case FontFamily: return QObject::tr("修改字体");
        case FontSize: return QObject::tr("修改字体大小");
        case FontBold: return QObject::tr("修改字体粗细");
        case FontItalic: return QObject::tr("修改字体样式");
        case FontUnderline: return QObject::tr("修改字体下划线");
        case FontColor: return QObject::tr("修改字体颜色");
        case TextAlignment: return QObject::tr("修改文本对齐方式");
        default: return QObject::tr("修改样式");
    }
}

void ChangeStyleCommand::applyProperty(const QVariant& value) {
    switch (m_property) {
        case FillColor:
            m_shape->setFillColor(value.value<QColor>());
            break;
        case LineColor:
            m_shape->setLineColor(value.value<QColor>());
            break;
        case Transparency:
            m_shape->setTransparency(value.toInt());
            break;
        case LineWidth:
            m_shape->setLineWidth(value.toReal());
            break;
        case LineStyle:
            m_shape->setLineStyle(value.toInt());
            break;
        case FontFamily:
            m_shape->setFontFamily(value.toString());
            break;
        case FontSize:
            m_shape->setFontSize(value.toInt());
            break;
        case FontBold:
            m_shape->setFontBold(value.toBool());
            break;
        case FontItalic:
            m_shape->setFontItalic(value.toBool());
            break;
        case FontUnderline:
            m_shape->setFontUnderline(value.toBool());
            break;
        case FontColor:
            m_shape->setFontColor(value.value<QColor>());
            break;
        case TextAlignment:
            m_shape->setTextAlignment(static_cast<Qt::Alignment>(value.toInt()));
            break;
    }
}

//=============================================================================
// BatchCommand
//=============================================================================

BatchCommand::BatchCommand(const QString& description)
    : m_description(description) {
}

BatchCommand::~BatchCommand() {
    // 清理所有子命令
    for (Command* command : m_commands) {
        delete command;
    }
    m_commands.clear();
}

void BatchCommand::addCommand(Command* command) {
    if (command) {
        m_commands.append(command);
    }
}

void BatchCommand::execute() {
    for (Command* command : m_commands) {
        command->execute();
    }
}

void BatchCommand::undo() {
    // 逆序撤销
    for (int i = m_commands.size() - 1; i >= 0; --i) {
        m_commands[i]->undo();
    }
}

QString BatchCommand::description() const {
    return m_description;
}

//=============================================================================
// BatchMoveCommand
//=============================================================================

BatchMoveCommand::BatchMoveCommand(DrawingArea* drawingArea, 
                               const QVector<Shape*>& shapes,
                               const QVector<QPoint>& oldPositions, 
                               const QVector<QPoint>& newPositions)
    : m_drawingArea(drawingArea), m_shapes(shapes), 
      m_oldPositions(oldPositions), m_newPositions(newPositions) {
}

void BatchMoveCommand::execute() {
    if (m_drawingArea) {
        for (int i = 0; i < m_shapes.size(); ++i) {
            if (i < m_newPositions.size()) {
                QRect rect = m_shapes[i]->getRect();
                rect.moveTopLeft(m_newPositions[i]);
                m_shapes[i]->setRect(rect);
            }
        }
        m_drawingArea->update();
    }
}

void BatchMoveCommand::undo() {
    if (m_drawingArea) {
        for (int i = 0; i < m_shapes.size(); ++i) {
            if (i < m_oldPositions.size()) {
                QRect rect = m_shapes[i]->getRect();
                rect.moveTopLeft(m_oldPositions[i]);
                m_shapes[i]->setRect(rect);
            }
        }
        m_drawingArea->update();
    }
}

QString BatchMoveCommand::description() const {
    return QObject::tr("批量移动图形");
} 