#include "chart/customtextedit.h"
#include <QTextCursor>
#include <QTextBlock>
CustomTextEdit::CustomTextEdit(QWidget* parent)
    : QTextEdit(parent)
{
    setAlignment(Qt::AlignCenter);
    m_alignment = Qt::AlignCenter;
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_centerPending = false;
}
void CustomTextEdit::setTextAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;
    setAlignment(alignment);
    m_centerPending = true;
    QMetaObject::invokeMethod(this, "performCenterCursor", Qt::QueuedConnection);
}
void CustomTextEdit::showEvent(QShowEvent* event)
{
    QTextEdit::showEvent(event);
    m_centerPending = true;
    QMetaObject::invokeMethod(this, "performCenterCursor", Qt::QueuedConnection);
}
void CustomTextEdit::resizeEvent(QResizeEvent* event)
{
    QTextEdit::resizeEvent(event);
    m_centerPending = true;
    QMetaObject::invokeMethod(this, "performCenterCursor", Qt::QueuedConnection);
}
void CustomTextEdit::performCenterCursor()
{
    if (m_centerPending) {
        m_centerPending = false;
        centerCursor();
    }
}
void CustomTextEdit::centerCursor()
{
    QString originalText = toPlainText();
    int cursorPosition = textCursor().position();
    clear();
    int availableHeight = height();
    int lineHeight = fontMetrics().height();
    int textHeight = 0;
    if (!originalText.isEmpty()) {
        QStringList lines = originalText.split('\n');
        textHeight = lines.size() * lineHeight;
        int topMargin = (availableHeight - textHeight) / 2;
        int linesToAdd = topMargin / lineHeight;
        for (int i = 0; i < linesToAdd; i++) {
            QTextCursor cursor = textCursor();
            cursor.insertBlock();
            setTextCursor(cursor);
        }
        QTextCursor cursor = textCursor();
        cursor.insertText(originalText);
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
    }
    else {
        int linesToAdd = document()->blockCount();
        int targetLines = height() / fontMetrics().height() / 2;
        for (int i = linesToAdd; i < targetLines; i++) {
            QTextCursor cursor = textCursor();
            cursor.insertBlock();
        }
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
    }
    setAlignment(m_alignment);
}
