#include "toolbar.h"
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QIcon>
#include "chart/shape.h"

// ShapeItem实现 (ShapeItem implementation)
ShapeItem::ShapeItem(const QString& type, QWidget* parent)
    : QWidget(parent), m_type(type)
{
    // 设置固定大小 (Set fixed size)
    setFixedSize(40, 40);
    
    // 根据类型调整形状的大小和位置 (Adjust shape size and position based on type)
    if (type == ShapeTypes::Rectangle || type == ShapeTypes::Ellipse) {
        m_shapeRect = QRect(8, 8, 24, 24);
    } else if (type == ShapeTypes::Circle) {
        m_shapeRect = QRect(8, 8, 24, 24);
    } else if (type == ShapeTypes::Pentagon) {
        m_shapeRect = QRect(8, 8, 24, 24);
    } else if (type == ShapeTypes::ArrowLine) {
        m_shapeRect = QRect(6, 20, 28, 1); // 为直线设置一个细长的矩形
    } else {
        m_shapeRect = QRect(8, 8, 24, 24);
    }
    
    // 设置工具提示
    if (type == ShapeTypes::Rectangle) {
        setToolTip(tr("矩形"));
    } else if (type == ShapeTypes::Circle) {
        setToolTip(tr("圆形"));
    } else if (type == ShapeTypes::Pentagon) {
        setToolTip(tr("五边形"));
    } else if (type == ShapeTypes::Ellipse) {
        setToolTip(tr("椭圆"));
    } else if (type == ShapeTypes::ArrowLine) {
        setToolTip(tr("箭头线"));
    }
    
    // 允许鼠标追踪 (Allow mouse tracking)
    setMouseTracking(true);
}

void ShapeItem::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    QColor bgColor = QColor(250, 250, 250);
    painter.fillRect(rect(), bgColor);
    
    // 设置边框
    painter.setPen(QPen(QColor(220, 220, 220), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    
    // 绘制形状
    painter.setPen(QPen(Qt::black, 1));
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
    } else if (m_type == ShapeTypes::ArrowLine) {
        // 绘制带箭头的直线
        QPoint startPoint = m_shapeRect.topLeft();
        QPoint endPoint = m_shapeRect.topRight();
        
        // 绘制直线
        painter.setPen(QPen(Qt::black, 1));
        painter.drawLine(startPoint, endPoint);
        
        // 绘制箭头
        const int arrowSize = 6;
        QPoint arrowP1 = endPoint - QPoint(arrowSize, arrowSize / 2);
        QPoint arrowP2 = endPoint - QPoint(arrowSize, -arrowSize / 2);
        
        QPolygon arrow;
        arrow << endPoint << arrowP1 << arrowP2;
        painter.setBrush(Qt::black);
        painter.drawPolygon(arrow);
    }
}

void ShapeItem::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QDrag* drag = new QDrag(this);
        QMimeData* mimeData = new QMimeData;
        
        // 使用形状类型作为拖拽数据
        mimeData->setText(m_type);
        drag->setMimeData(mimeData);
        
        // 设置拖拽的图像为透明背景
        QPixmap pixmap(size());
        pixmap.fill(Qt::transparent); // 使用透明填充而不是背景色
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // 绘制形状但不绘制背景
        painter.setPen(QPen(Qt::black, 1));
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
        } else if (m_type == ShapeTypes::ArrowLine) {
            // 绘制带箭头的直线
            QPoint startPoint = m_shapeRect.topLeft();
            QPoint endPoint = m_shapeRect.topRight();
            
            // 绘制直线
            painter.drawLine(startPoint, endPoint);
            
            // 绘制箭头
            const int arrowSize = 6;
            QPoint arrowP1 = endPoint - QPoint(arrowSize, arrowSize / 2);
            QPoint arrowP2 = endPoint - QPoint(arrowSize, -arrowSize / 2);
            
            QPolygon arrow;
            arrow << endPoint << arrowP1 << arrowP2;
            painter.setBrush(Qt::black);
            painter.drawPolygon(arrow);
        }
        
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

// ShapeCategory实现
ShapeCategory::ShapeCategory(const QString& title, QWidget* parent)
    : QWidget(parent), m_title(title), m_currentRow(0), m_currentColumn(0)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 0);
    m_mainLayout->setSpacing(5);
    
    // 创建标题标签（带有折叠图标）
    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setStyleSheet("font-size: 12px; font-weight: bold; padding-bottom: 5px;");
    m_mainLayout->addWidget(m_titleLabel);
    
    // 创建形状布局（网格形式）
    m_shapesLayout = new QGridLayout();
    m_shapesLayout->setContentsMargins(0, 0, 0, 0);
    m_shapesLayout->setSpacing(2);
    m_mainLayout->addLayout(m_shapesLayout);
}

void ShapeCategory::addShape(const QString& type)
{
    // 按照5列网格添加形状项
    ShapeItem* item = new ShapeItem(type, this);
    m_shapesLayout->addWidget(item, m_currentRow, m_currentColumn);
    
    // 更新行列计数
    m_currentColumn++;
    if (m_currentColumn >= 5) {
        m_currentColumn = 0;
        m_currentRow++;
    }
}

// ToolBar实现
ToolBar::ToolBar(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void ToolBar::setupUI()
{
    // 设置工具栏的布局
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    
    // 创建标签栏
    m_tabBar = new QTabBar(this);
    m_tabBar->addTab(tr("图形库"));
    m_tabBar->addTab(tr("风格"));
    m_tabBar->setExpanding(true);  // 让标签填充整个宽度
    m_tabBar->setDocumentMode(true);
    m_tabBar->setDrawBase(false);
    m_tabBar->setStyleSheet("QTabBar::tab { padding: 8px 0; border: none; border-bottom: 1px solid #e0e0e0; background-color: white; }"
                          "QTabBar::tab:selected { border-bottom: 2px solid #1a73e8; color: #1a73e8; }"
                          "QTabBar::tab:hover:!selected { background-color: #f5f5f5; }");
    connect(m_tabBar, &QTabBar::currentChanged, this, &ToolBar::onTabChanged);
    m_layout->addWidget(m_tabBar);
    
    // 创建搜索框
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText(tr("请输入搜索内容"));
    m_searchBox->setStyleSheet("QLineEdit { border: 1px solid #e0e0e0; border-radius: 3px; padding: 5px; margin: 8px; }");
    m_layout->addWidget(m_searchBox);
    
    // 创建滚动区域
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_layout->addWidget(m_scrollArea, 1); // 让滚动区域占据剩余空间
    
    // 创建图形库和风格面板
    m_libraryWidget = new QWidget(this);
    m_stylesWidget = new QWidget(this);
    
    // 创建图形库面板内容
    createGraphicsLibTab();
    
    // 创建风格面板内容
    createStylesTab();
    
    // 默认显示图形库面板
    m_scrollArea->setWidget(m_libraryWidget);
    m_stylesWidget->hide();
}

void ToolBar::createGraphicsLibTab()
{
    QVBoxLayout* layout = new QVBoxLayout(m_libraryWidget);
    layout->setContentsMargins(5, 0, 5, 5);
    layout->setSpacing(10);
    
    // 创建基础图形类别
    ShapeCategory* basicCategory = new ShapeCategory(tr("基础图形"), m_libraryWidget);
    basicCategory->addShape(ShapeTypes::Rectangle);
    basicCategory->addShape(ShapeTypes::Circle);
    basicCategory->addShape(ShapeTypes::Ellipse);
    basicCategory->addShape(ShapeTypes::Pentagon);
    basicCategory->addShape(ShapeTypes::ArrowLine);
    layout->addWidget(basicCategory);
    
    // 创建Flowchart流程图类别
    ShapeCategory* flowchartCategory = new ShapeCategory(tr("Flowchart 流程图"), m_libraryWidget);
    flowchartCategory->addShape(ShapeTypes::Rectangle);
    flowchartCategory->addShape(ShapeTypes::Pentagon);
    flowchartCategory->addShape(ShapeTypes::Ellipse);
    flowchartCategory->addShape(ShapeTypes::ArrowLine);
    layout->addWidget(flowchartCategory);
    
    // 创建UML类图类别
    ShapeCategory* umlCategory = new ShapeCategory(tr("UML 类图"), m_libraryWidget);
    umlCategory->addShape(ShapeTypes::Rectangle);
    umlCategory->addShape(ShapeTypes::ArrowLine);
    layout->addWidget(umlCategory);
    
    // 添加弹簧占用剩余空间
    layout->addStretch();
}

void ToolBar::createStylesTab()
{
    QVBoxLayout* layout = new QVBoxLayout(m_stylesWidget);
    layout->setContentsMargins(5, 0, 5, 5);
    layout->setSpacing(10);
    
    // 这里可以添加样式相关的控件
    QLabel* notImplementedLabel = new QLabel(tr("样式功能未实现"), m_stylesWidget);
    notImplementedLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(notImplementedLabel);
    
    // 添加弹簧占用剩余空间
    layout->addStretch();
}

void ToolBar::onTabChanged(int index)
{
    // 切换标签页
    if (index == 0) {  // 图形库
        m_scrollArea->setWidget(m_libraryWidget);
        m_libraryWidget->show();
        m_stylesWidget->hide();
    } else if (index == 1) {  // 风格
        m_scrollArea->setWidget(m_stylesWidget);
        m_stylesWidget->show();
        m_libraryWidget->hide();
    }
}
