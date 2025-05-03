#include "util/Utils.h"
#include <QPainter>  // 添加QPainter的头文件



QCursor Utils::getCrossCursor(){
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 1));
    // 画水平线
    painter.drawLine(0, 16, 31, 16);
    // 画垂直线
    painter.drawLine(16, 0, 16, 31);
    painter.end();

    // 设置热点位置（十字交叉点）
    QCursor customCursor(pixmap, 16, 16);
    return customCursor;
}