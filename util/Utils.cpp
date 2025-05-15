#include "util/Utils.h"
#include <QPainter>  
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
QCursor Utils::getCrossCursor(){
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(0, 16, 31, 16);
    painter.drawLine(16, 0, 16, 31);
    painter.end();
    QCursor customCursor(pixmap, 16, 16);
    return customCursor;
}
QPushButton* Utils::getAutoChangeColorsButton(
    QWidget* parent, 
    const QString& text, 
    int btnWidth, 
    int btnHeight, 
    int indicatorWidth, 
    int indicatorHeight
) {
    QPushButton* button = new QPushButton("", parent);
    button->setToolTip(QObject::tr("Font Color"));
    button->setFixedHeight(btnHeight);
    button->setFixedWidth(btnWidth);
    button->setEnabled(false);
    button->setStyleSheet(
        "QPushButton { "
        "  padding: 1px; "
        "  border-radius: 4px; "
        "  background-color: #f5f5f7; "
        "  text-align: center; "
        "}"
        "QPushButton:hover { background-color: #e5e5e5; }"
        "QPushButton:disabled { color: rgba(0, 0, 0, 0.25); }"
    );
    QVBoxLayout* fontBtnLayout = new QVBoxLayout(button);
    fontBtnLayout->setContentsMargins(0, 4, 0, 3);
    fontBtnLayout->setSpacing(3);
    fontBtnLayout->setAlignment(Qt::AlignCenter);
    QLabel* fontLabel = new QLabel(text, button);
    fontLabel->setFixedWidth(btnWidth);
    fontLabel->setAlignment(Qt::AlignCenter);
    fontLabel->setStyleSheet("QLabel { color: #333; font-size: 12px; }");
    fontBtnLayout->addWidget(fontLabel);
    QFrame* colorIndicator = new QFrame(button);
    colorIndicator->setFixedHeight(indicatorHeight);
    colorIndicator->setFixedWidth(indicatorWidth);
    colorIndicator->setFrameShape(QFrame::StyledPanel);
    colorIndicator->setStyleSheet(
        "QFrame { "
        "  background-color: white; "
        "  border: 1px solid #666; "
        "  border-radius: 3px; "
        "}"
    );
    QHBoxLayout* indicatorLayout = new QHBoxLayout();
    indicatorLayout->setContentsMargins(0, 0, 0, 0);
    indicatorLayout->setSpacing(0);
    indicatorLayout->setAlignment(Qt::AlignCenter);
    indicatorLayout->addWidget(colorIndicator);
    fontBtnLayout->addLayout(indicatorLayout);
    colorIndicator->setObjectName("colorIndicator");
    return button;
}
void Utils::updateColorButton(QPushButton* button, const QColor& color) {
    if (!button) return;
    QFrame* colorIndicator = button->findChild<QFrame*>("colorIndicator");
    if (!colorIndicator) return;
    if (button->isEnabled()) {
        colorIndicator->setStyleSheet(
            QString("QFrame { "
                   "  background-color: %1; "
                   "  border: 1px solid #666; "
                   "  border-radius: 3px; "
                   "}")
            .arg(color.name())
        );
        QLabel* label = button->findChild<QLabel*>();
        if (label) {
            label->setStyleSheet("QLabel { color: #333; font-size: 12px; }");
        }
    } else {
        colorIndicator->setStyleSheet(
            "QFrame { "
            "  background-color: rgb(255, 255, 255); "
            "  border: 1px solid #aaa; "
            "  border-radius: 3px; "
            "}"
        );
        QLabel* label = button->findChild<QLabel*>();
        if (label) {
            label->setStyleSheet("QLabel { color: rgba(0, 0, 0, 0.25); font-size: 12px; }");
        }
    }
}
