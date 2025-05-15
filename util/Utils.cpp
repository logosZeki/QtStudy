#include "util/Utils.h"
#include <QPainter>  // 添加QPainter的头文件
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>

QCursor Utils::getCrossCursor(){
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 2));
    // 画水平线
    painter.drawLine(0, 16, 31, 16);
    // 画垂直线
    painter.drawLine(16, 0, 16, 31);
    painter.end();

    // 设置热点位置（十字交叉点）
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
    // 创建主按钮
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
    
    // 创建一个垂直布局，包含文本和色块
    QVBoxLayout* fontBtnLayout = new QVBoxLayout(button);
    fontBtnLayout->setContentsMargins(0, 4, 0, 3);
    fontBtnLayout->setSpacing(3);
    fontBtnLayout->setAlignment(Qt::AlignCenter);
    
    // 添加文本标签
    QLabel* fontLabel = new QLabel(text, button);
    fontLabel->setFixedWidth(btnWidth);
    fontLabel->setAlignment(Qt::AlignCenter);
    fontLabel->setStyleSheet("QLabel { color: #333; font-size: 12px; }");
    fontBtnLayout->addWidget(fontLabel);
    
    // 添加颜色小方块
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
    
    // 让颜色指示器居中
    QHBoxLayout* indicatorLayout = new QHBoxLayout();
    indicatorLayout->setContentsMargins(0, 0, 0, 0);
    indicatorLayout->setSpacing(0);
    indicatorLayout->setAlignment(Qt::AlignCenter);
    indicatorLayout->addWidget(colorIndicator);
    fontBtnLayout->addLayout(indicatorLayout);
    
    // 设置按钮的属性，以便在主窗口中可以查找到颜色指示器
    colorIndicator->setObjectName("colorIndicator");
    
    return button;
}

void Utils::updateColorButton(QPushButton* button, const QColor& color) {
    if (!button) return;
    
    // 根据按钮的启用/禁用状态更新样式
    QFrame* colorIndicator = button->findChild<QFrame*>("colorIndicator");
    if (!colorIndicator) return;
    
    if (button->isEnabled()) {
        // 启用状态 - 显示当前颜色
        colorIndicator->setStyleSheet(
            QString("QFrame { "
                   "  background-color: %1; "
                   "  border: 1px solid #666; "
                   "  border-radius: 3px; "
                   "}")
            .arg(color.name())
        );
        
        // 启用标签 - 正常颜色
        QLabel* label = button->findChild<QLabel*>();
        if (label) {
            label->setStyleSheet("QLabel { color: #333; font-size: 12px; }");
        }
    } else {
        // 禁用状态 - 始终使用白色，不管传入的color是什么
        colorIndicator->setStyleSheet(
            "QFrame { "
            "  background-color: rgb(255, 255, 255); "
            "  border: 1px solid #aaa; "
            "  border-radius: 3px; "
            "}"
        );
        
        // 禁用标签 - 淡化文字颜色
        QLabel* label = button->findChild<QLabel*>();
        if (label) {
            label->setStyleSheet("QLabel { color: rgba(0, 0, 0, 0.25); font-size: 12px; }");
        }
    }
}