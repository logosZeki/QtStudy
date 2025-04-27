#include "toolbar.h"
#include <QDrag>
#include <QMimeData>
#include <QPainter>

// ShapeItem实现
ShapeItem::ShapeItem(const QString& type, const QString& displayName, QWidget* parent)
    : QWidget(parent), m_type(type), m_displayName(displayName)
{
    // 设置固定大小
    setFixedHeight(70);
    
    // 根据类型调整形状的大小和位置
    if (type == ShapeTypes::Rectangle || type == ShapeTypes::Ellipse) {
        m_shapeRect = QRect(15, 15, 70, 40);
    } else if (type == ShapeTypes::Circle) {
        m_shapeRect = QRect(25, 10, 50, 50);
    } else if (type == ShapeTypes::Pentagon) {
        m_shapeRect = QRect(25, 10, 50, 50);
    } else {
        m_shapeRect = QRect(25, 10, 50, 50);
    }
    
    // 允许鼠标追踪
    setMouseTracking(true);
}

void ShapeItem::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    painter.fillRect(rect(), QColor(240, 240, 240));
    
    // 绘制形状
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(QColor(255, 255, 255)));
    
    if (m_type == ShapeTypes::Rectangle) {
        painter.drawRect(m_shapeRect);
    } else if (m_type == ShapeTypes::Circle) {
        painter.drawEllipse(m_shapeRect);
    } else if (m_type == ShapeTypes::Pentagon) {
        QPolygon polygon;
        int centerX = m_shapeRect.center().x();
        int centerY = m_shapeRect.center().y();
        int radius = qMin(m_shapeRect.width(), m_shapeRect.height()) / 2;
        
        for (int i = 0; i < 5; ++i) {
            double angle = i * 2 * M_PI / 5 - M_PI / 2;
            int x = centerX + radius * cos(angle);
            int y = centerY + radius * sin(angle);
            polygon << QPoint(x, y);
        }
        
        painter.drawPolygon(polygon);
    } else if (m_type == ShapeTypes::Ellipse) {
        painter.drawEllipse(m_shapeRect);
    }

    // 绘制文字
    painter.drawText(rect(), Qt::AlignBottom | Qt::AlignHCenter, m_displayName);
}

void ShapeItem::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QDrag* drag = new QDrag(this);
        QMimeData* mimeData = new QMimeData;
        
        // 使用形状类型作为拖拽数据
        mimeData->setText(m_type);
        drag->setMimeData(mimeData);
        
        // 设置拖拽的图像
        QPixmap pixmap(size());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        render(&painter);
        drag->setPixmap(pixmap);
        drag->setHotSpot(event->pos());
        
        drag->exec(Qt::CopyAction);
    }
}

void ShapeItem::mouseMoveEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    // 鼠标移动时的行为可以在这里添加
}

// ToolBar实现
ToolBar::ToolBar(QWidget* parent)
    : QWidget(parent)
{
    // 设置工具栏的布局
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    
    // 添加标题
    m_titleLabel = new QLabel("形状工具箱", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("background-color: #cccccc; font-weight: bold; padding: 5px;");
    m_layout->addWidget(m_titleLabel);
    
    // 添加形状项
    m_layout->addWidget(new ShapeItem(ShapeTypes::Rectangle, "矩形", this));
    m_layout->addWidget(new ShapeItem(ShapeTypes::Circle, "圆形", this));
    m_layout->addWidget(new ShapeItem(ShapeTypes::Pentagon, "五边形", this));
    m_layout->addWidget(new ShapeItem(ShapeTypes::Ellipse, "椭圆形", this));
    
    // 添加弹簧以占用剩余空间
    m_layout->addStretch();
    
    // 设置固定宽度
    setFixedWidth(100);
}
