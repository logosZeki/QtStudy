#include "util/Utils.h"
#include <QPainter>  // 添加QPainter的头文件

// 定义默认尺寸（像素）
static constexpr int Default_WIDTH = 1600;    
static constexpr int Default_HEIGHT = 780;   

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