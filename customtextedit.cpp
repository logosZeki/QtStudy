#include "customtextedit.h"
#include <QTextCursor>
#include <QTextBlock>

CustomTextEdit::CustomTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    setAlignment(Qt::AlignCenter);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void CustomTextEdit::showEvent(QShowEvent *event)
{
    QTextEdit::showEvent(event);
    centerCursor();
}

void CustomTextEdit::resizeEvent(QResizeEvent *event)
{
    QTextEdit::resizeEvent(event);
    if (toPlainText().isEmpty()) {
        centerCursor();
    }
}

void CustomTextEdit::centerCursor()
{
    if (toPlainText().isEmpty()) {
        // Create an empty document with a single block
        clear();
        
        // Add empty lines to push the cursor down
        int linesToAdd = document()->blockCount();
        int targetLines = height() / fontMetrics().height() / 2;
        
        for (int i = linesToAdd; i < targetLines; i++) {
            QTextCursor cursor = textCursor();
            cursor.insertBlock();
        }
        
        // Move cursor to the middle and ensure it's centered horizontally
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        
        // Ensure horizontal center alignment remains
        setAlignment(Qt::AlignCenter);
    }
}
