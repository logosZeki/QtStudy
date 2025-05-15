#include "mainwindow.h"
#include "toolbar.h"
#include "drawingarea.h"
#include "pagesettingdialog.h"
#include "util/Utils.h"
#include <QAction>
#include <QStyle>
#include <QIcon>
#include <QScrollArea>
#include <QFontDatabase>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QGraphicsEffect>
#include <QPropertyAnimation>
#include <QToolButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_pageSettingDialog(nullptr)
{
    // 设置无边框窗口
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    
    setupUi();
    
    // 设置窗口标题和大小
    setWindowTitle(tr("Flowchart Designer"));
    resize(1900, 1000);
    
    // 连接DrawingArea的shapeSelectionChanged信号到updateFontControls槽
    connect(m_drawingArea, &DrawingArea::shapeSelectionChanged, this, &MainWindow::updateFontControls);
    
    // 连接DrawingArea的shapeSelectionChanged信号到updateArrangeControls槽
    connect(m_drawingArea, &DrawingArea::shapeSelectionChanged, this, &MainWindow::updateArrangeControls);
    
    // 连接DrawingArea的颜色变化信号到updateColorButtons槽
    connect(m_drawingArea, &DrawingArea::fontColorChanged, this, &MainWindow::updateColorButtons);
    connect(m_drawingArea, &DrawingArea::fillColorChanged, this, &MainWindow::updateColorButtons);
    connect(m_drawingArea, &DrawingArea::lineColorChanged, this, &MainWindow::updateColorButtons);
    connect(m_drawingArea, &DrawingArea::shapeSelectionChanged, this, &MainWindow::updateColorButtons);
    
    // 连接状态栏更新信号
    connect(m_drawingArea, &DrawingArea::selectionChanged, this, &MainWindow::updateStatusBarInfo);
    connect(m_drawingArea, &DrawingArea::scaleChanged, this, &MainWindow::updateZoomSlider);
    connect(m_drawingArea, &DrawingArea::shapesCountChanged, this, &MainWindow::updateStatusBarInfo);
    
    // 设置DrawingArea初始缩放比例
    updateZoomSlider();
    
    // 初始化字体控件状态
    updateFontControls();
    
    // 初始化状态栏信息
    updateStatusBarInfo();
}

MainWindow::~MainWindow()
{
    if (m_pageSettingDialog) {
        delete m_pageSettingDialog;
    }
}

void MainWindow::setupUi()
{
    // 创建中央部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // 创建主垂直布局
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // 设置应用程序整体样式表
    setStyleSheet(R"(
        QWidget {
            font-family: -apple-system, BlinkMacSystemFont, 'SF Pro Display', 'Helvetica Neue', sans-serif;
            font-size: 13px;
        }
        QToolButton, QPushButton {
            border: none;
            border-radius: 5px;
            padding: 5px 10px;
            background-color: rgba(240, 240, 240, 0.8);
            color: #333;
        }
        QToolButton:hover, QPushButton:hover {
            background-color: rgba(220, 220, 220, 0.9);
        }
        QToolButton:pressed, QPushButton:pressed {
            background-color: rgba(200, 200, 200, 0.9);
        }
        QComboBox {
            border: 1px solid #ccc;
            border-radius: 5px;
            padding: 4px 8px;
            background-color: white;
            min-height: 24px;
        }
        QComboBox::drop-down {
            border: none;
            width: 20px;
        }
        QComboBox QAbstractItemView {
            border: 1px solid #ccc;
            border-radius: 5px;
            selection-background-color: #f0f0f0;
        }
        QSlider::groove:horizontal {
            height: 4px;
            background: #ddd;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: #007aff;
            border-radius: 7px;
            width: 14px;
            height: 14px;
            margin: -5px 0;
        }
        QSlider::add-page:horizontal {
            background: #ddd;
            border-radius: 2px;
        }
        QSlider::sub-page:horizontal {
            background: #007aff;
            border-radius: 2px;
        }
        QStatusBar {
            background-color: #f5f5f7;
            border-top: 1px solid #e0e0e0;
        }
        QScrollBar:vertical {
            border: none;
            background: transparent;
            width: 8px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: rgba(0, 0, 0, 0.2);
            min-height: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: rgba(0, 0, 0, 0.3);
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            background: none;
            height: 0px;
        }
        QScrollBar:horizontal {
            border: none;
            background: transparent;
            height: 8px;
            margin: 0px;
        }
        QScrollBar::handle:horizontal {
            background: rgba(0, 0, 0, 0.2);
            min-width: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal:hover {
            background: rgba(0, 0, 0, 0.3);
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            background: none;
            width: 0px;
        }
    )");

    // 创建自定义标题栏
    createTitleBar();
    
    // 创建顶部工具栏
    createTopToolbar();
    
    // 创建各个选项卡对应的工具栏
    createMainToolbar();       // 开始
    createArrangeToolbar();    // 排列
    createExportAndImportToolbar();     // 导出和导入
    
    // 默认只显示"开始"选项卡对应的工具栏
    m_arrangeToolbar->hide();
    m_exportAndImportToolbar->hide();

    // 创建内容区域水平布局
    m_contentLayout = new QHBoxLayout();
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);
    m_mainLayout->addLayout(m_contentLayout);

    // 创建工具栏和绘图区域
    m_toolBar = new ToolBar(this);
    m_drawingArea = new DrawingArea(this);

    // 创建滚动区域来容纳绘图区域
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(m_drawingArea);
    scrollArea->setWidgetResizable(false);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #f5f5f7; }");

    // 设置工具栏固定宽度
    m_toolBar->setFixedWidth(249);
    m_toolBar->setStyleSheet("QWidget { background-color: #f5f5f7; border-right: 1px solid #e0e0e0; }");

    // 添加到布局
    m_contentLayout->addWidget(m_toolBar);
    m_contentLayout->addWidget(scrollArea, 1);
    
    // 创建状态栏
    createStatusBar();
}

void MainWindow::createTitleBar()
{
    // 创建自定义标题栏
    QWidget* titleBar = new QWidget(this);
    titleBar->setFixedHeight(30);
    titleBar->setStyleSheet("background-color: #f5f5f7; border-bottom: 1px solid #e5e5e5;");
    
    QHBoxLayout* layout = new QHBoxLayout(titleBar);
    layout->setContentsMargins(10, 0, 10, 0);
    
    // 添加标题标签
    QLabel* titleLabel = new QLabel(tr("Flowchart Designer"), this);
    titleLabel->setStyleSheet("font-size: 13px; font-weight: 500; color: #333;");
    
    // 添加最小化、最大化和关闭按钮 - Apple样式
    QPushButton* closeButton = new QPushButton("", this);
    closeButton->setFixedSize(12, 12);
    closeButton->setStyleSheet(
        "QPushButton { background-color: #fc615d; border-radius: 6px; border: none; }"
        "QPushButton:hover { background-color: #fc615d; background-image: url('data:image/svg+xml;utf8,<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"10\" height=\"10\" viewBox=\"0 0 10 10\"><path d=\"M3.5,6.5 L3.5,6.5 L1.5,4.5 M3.5,4.5 L1.5,6.5 M6.5,6.5 L8.5,4.5 M6.5,4.5 L8.5,6.5\" stroke=\"black\" stroke-width=\"1.15\" fill=\"none\" /></svg>'); }"
    );
    
    QPushButton* minButton = new QPushButton("", this);
    minButton->setFixedSize(12, 12);
    minButton->setStyleSheet(
        "QPushButton { background-color: #fdbc40; border-radius: 6px; border: none; }"
        "QPushButton:hover { background-color: #fdbc40; background-image: url('data:image/svg+xml;utf8,<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"10\" height=\"10\" viewBox=\"0 0 10 10\"><path d=\"M2.5,5 L7.5,5\" stroke=\"black\" stroke-width=\"1.15\" fill=\"none\" /></svg>'); }"
    );
    
    QPushButton* maxButton = new QPushButton("", this);
    maxButton->setFixedSize(12, 12);
    maxButton->setStyleSheet(
        "QPushButton { background-color: #34c749; border-radius: 6px; border: none; }"
        "QPushButton:hover { background-color: #34c749; background-image: url('data:image/svg+xml;utf8,<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"10\" height=\"10\" viewBox=\"0 0 10 10\"><path d=\"M2.5,2.5 L7.5,2.5 L7.5,7.5 L2.5,7.5 L2.5,2.5 Z\" stroke=\"black\" stroke-width=\"1.15\" fill=\"none\" /></svg>'); }"
    );
    
    // 创建一个水平布局来包含这些按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addWidget(closeButton);
    buttonLayout->addWidget(minButton);
    buttonLayout->addWidget(maxButton);
    
    // 添加到布局
    layout->addLayout(buttonLayout);
    layout->addWidget(titleLabel, 0, Qt::AlignCenter);
    layout->addStretch();
    
    // 添加到主布局
    m_mainLayout->addWidget(titleBar);
    
    // 连接按钮信号
    connect(minButton, &QPushButton::clicked, this, &MainWindow::showMinimized);
    connect(maxButton, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::close);
    
    // 使标题栏可拖动
    titleBar->installEventFilter(this);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    static QPoint lastPos;
    
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            lastPos = mouseEvent->globalPos();
        }
        return true;
    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->buttons() & Qt::LeftButton) {
            move(pos() + mouseEvent->globalPos() - lastPos);
            lastPos = mouseEvent->globalPos();
        }
        return true;
    }
    
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::createTopToolbar()
{
    // 创建一个水平布局用于居中标签栏
    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);
    
    // 创建一个容器Widget来设置背景色
    QWidget *containerWidget = new QWidget(this);
    containerWidget->setStyleSheet("background-color: #f5f5f7;");
    containerWidget->setLayout(centerLayout);
    
    // 创建标签栏 - Apple风格
    m_tabBar = new QTabBar(this);
    m_tabBar->addTab(tr("Start"));
    m_tabBar->addTab(tr("Arrange"));
    m_tabBar->addTab(tr("Export and Import"));
    m_tabBar->setExpanding(false);
    m_tabBar->setDocumentMode(true);
    m_tabBar->setDrawBase(false);
    m_tabBar->setStyleSheet(
        "QTabBar { background-color: #f5f5f7; }"
        "QTabBar::tab { padding: 12px 30px; border: none; background-color: #f5f5f7; color: #555; font-weight: 500; min-width: 140px; }"
        "QTabBar::tab:selected { color: #007aff; border-bottom: 2px solid #007aff; }"
        "QTabBar::tab:hover:!selected { color: #333; }"
    );
    
    // 连接标签切换信号
    connect(m_tabBar, &QTabBar::currentChanged, this, &MainWindow::onTabBarClicked);
    
    // 将标签栏放入水平布局中，并添加弹簧使其居中
    centerLayout->addStretch();
    centerLayout->addWidget(m_tabBar);
    centerLayout->addStretch();
    
    // 将容器添加到主布局中
    m_mainLayout->addWidget(containerWidget);
}

void MainWindow::onTabBarClicked(int index)
{
    // 隐藏所有工具栏
    m_mainToolbar->hide();
    m_arrangeToolbar->hide();
    m_exportAndImportToolbar->hide();
    
    // 根据选中的标签显示对应的工具栏
    switch (index) {
    case 0: // 开始
        m_mainToolbar->show();
        break;
    case 1: // 排列
        m_arrangeToolbar->show();
        break;
    case 2: // 导出和导入
        m_exportAndImportToolbar->show();
        break;
    default:
        m_mainToolbar->show(); // 默认显示主工具栏
        break;
    }
}

void MainWindow::createMainToolbar()
{
    // 主工具栏 (对应"开始"选项卡) - Apple风格
    m_mainToolbar = new QToolBar(this);
    m_mainToolbar->setMovable(false);
    m_mainToolbar->setIconSize(QSize(16, 16));
    m_mainToolbar->setStyleSheet(
        "QToolBar { background-color: #f5f5f7; border-bottom: 1px solid #e0e0e0; padding: 6px; min-height: 44px; }"
        "QToolBar QToolButton { padding: 5px 10px; margin: 0 2px; background-color: #f5f5f7; }"
        "QToolBar QToolButton:hover { background-color: #e5e5e5; }"
        "QToolBar QToolButton:disabled { color: rgba(0, 0, 0, 0.25); }"
        "QToolBar QComboBox { margin: 0 2px; background-color: #f5f5f7; }"
        "QToolBar QComboBox:hover { background-color: #e5e5e5; }"
        "QToolBar QComboBox:disabled { color: rgba(0, 0, 0, 0.25); }"
        "QToolBar QPushButton { padding: 5px 10px; margin: 0 2px; background-color: #f5f5f7; }"
        "QToolBar QPushButton:hover { background-color: #e5e5e5; }"
        "QToolBar QPushButton:disabled { color: rgba(0, 0, 0, 0.25); }"
    );
    m_mainLayout->addWidget(m_mainToolbar);
    
    // 添加左侧弹性空间
    QWidget* leftSpacer = new QWidget();
    leftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_mainToolbar->addWidget(leftSpacer);
    
    // 添加字体设置 - Apple风格
    m_fontCombo = new QComboBox();
    m_fontCombo->addItem(tr("San Francisco"));
    m_fontCombo->addItem(tr("微软雅黑"));
    m_fontCombo->addItem(tr("宋体"));
    m_fontCombo->addItems(QFontDatabase().families());
    m_fontCombo->setFixedWidth(120);
    m_fontCombo->setEnabled(false);
    m_mainToolbar->addWidget(m_fontCombo);

    m_fontSizeCombo = new QComboBox();
    for (int i = 9; i <= 18; i++) {
        m_fontSizeCombo->addItem(QString("%1 px").arg(i));
    }
    m_fontSizeCombo->addItem(tr("20 px"));
    m_fontSizeCombo->addItem(tr("24 px"));
    m_fontSizeCombo->addItem(tr("32 px"));
    m_fontSizeCombo->addItem(tr("40 px"));
    m_fontSizeCombo->addItem(tr("64 px"));
    
    m_fontSizeCombo->setFixedWidth(90);
    m_fontSizeCombo->setEnabled(false);
    m_mainToolbar->addWidget(m_fontSizeCombo);
    
    m_mainToolbar->addSeparator();
    
    // 添加常用格式按钮 - Apple风格
    m_boldAction = new QAction(tr("B"), this);
    m_boldAction->setToolTip(tr("Bold"));
    QFont boldFont("SF Pro Text", 13, QFont::Bold);
    m_boldAction->setFont(boldFont);
    m_boldAction->setCheckable(true);
    m_boldAction->setEnabled(false);
    
    m_italicAction = new QAction(tr("I"), this);
    m_italicAction->setToolTip(tr("Italic"));
    QFont italicFont("SF Pro Text", 13);
    italicFont.setItalic(true);
    m_italicAction->setFont(italicFont);
    m_italicAction->setCheckable(true);
    m_italicAction->setEnabled(false);
    
    m_underlineAction = new QAction(tr("U"), this);
    m_underlineAction->setToolTip(tr("Underline"));
    QFont underlineFont("SF Pro Text", 13);
    underlineFont.setUnderline(true);
    m_underlineAction->setFont(underlineFont);
    m_underlineAction->setCheckable(true);
    m_underlineAction->setEnabled(false);
    
    // 直接将这些操作添加到工具栏，而不是使用QToolButton
    m_mainToolbar->addAction(m_boldAction);
    m_mainToolbar->addAction(m_italicAction);
    m_mainToolbar->addAction(m_underlineAction);
    
    m_mainToolbar->addSeparator();
    
    // 添加字体颜色按钮 - Apple风格
    m_fontColorButton = Utils::getAutoChangeColorsButton(this, tr("Font"), 54, 38, 28, 8);
    m_fontColorIndicator = m_fontColorButton->findChild<QFrame*>("colorIndicator");
    m_mainToolbar->addWidget(m_fontColorButton);
    
    // 设置字体颜色按钮的初始状态
    Utils::updateColorButton(m_fontColorButton, QColor(0, 0, 0)); // 默认黑色
    
    m_mainToolbar->addSeparator();

    // 添加对齐设置 - Apple风格
    m_alignCombo = new QComboBox();
    m_alignCombo->addItem(tr("Center"));
    m_alignCombo->addItem(tr("Left"));
    m_alignCombo->addItem(tr("Right"));
    m_alignCombo->setFixedWidth(90);
    m_alignCombo->setEnabled(false);
    m_mainToolbar->addWidget(m_alignCombo);
    
    m_mainToolbar->addSeparator();

    // 创建填充颜色按钮 - Apple风格
    m_fillColorButton = Utils::getAutoChangeColorsButton(this, tr("Fill"), 54, 38, 28, 8);
    m_fillColorIndicator = m_fillColorButton->findChild<QFrame*>("colorIndicator");
    m_mainToolbar->addWidget(m_fillColorButton);
    
    // 连接填充颜色按钮的点击信号
    connect(m_fillColorButton, &QPushButton::clicked, this, &MainWindow::onFillColorButtonClicked);
    
    m_mainToolbar->addSeparator();

    // 添加透明度组合框 - Apple风格
    QLabel* transparencyLabel = new QLabel(tr("Opacity:"));
    transparencyLabel->setStyleSheet("color: #555;");
    m_mainToolbar->addWidget(transparencyLabel);
    
    m_transparencyCombo = new QComboBox();
    for (int i = 0; i <= 100; i += 10) {
        m_transparencyCombo->addItem(QString("%1%").arg(i));
    }
    m_transparencyCombo->setCurrentIndex(10);
    m_transparencyCombo->setFixedWidth(70);
    m_transparencyCombo->setEnabled(false);
    m_mainToolbar->addWidget(m_transparencyCombo);
    
    m_mainToolbar->addSeparator();
    
    // 创建线条颜色按钮 - Apple风格
    m_lineColorButton = Utils::getAutoChangeColorsButton(this, tr("Line"), 54, 38, 28, 8);
    m_lineColorIndicator = m_lineColorButton->findChild<QFrame*>("colorIndicator");
    m_mainToolbar->addWidget(m_lineColorButton);
    
    // 连接线条颜色按钮的点击信号
    connect(m_lineColorButton, &QPushButton::clicked, this, &MainWindow::onLineColorButtonClicked);
    
    m_mainToolbar->addSeparator();

    // 添加线条粗细下拉框 - Apple风格
    QLabel* lineWidthLabel = new QLabel(tr("Width:"));
    lineWidthLabel->setStyleSheet("color: #555;");
    m_mainToolbar->addWidget(lineWidthLabel);
    
    m_lineWidthCombo = new QComboBox();
    m_lineWidthCombo->addItem(tr("0px"));
    m_lineWidthCombo->addItem(tr("0.5px"));
    m_lineWidthCombo->addItem(tr("1px"));
    m_lineWidthCombo->addItem(tr("1.5px"));
    m_lineWidthCombo->addItem(tr("2px"));
    m_lineWidthCombo->addItem(tr("3px"));
    m_lineWidthCombo->addItem(tr("4px"));
    m_lineWidthCombo->addItem(tr("5px"));
    m_lineWidthCombo->addItem(tr("6px"));
    m_lineWidthCombo->addItem(tr("8px"));
    m_lineWidthCombo->addItem(tr("10px"));
    
    m_lineWidthCombo->setCurrentIndex(3);
    m_lineWidthCombo->setFixedWidth(70);
    m_lineWidthCombo->setEnabled(false);
    m_mainToolbar->addWidget(m_lineWidthCombo);
    
    // 添加线条样式下拉框 - Apple风格
    QLabel* lineStyleLabel = new QLabel(tr("Style:"));
    lineStyleLabel->setStyleSheet("color: #555;");
    m_mainToolbar->addWidget(lineStyleLabel);
    
    m_lineStyleCombo = new QComboBox();
    m_lineStyleCombo->addItem(tr("Solid"));
    m_lineStyleCombo->addItem(tr("Dash S"));
    m_lineStyleCombo->addItem(tr("Dash L"));
    m_lineStyleCombo->addItem(tr("Dash"));
    
    m_lineStyleCombo->setCurrentIndex(0);
    m_lineStyleCombo->setFixedWidth(70);
    m_lineStyleCombo->setEnabled(false);
    m_mainToolbar->addWidget(m_lineStyleCombo);
    
    // 添加页面设置按钮 - Apple风格
    m_pageSettingButton = new QPushButton(tr("Page Setup"));
    m_pageSettingButton->setToolTip(tr("Page Setup"));
    m_pageSettingButton->setFixedHeight(30);
    m_pageSettingButton->setMinimumWidth(100);
    m_pageSettingButton->setStyleSheet(
        "QPushButton { padding-left: 8px; padding-right: 8px; border-radius: 4px; }"
        "QPushButton { background-color: #007aff; color: white; }"
        "QPushButton:hover { background-color: #0069d9; }"
        "QPushButton:pressed { background-color: #0062cc; }"
        "QPushButton:disabled { background-color: rgba(0, 122, 255, 0.5); color: rgba(255, 255, 255, 0.5); }"
    );
    m_mainToolbar->addWidget(m_pageSettingButton);
    
    // 添加右侧弹性空间
    QWidget* rightSpacer = new QWidget();
    rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_mainToolbar->addWidget(rightSpacer);

    // 连接页面设置按钮的点击信号
    connect(m_pageSettingButton, &QPushButton::clicked, this, &MainWindow::showPageSettingDialog);
    
    // 连接字体控件的信号
    connect(m_fontCombo, &QComboBox::currentTextChanged, this, &MainWindow::onFontFamilyChanged);
    connect(m_fontSizeCombo, &QComboBox::currentTextChanged, this, &MainWindow::onFontSizeChanged);
    connect(m_boldAction, &QAction::triggered, this, &MainWindow::onBoldActionTriggered);
    connect(m_italicAction, &QAction::triggered, this, &MainWindow::onItalicActionTriggered);
    connect(m_underlineAction, &QAction::triggered, this, &MainWindow::onUnderlineActionTriggered);
    connect(m_fontColorButton, &QPushButton::clicked, this, &MainWindow::onFontColorButtonClicked);
    connect(m_alignCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onAlignmentChanged);
    connect(m_transparencyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onTransparencyChanged);
    connect(m_lineWidthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onLineWidthChanged);
    connect(m_lineStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onLineStyleChanged);
}

void MainWindow::createArrangeToolbar()
{
    // 排列工具栏 (对应"排列"选项卡) - Apple风格
    m_arrangeToolbar = new QToolBar(this);
    m_arrangeToolbar->setMovable(false);
    m_arrangeToolbar->setIconSize(QSize(16, 16));
    m_arrangeToolbar->setStyleSheet(
        "QToolBar { background-color: #f5f5f7; border-bottom: 1px solid #e0e0e0; padding: 6px; min-height: 44px; }"
        "QToolBar QToolButton { padding: 5px 10px; margin: 0 2px; background-color: #f5f5f7; }"
        "QToolBar QToolButton:hover { background-color: #e5e5e5; }"
        "QToolBar QToolButton:disabled { color: rgba(0, 0, 0, 0.25); }"
        "QToolBar QPushButton { padding: 5px 10px; margin: 0 2px; background-color: #f5f5f7; }"
        "QToolBar QPushButton:hover { background-color: #e5e5e5; }"
        "QToolBar QPushButton:disabled { color: rgba(0, 0, 0, 0.25); }"
    );
    m_mainLayout->addWidget(m_arrangeToolbar);
    
    // 添加左侧弹性空间
    QWidget* leftSpacer = new QWidget();
    leftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_arrangeToolbar->addWidget(leftSpacer);
    
    // 创建按钮并设置Apple风格
    // 置于顶层按钮
    m_bringToFrontButton = new QPushButton(tr("Move Shape To Top"));
    m_bringToFrontButton->setIcon(QIcon::fromTheme("go-top"));
    m_bringToFrontButton->setFixedHeight(30);
    m_bringToFrontButton->setStyleSheet(
        "QPushButton { padding-left: 8px; padding-right: 8px; border-radius: 4px; background-color: #f5f5f7; }"
        "QPushButton:hover { background-color: #e5e5e5; }"
        "QPushButton:disabled { color: rgba(0, 0, 0, 0.25); }"
    );
    m_arrangeToolbar->addWidget(m_bringToFrontButton);
    
    m_arrangeToolbar->addSeparator();
    
    // 置于底层按钮
    m_sendToBackButton = new QPushButton(tr("Move Shape To Bottom"));
    m_sendToBackButton->setIcon(QIcon::fromTheme("go-bottom"));
    m_sendToBackButton->setFixedHeight(30);
    m_sendToBackButton->setStyleSheet(
        "QPushButton { padding-left: 8px; padding-right: 8px; border-radius: 4px; background-color: #f5f5f7; }"
        "QPushButton:hover { background-color: #e5e5e5; }"
        "QPushButton:disabled { color: rgba(0, 0, 0, 0.25); }"
    );
    m_arrangeToolbar->addWidget(m_sendToBackButton);
    
    m_arrangeToolbar->addSeparator();
    
    // 上移一层按钮
    m_bringForwardButton = new QPushButton(tr("Move Shape Up"));
    m_bringForwardButton->setIcon(QIcon::fromTheme("go-up"));
    m_bringForwardButton->setFixedHeight(30);
    m_bringForwardButton->setStyleSheet(
        "QPushButton { padding-left: 8px; padding-right: 8px; border-radius: 4px; background-color: #f5f5f7; }"
        "QPushButton:hover { background-color: #e5e5e5; }"
        "QPushButton:disabled { color: rgba(0, 0, 0, 0.25); }"
    );
    m_arrangeToolbar->addWidget(m_bringForwardButton);
    
    m_arrangeToolbar->addSeparator();
    
    // 下移一层按钮
    m_sendBackwardButton = new QPushButton(tr("Move Shape Down"));
    m_sendBackwardButton->setIcon(QIcon::fromTheme("go-down"));
    m_sendBackwardButton->setFixedHeight(30);
    m_sendBackwardButton->setStyleSheet(
        "QPushButton { padding-left: 8px; padding-right: 8px; border-radius: 4px; background-color: #f5f5f7; }"
        "QPushButton:hover { background-color: #e5e5e5; }"
        "QPushButton:disabled { color: rgba(0, 0, 0, 0.25); }"
    );
    m_arrangeToolbar->addWidget(m_sendBackwardButton);
    
    // 添加右侧弹性空间
    QWidget* rightSpacer = new QWidget();
    rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_arrangeToolbar->addWidget(rightSpacer);
    
    // 添加按钮信号连接
    connect(m_bringToFrontButton, &QPushButton::clicked, m_drawingArea, &DrawingArea::moveShapeToTop);
    connect(m_sendToBackButton, &QPushButton::clicked, m_drawingArea, &DrawingArea::moveShapeToBottom);
    connect(m_bringForwardButton, &QPushButton::clicked, m_drawingArea, &DrawingArea::moveShapeUp);
    connect(m_sendBackwardButton, &QPushButton::clicked, m_drawingArea, &DrawingArea::moveShapeDown);
    
    // 初始化按钮状态为禁用
    m_bringToFrontButton->setEnabled(false);
    m_sendToBackButton->setEnabled(false);
    m_bringForwardButton->setEnabled(false);
    m_sendBackwardButton->setEnabled(false);
}

void MainWindow::createExportAndImportToolbar()
{
    // 导出工具栏 (对应"导出"选项卡) - Apple风格
    m_exportAndImportToolbar = new QToolBar(this);
    m_exportAndImportToolbar->setMovable(false);
    m_exportAndImportToolbar->setIconSize(QSize(16, 16));
    m_exportAndImportToolbar->setStyleSheet(
        "QToolBar { background-color: #f5f5f7; border-bottom: 1px solid #e0e0e0; padding: 6px; min-height: 44px; }"
        "QToolBar QToolButton { padding: 5px 10px; margin: 0 2px; background-color: #f5f5f7; }"
        "QToolBar QToolButton:hover { background-color: #e5e5e5; }"
        "QToolBar QToolButton:disabled { color: rgba(0, 0, 0, 0.25); }"
        "QToolBar QPushButton { padding: 5px 10px; margin: 0 2px; background-color: #f5f5f7; }"
        "QToolBar QPushButton:hover { background-color: #e5e5e5; }"
        "QToolBar QPushButton:disabled { color: rgba(0, 0, 0, 0.25); }"
    );
    m_mainLayout->addWidget(m_exportAndImportToolbar);
    
    // 添加左侧弹性空间
    QWidget* leftSpacer = new QWidget();
    leftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_exportAndImportToolbar->addWidget(leftSpacer);
    
    // 添加导出工具栏的按钮和控件 - Apple风格
    QPushButton* exportAsPngButton = new QPushButton(tr("Export PNG"));
    exportAsPngButton->setIcon(QIcon::fromTheme("image-x-generic"));
    exportAsPngButton->setFixedHeight(30);
    exportAsPngButton->setStyleSheet(
        "QPushButton { padding-left: 8px; padding-right: 8px; border-radius: 4px; background-color: #f5f5f7; }"
        "QPushButton:hover { background-color: #e5e5e5; }"
        "QPushButton:disabled { color: rgba(0, 0, 0, 0.25); }"
    );
    
    QPushButton* exportAsPdfButton = new QPushButton(tr("Export PDF"));
    exportAsPdfButton->setIcon(QIcon::fromTheme("application-pdf"));
    exportAsPdfButton->setFixedHeight(30);
    exportAsPdfButton->setStyleSheet(
        "QPushButton { padding-left: 8px; padding-right: 8px; border-radius: 4px; background-color: #f5f5f7; }"
        "QPushButton:hover { background-color: #e5e5e5; }"
        "QPushButton:disabled { color: rgba(0, 0, 0, 0.25); }"
    );
    
    QPushButton* exportAsSvgButton = new QPushButton(tr("Export SVG"));
    exportAsSvgButton->setIcon(QIcon::fromTheme("image-svg+xml"));
    exportAsSvgButton->setFixedHeight(30);
    exportAsSvgButton->setStyleSheet(
        "QPushButton { padding-left: 8px; padding-right: 8px; border-radius: 4px; background-color: #f5f5f7; }"
        "QPushButton:hover { background-color: #e5e5e5; }"
        "QPushButton:disabled { color: rgba(0, 0, 0, 0.25); }"
    );

    m_exportAndImportToolbar->addWidget(exportAsPngButton);
    m_exportAndImportToolbar->addSeparator();
    m_exportAndImportToolbar->addWidget(exportAsPdfButton);
    m_exportAndImportToolbar->addSeparator();
    m_exportAndImportToolbar->addWidget(exportAsSvgButton);
    m_exportAndImportToolbar->addSeparator();

    // 添加导入工具栏的按钮和控件 - Apple风格
    QPushButton* importFromSvgButton = new QPushButton(tr("Import SVG"));
    importFromSvgButton->setIcon(QIcon::fromTheme("document-open"));
    importFromSvgButton->setFixedHeight(30);
    importFromSvgButton->setStyleSheet(
        "QPushButton { padding-left: 8px; padding-right: 8px; border-radius: 4px; }"
        "QPushButton { background-color: #007aff; color: white; }"
        "QPushButton:hover { background-color: #0069d9; }"
        "QPushButton:pressed { background-color: #0062cc; }"
        "QPushButton:disabled { background-color: rgba(0, 122, 255, 0.5); color: rgba(255, 255, 255, 0.5); }"
    );
    m_exportAndImportToolbar->addWidget(importFromSvgButton);
    
    // 添加右侧弹性空间
    QWidget* rightSpacer = new QWidget();
    rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_exportAndImportToolbar->addWidget(rightSpacer);

    // 连接导出按钮的信号到相应的槽函数
    connect(exportAsPngButton, &QPushButton::clicked, this, &MainWindow::exportAsPng);
    connect(exportAsSvgButton, &QPushButton::clicked, this, &MainWindow::exportAsSvg);
    connect(importFromSvgButton, &QPushButton::clicked, this, &MainWindow::importFromSvg);
}

void MainWindow::showPageSettingDialog()
{
    // 如果对话框不存在，则创建
    if (!m_pageSettingDialog) {
        m_pageSettingDialog = new PageSettingDialog(this, m_drawingArea);
        
        // 设置对话框样式为Apple风格
        m_pageSettingDialog->setStyleSheet(
            "QDialog { background-color: #f5f5f7; }"
            "QLabel { font-size: 13px; }"
            "QPushButton { border-radius: 4px; padding: 6px 12px; }"
            "QPushButton#okButton, QPushButton#applyButton { background-color: #007aff; color: white; }"
            "QPushButton#okButton:hover, QPushButton#applyButton:hover { background-color: #0069d9; }"
            "QPushButton#okButton:pressed, QPushButton#applyButton:pressed { background-color: #0062cc; }"
            "QPushButton#cancelButton { background-color: #f5f5f7; border: 1px solid #ccc; }"
            "QPushButton#cancelButton:hover { background-color: #e5e5e5; }"
            "QComboBox, QSpinBox { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }"
            "QCheckBox { spacing: 8px; }"
            "QCheckBox::indicator { width: 16px; height: 16px; }"
        );
        
        // 连接应用信号到相应的槽
        connect(m_pageSettingDialog, &PageSettingDialog::settingsApplied, 
                this, &MainWindow::applyPageSettings);
    }
    
    // 设置对话框的初始值为绘图区域当前的设置
    m_pageSettingDialog->setModal(true);
    m_pageSettingDialog->exec();
}

void MainWindow::applyPageSettings()
{
    if (!m_pageSettingDialog || !m_drawingArea) return;
    
    // 从对话框获取设置并应用到绘图区域
    m_drawingArea->setBackgroundColor(m_pageSettingDialog->getBackgroundColor());
    m_drawingArea->setPageSize(m_pageSettingDialog->getPageSize());
    m_drawingArea->setShowGrid(m_pageSettingDialog->getShowGrid());
    m_drawingArea->setGridColor(m_pageSettingDialog->getGridColor());
    m_drawingArea->setGridSize(m_pageSettingDialog->getGridSize());
    m_drawingArea->setGridThickness(m_pageSettingDialog->getGridThickness());
    
    // 应用设置后更新绘图区域
    m_drawingArea->applyPageSettings();
}

// 实现字体相关的槽函数
void MainWindow::updateFontControls()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    
    // 启用或禁用控件，取决于是否有选中的图形
    bool hasSelection = (selectedShape != nullptr);
    m_fontCombo->setEnabled(hasSelection);
    m_fontSizeCombo->setEnabled(hasSelection);
    m_boldAction->setEnabled(hasSelection);
    m_italicAction->setEnabled(hasSelection);
    m_underlineAction->setEnabled(hasSelection);
    m_fontColorButton->setEnabled(hasSelection);
    m_alignCombo->setEnabled(hasSelection);
    
    // 也更新填充颜色和线条颜色按钮的状态
    m_fillColorButton->setEnabled(hasSelection);
    m_lineColorButton->setEnabled(hasSelection);
    
    // 如果有选中的图形，更新指示器显示选中图形的颜色
    // 如果没有选中图形，将指示器颜色重置为白色
    if (hasSelection) {
        // 更新所有颜色按钮
        updateColorButtons();
    } else {
        // 无选中图形时重置为白色
        Utils::updateColorButton(m_fontColorButton, QColor(255, 255, 255));
        Utils::updateColorButton(m_fillColorButton, QColor(255, 255, 255));
        Utils::updateColorButton(m_lineColorButton, QColor(255, 255, 255));
    }
    
    // 也更新透明度、线条粗细和线条样式下拉框的状态
    m_transparencyCombo->setEnabled(hasSelection);
    m_lineWidthCombo->setEnabled(hasSelection);
    m_lineStyleCombo->setEnabled(hasSelection);
    
    // 如果有选中图形，更新控件显示当前设置
    if (hasSelection) {
        // 更新字体类型下拉框
        QString fontFamily = selectedShape->fontFamily();
        int fontFamilyIndex = m_fontCombo->findText(fontFamily);
        if (fontFamilyIndex >= 0) {
            m_fontCombo->setCurrentIndex(fontFamilyIndex);
        }
        
        // 更新字体大小下拉框
        int fontSize = selectedShape->fontSize();
        int fontSizeIndex = m_fontSizeCombo->findText(QString("%1 px").arg(fontSize));
        if (fontSizeIndex >= 0) {
            m_fontSizeCombo->setCurrentIndex(fontSizeIndex);
        }
        
        // 更新粗体、斜体、下划线按钮状态
        m_boldAction->setChecked(selectedShape->isFontBold());
        m_italicAction->setChecked(selectedShape->isFontItalic());
        m_underlineAction->setChecked(selectedShape->isFontUnderline());
        
        // 更新对齐方式下拉框
        Qt::Alignment alignment = selectedShape->textAlignment();
        if (alignment & Qt::AlignCenter) {
            m_alignCombo->setCurrentIndex(0);
        } else if (alignment & Qt::AlignLeft) {
            m_alignCombo->setCurrentIndex(1);
        } else if (alignment & Qt::AlignRight) {
            m_alignCombo->setCurrentIndex(2);
        }
        
        // 更新透明度组合框
        int transparency = selectedShape->transparency();
        int transparencyIndex = transparency / 10; // 0-10之间的索引
        m_transparencyCombo->setCurrentIndex(transparencyIndex);
        
        // 更新线条粗细下拉框
        qreal lineWidth = selectedShape->lineWidth();
        int lineWidthIndex = 0;
        
        // 查找最接近的线条粗细选项
        if (lineWidth <= 0) lineWidthIndex = 0;
        else if (lineWidth <= 0.5) lineWidthIndex = 1;
        else if (lineWidth <= 1) lineWidthIndex = 2;
        else if (lineWidth <= 1.5) lineWidthIndex = 3;
        else if (lineWidth <= 2) lineWidthIndex = 4;
        else if (lineWidth <= 3) lineWidthIndex = 5;
        else if (lineWidth <= 4) lineWidthIndex = 6;
        else if (lineWidth <= 5) lineWidthIndex = 7;
        else if (lineWidth <= 6) lineWidthIndex = 8;
        else if (lineWidth <= 8) lineWidthIndex = 9;
        else lineWidthIndex = 10;
        
        m_lineWidthCombo->setCurrentIndex(lineWidthIndex);
        
        // 更新线条样式下拉框
        int lineStyle = selectedShape->lineStyle();
        m_lineStyleCombo->setCurrentIndex(lineStyle);
    }
}

void MainWindow::onFontFamilyChanged(const QString& family)
{
    // 应用新的字体族到选中的图形
    m_drawingArea->setSelectedShapeFontFamily(family);
}

void MainWindow::onFontSizeChanged(const QString& sizeText)
{
    // 从字符串中提取字体大小数值（去掉"px"后缀）
    bool ok;
    int size = sizeText.split(" ").first().toInt(&ok);
    if (ok) {
        // 应用新的字体大小到选中的图形
        m_drawingArea->setSelectedShapeFontSize(size);
    }
}

void MainWindow::onBoldActionTriggered()
{
    // 应用加粗状态到选中的图形
    m_drawingArea->setSelectedShapeFontBold(m_boldAction->isChecked());
}

void MainWindow::onItalicActionTriggered()
{
    // 应用斜体状态到选中的图形
    m_drawingArea->setSelectedShapeFontItalic(m_italicAction->isChecked());
}

void MainWindow::onUnderlineActionTriggered()
{
    // 应用下划线状态到选中的图形
    m_drawingArea->setSelectedShapeFontUnderline(m_underlineAction->isChecked());
}

void MainWindow::onFontColorButtonClicked()
{
    // 打开颜色选择对话框，应用新的字体颜色到选中的图形
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        QColor initialColor = selectedShape->fontColor();
        QColor color = QColorDialog::getColor(initialColor, this, tr("选择字体颜色"));
        
        if (color.isValid()) {
            m_drawingArea->setSelectedShapeFontColor(color);
            // 手动更新颜色指示器
            Utils::updateColorButton(m_fontColorButton, color);
        }
    }
}

void MainWindow::onAlignmentChanged(int index)
{
    // 根据索引设置不同的对齐方式
    Qt::Alignment alignment;
    switch (index) {
    case 0:  // 居中对齐
        alignment = Qt::AlignCenter;
        break;
    case 1:  // 左对齐，并垂直居中
        alignment = Qt::AlignLeft | Qt::AlignVCenter; 
        break;
    case 2:  // 右对齐
        alignment = Qt::AlignRight | Qt::AlignVCenter;
        break;
    default:
        alignment = Qt::AlignCenter;
        break;
    }
    
    // 应用对齐设置到选中的图形
    m_drawingArea->setSelectedShapeTextAlignment(alignment);
}

void MainWindow::updateFontColorButton(const QColor& color)
{
    // 如果没有选中图形，直接更新按钮颜色
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (!selectedShape) {
        Utils::updateColorButton(m_fontColorButton, color);
        return;
    }
    
    // 如果有选中图形，调用updateColorButtons来统一处理所有颜色按钮
    updateColorButtons();
}

void MainWindow::exportAsPng()
{
    if (!m_drawingArea) return;
    
    // 获取当前日期时间，格式化为字符串作为默认文件名
    QString defaultFileName = QString("FlowChart_%1").arg(
        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    
    // 打开文件保存对话框 - Apple风格
    QFileDialog dialog(this, tr("Export as PNG"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("PNG Images (*.png)"));
    dialog.setDirectory(QDir::homePath());
    dialog.selectFile(defaultFileName);
    dialog.setDefaultSuffix("png");
    
    // 设置对话框样式
    dialog.setStyleSheet(
        "QFileDialog { background-color: #f5f5f7; }"
        "QPushButton { border-radius: 4px; padding: 6px 12px; }"
        "QPushButton[text=\"Save\"], QPushButton[text=\"OK\"] { background-color: #007aff; color: white; }"
        "QPushButton[text=\"Cancel\"] { background-color: #f5f5f7; border: 1px solid #ccc; }"
        "QComboBox, QLineEdit { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }"
    );
    
    // 如果用户取消了对话框，则返回
    if (!dialog.exec()) {
        return;
    }
    
    // 获取选择的文件路径
    QString filePath = dialog.selectedFiles().first();
    
    // 确保文件路径以.png结尾
    if (!filePath.endsWith(".png", Qt::CaseInsensitive)) {
        filePath += ".png";
    }
    
    // 调用绘图区域的导出方法
    bool success = m_drawingArea->exportToPng(filePath);
    
    // 显示导出结果提示 - Apple风格
    if (success) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Export Successful"));
        msgBox.setText(tr("Flowchart has been exported to PNG image successfully!"));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStyleSheet(
            "QMessageBox { background-color: #f5f5f7; }"
            "QLabel { font-size: 13px; min-width: 300px; }"
            "QPushButton { border-radius: 4px; padding: 6px 12px; min-width: 80px; }"
            "QPushButton { background-color: #007aff; color: white; }"
            "QPushButton:hover { background-color: #0069d9; }"
            "QPushButton:pressed { background-color: #0062cc; }"
        );
        msgBox.exec();
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Export Failed"));
        msgBox.setText(tr("An error occurred while exporting the PNG image. Please check the file path and permissions."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStyleSheet(
            "QMessageBox { background-color: #f5f5f7; }"
            "QLabel { font-size: 13px; min-width: 300px; }"
            "QPushButton { border-radius: 4px; padding: 6px 12px; min-width: 80px; }"
            "QPushButton { background-color: #007aff; color: white; }"
            "QPushButton:hover { background-color: #0069d9; }"
            "QPushButton:pressed { background-color: #0062cc; }"
        );
        msgBox.exec();
    }
}

void MainWindow::exportAsSvg()
{
    if (!m_drawingArea) return;
    
    // 获取当前日期时间，格式化为字符串作为默认文件名
    QString defaultFileName = QString("FlowChart_%1").arg(
        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    
    // 打开文件保存对话框 - Apple风格
    QFileDialog dialog(this, tr("Export as SVG"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("SVG Files (*.svg)"));
    dialog.setDirectory(QDir::homePath());
    dialog.selectFile(defaultFileName);
    dialog.setDefaultSuffix("svg");
    
    // 设置对话框样式
    dialog.setStyleSheet(
        "QFileDialog { background-color: #f5f5f7; }"
        "QPushButton { border-radius: 4px; padding: 6px 12px; }"
        "QPushButton[text=\"Save\"], QPushButton[text=\"OK\"] { background-color: #007aff; color: white; }"
        "QPushButton[text=\"Cancel\"] { background-color: #f5f5f7; border: 1px solid #ccc; }"
        "QComboBox, QLineEdit { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }"
    );
    
    // 如果用户取消了对话框，则返回
    if (!dialog.exec()) {
        return;
    }
    
    // 获取选择的文件路径
    QString filePath = dialog.selectedFiles().first();
    
    // 确保文件路径以.svg结尾
    if (!filePath.endsWith(".svg", Qt::CaseInsensitive)) {
        filePath += ".svg";
    }
    
    // 调用绘图区域的导出方法
    bool success = m_drawingArea->exportToSvg(filePath);
    
    // 显示导出结果提示 - Apple风格
    if (success) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Export Successful"));
        msgBox.setText(tr("Flowchart has been exported to SVG vector image successfully!"));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStyleSheet(
            "QMessageBox { background-color: #f5f5f7; }"
            "QLabel { font-size: 13px; min-width: 300px; }"
            "QPushButton { border-radius: 4px; padding: 6px 12px; min-width: 80px; }"
            "QPushButton { background-color: #007aff; color: white; }"
            "QPushButton:hover { background-color: #0069d9; }"
            "QPushButton:pressed { background-color: #0062cc; }"
        );
        msgBox.exec();
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Export Failed"));
        msgBox.setText(tr("An error occurred while exporting the SVG file. Please check the file path and permissions."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStyleSheet(
            "QMessageBox { background-color: #f5f5f7; }"
            "QLabel { font-size: 13px; min-width: 300px; }"
            "QPushButton { border-radius: 4px; padding: 6px 12px; min-width: 80px; }"
            "QPushButton { background-color: #007aff; color: white; }"
            "QPushButton:hover { background-color: #0069d9; }"
            "QPushButton:pressed { background-color: #0062cc; }"
        );
        msgBox.exec();
    }
}

void MainWindow::importFromSvg()
{
    if (!m_drawingArea) return;
    
    // 打开文件选择对话框 - Apple风格
    QFileDialog dialog(this, tr("Import from SVG"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("SVG Files (*.svg)"));
    dialog.setDirectory(QDir::homePath());
    
    // 设置对话框样式
    dialog.setStyleSheet(
        "QFileDialog { background-color: #f5f5f7; }"
        "QPushButton { border-radius: 4px; padding: 6px 12px; }"
        "QPushButton[text=\"Open\"], QPushButton[text=\"OK\"] { background-color: #007aff; color: white; }"
        "QPushButton[text=\"Cancel\"] { background-color: #f5f5f7; border: 1px solid #ccc; }"
        "QComboBox, QLineEdit { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }"
    );
    
    // 如果用户取消了对话框，则返回
    if (!dialog.exec()) {
        return;
    }
    
    // 获取选择的文件路径
    QString filePath = dialog.selectedFiles().first();
    
    // 提示用户确认是否要导入 - Apple风格
    QMessageBox confirmBox;
    confirmBox.setWindowTitle(tr("Confirm Import"));
    confirmBox.setText(tr("Importing SVG will clear all content on the current canvas. Do you want to continue?"));
    confirmBox.setIcon(QMessageBox::Question);
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.setDefaultButton(QMessageBox::No);
    confirmBox.setStyleSheet(
        "QMessageBox { background-color: #f5f5f7; }"
        "QLabel { font-size: 13px; min-width: 300px; }"
        "QPushButton { border-radius: 4px; padding: 6px 12px; min-width: 80px; }"
        "QPushButton[text=\"Yes\"] { background-color: #007aff; color: white; }"
        "QPushButton[text=\"No\"] { background-color: #f5f5f7; border: 1px solid #ccc; }"
    );
    
    if (confirmBox.exec() == QMessageBox::No) {
        return;
    }
    
    // 调用绘图区域的导入方法
    bool success = m_drawingArea->importFromSvg(filePath);
    
    // 显示导入结果提示 - Apple风格
    if (success) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Import Successful"));
        msgBox.setText(tr("SVG file has been imported successfully!"));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStyleSheet(
            "QMessageBox { background-color: #f5f5f7; }"
            "QLabel { font-size: 13px; min-width: 300px; }"
            "QPushButton { border-radius: 4px; padding: 6px 12px; min-width: 80px; }"
            "QPushButton { background-color: #007aff; color: white; }"
            "QPushButton:hover { background-color: #0069d9; }"
            "QPushButton:pressed { background-color: #0062cc; }"
        );
        msgBox.exec();
    } else {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Import Failed"));
        msgBox.setText(tr("An error occurred while importing the SVG file. Please ensure the file format is correct."));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setStyleSheet(
            "QMessageBox { background-color: #f5f5f7; }"
            "QLabel { font-size: 13px; min-width: 300px; }"
            "QPushButton { border-radius: 4px; padding: 6px 12px; min-width: 80px; }"
            "QPushButton { background-color: #007aff; color: white; }"
            "QPushButton:hover { background-color: #0069d9; }"
            "QPushButton:pressed { background-color: #0062cc; }"
        );
        msgBox.exec();
    }
}

void MainWindow::createStatusBar()
{
    // 创建状态栏 - Apple风格
    m_statusBar = new QStatusBar(this);
    m_statusBar->setStyleSheet(
        "QStatusBar { background-color: #f5f5f7; border-top: 1px solid #e0e0e0; padding: 3px; }"
        "QStatusBar QLabel { color: #555; }"
    );
    setStatusBar(m_statusBar);
    
    // 创建显示图形数量的标签
    m_shapesCountLabel = new QLabel(tr("Number of shapes: 0"));
    m_shapesCountLabel->setStyleSheet("font-size: 12px;");
    m_statusBar->addWidget(m_shapesCountLabel);
    
    // 添加一个固定宽度的空白区域
    QWidget* spacer = new QWidget();
    spacer->setFixedWidth(20);
    m_statusBar->addWidget(spacer);
    
    // 创建显示缩放比例的标签
    m_zoomLabel = new QLabel(tr("Zoom: 100%"));
    m_zoomLabel->setStyleSheet("font-size: 12px;");
    m_statusBar->addWidget(m_zoomLabel);
    
    // 添加一个固定宽度的空白区域
    QWidget* sliderSpacer = new QWidget();
    sliderSpacer->setFixedWidth(10);
    m_statusBar->addWidget(sliderSpacer);
    
    // 创建缩放滑块 - Apple风格
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setMinimumWidth(150);
    m_zoomSlider->setMaximumWidth(200);
    m_zoomSlider->setMinimum(0);
    m_zoomSlider->setMaximum(100);
    m_zoomSlider->setTickInterval(10);
    m_zoomSlider->setTickPosition(QSlider::TicksBelow);
    m_statusBar->addWidget(m_zoomSlider);
    
    // 连接缩放滑块值改变信号
    connect(m_zoomSlider, &QSlider::valueChanged, this, &MainWindow::onZoomSliderValueChanged);
}

void MainWindow::updateStatusBarInfo()
{
    // 更新图形数量信息
    if (m_drawingArea) {
        // 获取图形数量 (包括所有图形和连接线)
        int shapeCount = m_drawingArea->getShapesCount();
        
        // 更新图形数量标签
        m_shapesCountLabel->setText(tr("Number of shapes: %1").arg(shapeCount));
        
        // 更新缩放比例标签
        double zoomPercent = m_drawingArea->getScale() * 100.0;
        m_zoomLabel->setText(tr("Zoom level: %1%").arg(zoomPercent, 0, 'f', 1));
    }
}

void MainWindow::updateZoomSlider()
{
    if (m_drawingArea && m_zoomSlider) {
        // 获取 DrawingArea 的当前缩放比例
        qreal currentScale = m_drawingArea->getScale();
        qreal minScale = m_drawingArea->MIN_SCALE;
        qreal maxScale = m_drawingArea->MAX_SCALE;
        
        // 计算滑块位置 (将缩放比例映射到滑块范围)
        // 滑块范围: 0 - 100
        int sliderPos = static_cast<int>((currentScale - minScale) / (maxScale - minScale) * 100);
        
        // 更新滑块位置 (阻断信号以避免无限循环)
        m_zoomSlider->blockSignals(true);
        m_zoomSlider->setValue(sliderPos);
        m_zoomSlider->blockSignals(false);
        
        // 更新缩放比例标签
        double zoomPercent = currentScale * 100.0;
        m_zoomLabel->setText(tr("Zoom level: %1%").arg(zoomPercent, 0, 'f', 1));
    }
}

void MainWindow::onZoomSliderValueChanged(int value)
{
    if (m_drawingArea) {
        // 获取 DrawingArea 的缩放范围
        qreal minScale = m_drawingArea->MIN_SCALE;
        qreal maxScale = m_drawingArea->MAX_SCALE;
        
        // 将滑块值映射回缩放比例
        qreal newScale = minScale + (value / 100.0) * (maxScale - minScale);
        
        // 设置 DrawingArea 的缩放比例
        m_drawingArea->setScale(newScale);
        
        // 更新缩放比例标签
        double zoomPercent = newScale * 100.0;
        m_zoomLabel->setText(tr("Zoom level: %1%").arg(zoomPercent, 0, 'f', 1));
    }
}

// 填充颜色按钮的点击槽函数
void MainWindow::onFillColorButtonClicked()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        QColor initialColor = selectedShape->fillColor();
        QColor color = QColorDialog::getColor(initialColor, this, tr("选择填充颜色"));
        
        if (color.isValid()) {
            m_drawingArea->setSelectedShapeFillColor(color);
            // 手动更新颜色指示器
            Utils::updateColorButton(m_fillColorButton, color);
        }
    }
}

// 线条颜色按钮的点击槽函数
void MainWindow::onLineColorButtonClicked()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        QColor initialColor = selectedShape->lineColor();
        QColor color = QColorDialog::getColor(initialColor, this, tr("选择线条颜色"));
        
        if (color.isValid()) {
            m_drawingArea->setSelectedShapeLineColor(color);
            // 手动更新颜色指示器
            Utils::updateColorButton(m_lineColorButton, color);
        }
    }
}

// 更新颜色按钮的显示
void MainWindow::updateColorButtons()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (!selectedShape) {
        return;
    }
    
    // 更新字体颜色按钮
    QColor fontColor = selectedShape->fontColor();
    Utils::updateColorButton(m_fontColorButton, fontColor);
    
    // 更新填充颜色按钮
    QColor fillColor = selectedShape->fillColor();
    Utils::updateColorButton(m_fillColorButton, fillColor);
    
    // 更新线条颜色按钮
    QColor lineColor = selectedShape->lineColor();
    Utils::updateColorButton(m_lineColorButton, lineColor);
}

void MainWindow::onTransparencyChanged(int index)
{
    // 计算透明度值（0-100）
    int transparency = index * 10;
    
    // 获取当前选中的图形
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        // 设置透明度
        selectedShape->setTransparency(transparency);
        
        // 更新绘图区域
        m_drawingArea->update();
    }
}

void MainWindow::onLineWidthChanged(int index)
{
    // 获取线条粗细值
    qreal lineWidth = 0.0;
    
    switch (index) {
    case 0: lineWidth = 0.0; break;
    case 1: lineWidth = 0.5; break;
    case 2: lineWidth = 1.0; break;
    case 3: lineWidth = 1.5; break;
    case 4: lineWidth = 2.0; break;
    case 5: lineWidth = 3.0; break;
    case 6: lineWidth = 4.0; break;
    case 7: lineWidth = 5.0; break;
    case 8: lineWidth = 6.0; break;
    case 9: lineWidth = 8.0; break;
    case 10: lineWidth = 10.0; break;
    default: lineWidth = 1.0;
    }
    
    // 获取当前选中的图形
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        // 设置线条粗细
        selectedShape->setLineWidth(lineWidth);
        
        // 更新绘图区域
        m_drawingArea->update();
    }
}

void MainWindow::onLineStyleChanged(int index)
{
    // 获取当前选中的图形
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        // 设置线条样式
        selectedShape->setLineStyle(index);
        
        // 更新绘图区域
        m_drawingArea->update();
    }
}

// 实现Arrange栏按钮状态更新
void MainWindow::updateArrangeControls()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    
    // 启用或禁用控件，取决于是否有选中的图形
    bool hasSelection = (selectedShape != nullptr);
    
    // 更新Arrange栏按钮状态
    m_bringToFrontButton->setEnabled(hasSelection);
    m_sendToBackButton->setEnabled(hasSelection);
    m_bringForwardButton->setEnabled(hasSelection);
    m_sendBackwardButton->setEnabled(hasSelection);
}
