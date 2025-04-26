#include "toolbar.h"
#include <QDrag>
#include <QMimeData>
#include <QPainter>

// ShapeItem实现
ShapeItem::ShapeItem(ShapeType type, const QString &name, QWidget *parent)
    : QWidget(parent), m_type(type), m_name(name)
{
    // 设置固定大小
    setFixedHeight(70);
    m_shapeRect = QRect(20, 10, 50, 50);

    // 允许鼠标追踪
    setMouseTracking(true);
}

void ShapeItem::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    painter.fillRect(rect(), QColor(240, 240, 240));

    // 绘制形状
    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(QBrush(QColor(255, 255, 255)));

    switch (m_type) {
    case Rectangle:
        painter.drawRect(m_shapeRect);
        break;
    case Circle:
        painter.drawEllipse(m_shapeRect);
        break;
    case Pentagon: {
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
        break;
    }
    case Ellipse:
        painter.drawEllipse(m_shapeRect);
        break;
    }

    // 绘制名称
    painter.drawText(QRect(0, m_shapeRect.bottom() + 5, width(), 20),
                    Qt::AlignCenter, m_name);
}

void ShapeItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_shapeRect.contains(event->pos())) {
        // 记录拖动开始
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        // 存储形状类型信息
        mimeData->setText(QString::number(m_type));
        drag->setMimeData(mimeData);

        // 开始拖动
        drag->exec(Qt::CopyAction);
    }
}

void ShapeItem::mouseMoveEvent(QMouseEvent *event)
{
    if (m_shapeRect.contains(event->pos())) {
        setCursor(Qt::OpenHandCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

// ToolBar实现
ToolBar::ToolBar(QWidget *parent)
    : QWidget(parent)
{
    // 设置背景色
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(240, 240, 240));
    setAutoFillBackground(true);
    setPalette(pal);

    // 创建布局
    m_layout = new QVBoxLayout(this);

    // 创建标题
    m_titleLabel = new QLabel("工具栏", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

    // 创建形状项
    ShapeItem *rectangleItem = new ShapeItem(Rectangle, "矩形", this);
    ShapeItem *circleItem = new ShapeItem(Circle, "圆形", this);
    ShapeItem *pentagonItem = new ShapeItem(Pentagon, "五边形", this);
    ShapeItem *ellipseItem = new ShapeItem(Ellipse, "椭圆形", this);

    // 添加到布局
    m_layout->addWidget(m_titleLabel);
    m_layout->addWidget(rectangleItem);
    m_layout->addWidget(circleItem);
    m_layout->addWidget(pentagonItem);
    m_layout->addWidget(ellipseItem);
    m_layout->addStretch();
}
