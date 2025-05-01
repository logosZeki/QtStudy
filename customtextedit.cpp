#include "customtextedit.h"
#include <QTextCursor>
#include <QTextBlock>

CustomTextEdit::CustomTextEdit(QWidget* parent)
    : QTextEdit(parent)
{
    setAlignment(Qt::AlignCenter);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_centerPending = false;
}

void CustomTextEdit::showEvent(QShowEvent* event)
{
    QTextEdit::showEvent(event);
    m_centerPending = true;

    // 使用单次计时器延迟执行centerCursor，确保只执行一次
    QMetaObject::invokeMethod(this, "performCenterCursor", Qt::QueuedConnection);
}

void CustomTextEdit::resizeEvent(QResizeEvent* event)
{
    QTextEdit::resizeEvent(event);
    // 标记需要居中，但不立即执行
    m_centerPending = true;

    // 使用单次计时器延迟执行centerCursor，确保只执行一次
    QMetaObject::invokeMethod(this, "performCenterCursor", Qt::QueuedConnection);

}
void CustomTextEdit::performCenterCursor()
{
    // 检查是否仍需执行居中操作
    if (m_centerPending) {
        m_centerPending = false;
        centerCursor();
    }
}
void CustomTextEdit::centerCursor()
{
    // Save current text and cursor position
    QString originalText = toPlainText();
    int cursorPosition = textCursor().position();

    // Clear the document to start fresh
    clear();

    // Add empty lines to push the content vertically center
    int availableHeight = height();
    int lineHeight = fontMetrics().height();
    int textHeight = 0;

    if (!originalText.isEmpty()) {
        // Calculate the expected height of the text
        QStringList lines = originalText.split('\n');
        textHeight = lines.size() * lineHeight;

        // Calculate top margin (empty lines before text)
        int topMargin = (availableHeight - textHeight) / 2;
        int linesToAdd = topMargin / lineHeight;

        // Add empty lines at the top
        for (int i = 0; i < linesToAdd; i++) {
            QTextCursor cursor = textCursor();
            cursor.insertBlock();
            setTextCursor(cursor);
        }

        // Insert the text
        QTextCursor cursor = textCursor();
        cursor.insertText(originalText);

        // Position cursor after the text
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
    }
    else {
        // Original empty text handling
        int linesToAdd = document()->blockCount();
        int targetLines = height() / fontMetrics().height() / 2;

        for (int i = linesToAdd; i < targetLines; i++) {
            QTextCursor cursor = textCursor();
            cursor.insertBlock();
        }

        // Move cursor to the middle
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
    }

    // Ensure horizontal center alignment
    setAlignment(Qt::AlignCenter);
}
