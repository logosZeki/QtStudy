#ifndef UTILS_H
#define UTILS_H

#include <QCursor>  // 添加这一行以包含QCursor的声明

class Utils {
public:


    // 禁用构造和析构
    Utils() = delete;
    ~Utils() = delete;
    
    // 禁用拷贝和移动
    Utils(const Utils&) = delete;
    Utils& operator=(const Utils&) = delete;
    Utils(Utils&&) = delete;
    Utils& operator=(Utils&&) = delete;

    // 静态工具方法
    static QCursor getCrossCursor();

    static constexpr int Default_WIDTH = 1600;    
    static constexpr int Default_HEIGHT = 840;
        // 定义A4尺寸（像素）
    static constexpr int A4_WIDTH = 1050;    // A4宽度为1050像素
    static constexpr int A4_HEIGHT = 1500;   // A4高度为1500像素

    // 定义A3尺寸（像素）
    static constexpr int A3_WIDTH = 1500;    // A3宽度为1500像素
    static constexpr int A3_HEIGHT = 2100;   // A3高度为2100像素

    // 定义A5尺寸（像素）
    static constexpr int A5_WIDTH = 750;    // A5宽度为750像素
    static constexpr int A5_HEIGHT = 1050;   // A5高度为1050像素

    static constexpr qreal MAX_SCALE = 2.0;      // 最大缩放比例
    static constexpr qreal MIN_SCALE = 0.20;     // 最小缩放比例

};

#endif // UTILS_H