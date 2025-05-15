#ifndef UTILS_H
#define UTILS_H

#include <QCursor> 
#include <QPushButton>
#include <QColor>

class Utils {
public:


    // 禁用构造和析构
    Utils() = delete;
    ~Utils() = delete;
    
    Utils(const Utils&) = delete;
    Utils& operator=(const Utils&) = delete;
    Utils(Utils&&) = delete;
    Utils& operator=(Utils&&) = delete;

    // 十字无箭头光标
    static QCursor getCrossCursor();
    
    // 获取自动变色按钮
    static QPushButton* getAutoChangeColorsButton(
        QWidget* parent, 
        const QString& text = "Font", 
        int btnWidth = 54, 
        int btnHeight = 38, 
        int indicatorWidth = 28, 
        int indicatorHeight = 8
    );
    
    // 更新颜色按钮的显示颜色
    static void updateColorButton(QPushButton* button, const QColor& color);

    static const int Default_WIDTH = 1600;    
    static const int Default_HEIGHT = 780;
    static const int A4_WIDTH = 1050; 
    static const int A4_HEIGHT = 1500; 
    static const int A3_WIDTH = 1500; 
    static const int A3_HEIGHT = 2100; 
    static const int A5_WIDTH = 750; 
    static const int A5_HEIGHT = 1050; 
    static constexpr qreal MAX_SCALE = 2.0;  
    static constexpr qreal MIN_SCALE = 0.35; 

};

#endif // UTILS_H