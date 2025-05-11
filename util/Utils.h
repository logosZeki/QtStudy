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
    static constexpr int Default_HEIGHT = 780;

};

#endif // UTILS_H