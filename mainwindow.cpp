#include "mainwindow.h"
#include "toolbar.h"
#include "drawingarea.h"
#include "pagesettingdialog.h"
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_pageSettingDialog(nullptr)
{
    setupUi();
    
    // 设置窗口标题和大小
    setWindowTitle(tr("Flowchart Designer"));
    resize(1900, 1000);
    //showMaximized();

    
    // 连接DrawingArea的shapeSelectionChanged信号到updateFontControls槽
    connect(m_drawingArea, &DrawingArea::shapeSelectionChanged, this, &MainWindow::updateFontControls);
    
    // 连接DrawingArea的fontColorChanged信号到updateFontColorButton槽
    connect(m_drawingArea, &DrawingArea::fontColorChanged, this, &MainWindow::updateFontColorButton);
    
    // 连接状态栏更新信号
    connect(m_drawingArea, &DrawingArea::selectionChanged, this, &MainWindow::updateStatusBarInfo);
    connect(m_drawingArea, &DrawingArea::scaleChanged, this, &MainWindow::updateZoomSlider);
    connect(m_drawingArea, &DrawingArea::shapesCountChanged, this, &MainWindow::updateStatusBarInfo);
    
    // 连接填充颜色和线条颜色变化信号到更新按钮状态槽
    connect(m_drawingArea, &DrawingArea::fillColorChanged, this, &MainWindow::updateColorButtons);
    connect(m_drawingArea, &DrawingArea::lineColorChanged, this, &MainWindow::updateColorButtons);
    connect(m_drawingArea, &DrawingArea::shapeSelectionChanged, this, &MainWindow::updateColorButtons);
    
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
    //scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #f0f0f0; }");

    // 设置工具栏固定宽度
    m_toolBar->setFixedWidth(249);
    m_toolBar->setStyleSheet("QWidget { background-color: white; border-right: 1px solid #e0e0e0; }");

    // 添加到布局
    m_contentLayout->addWidget(m_toolBar);
    m_contentLayout->addWidget(scrollArea, 1);
    
    // 创建状态栏
    createStatusBar();
}

void MainWindow::createTopToolbar()
{
    
    // 创建标签栏
    m_tabBar = new QTabBar(this);
    m_tabBar->addTab(tr("Start"));
    m_tabBar->addTab(tr("Arrange"));
    m_tabBar->addTab(tr("Export and Import"));
    m_tabBar->setExpanding(false);
    m_tabBar->setDocumentMode(true);
    m_tabBar->setDrawBase(false);
    m_tabBar->setStyleSheet("QTabBar::tab { padding: 6px 16px; border: none; }"
                           "QTabBar::tab:selected { border-bottom: 2px solid #1a73e8; color: #1a73e8; }"
                           "QTabBar::tab:hover:!selected { background-color: #f5f5f5; }");
    
    // 连接标签切换信号
    connect(m_tabBar, &QTabBar::currentChanged, this, &MainWindow::onTabBarClicked);
    
    m_mainLayout->addWidget(m_tabBar);
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
    // 主工具栏 (对应"开始"选项卡)
    m_mainToolbar = new QToolBar(this);
    m_mainToolbar->setMovable(false);
    m_mainToolbar->setIconSize(QSize(16, 16));
    m_mainToolbar->setStyleSheet("QToolBar { background-color: #f9f9f9; border-bottom: 1px solid #e0e0e0; padding: 4px; }");
    m_mainLayout->addWidget(m_mainToolbar);
    
    // 添加撤销和重做按钮
    QAction* undoAction = m_mainToolbar->addAction(style()->standardIcon(QStyle::SP_ArrowBack), tr("Undo"));
    QAction* redoAction = m_mainToolbar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), tr("Redo"));
    
    m_mainToolbar->addSeparator();
    
    // 添加字体设置
    m_fontCombo = new QComboBox();
    m_fontCombo->addItem(tr("微软雅黑"));
    m_fontCombo->addItem(tr("宋体"));
    m_fontCombo->addItems(QFontDatabase().families());  // 添加系统中所有可用的字体
    m_fontCombo->setFixedWidth(120);
    m_fontCombo->setEnabled(false);  // 初始状态下禁用
    m_mainToolbar->addWidget(m_fontCombo);

    m_fontSizeCombo = new QComboBox();
    // 添加PRD要求的字体大小选项
    // 12px至20px（间隔为1px）
    for (int i = 9; i <= 18; i++) {
        m_fontSizeCombo->addItem(QString("%1 px").arg(i));
    }
    // 添加额外的大字号
    m_fontSizeCombo->addItem(tr("20 px"));
    m_fontSizeCombo->addItem(tr("24 px"));
    m_fontSizeCombo->addItem(tr("32 px"));
    m_fontSizeCombo->addItem(tr("40 px"));
    m_fontSizeCombo->addItem(tr("64 px"));
    
    m_fontSizeCombo->setFixedWidth(100);
    m_fontSizeCombo->setEnabled(false);  // 初始状态下禁用
    m_mainToolbar->addWidget(m_fontSizeCombo);
    
    m_mainToolbar->addSeparator();
    
    // 添加常用格式按钮 - 使用纯文本代替不存在的图标
    m_boldAction = m_mainToolbar->addAction(tr("B"));
    m_boldAction->setToolTip(tr("Bold"));
    m_boldAction->setFont(QFont("Arial", 9, QFont::Bold));
    m_boldAction->setCheckable(true);  // 设置为可选中状态
    m_boldAction->setEnabled(false);   // 初始状态下禁用
    
    m_italicAction = m_mainToolbar->addAction(tr("I"));
    m_italicAction->setToolTip(tr("Italic"));
    QFont italicFont("Arial", 9);
    italicFont.setItalic(true);
    m_italicAction->setFont(italicFont);
    m_italicAction->setCheckable(true);  // 设置为可选中状态
    m_italicAction->setEnabled(false);   // 初始状态下禁用
    
    m_underlineAction = m_mainToolbar->addAction(tr("U"));
    m_underlineAction->setToolTip(tr("Underline"));
    QFont underlineFont("Arial", 9);
    underlineFont.setUnderline(true);
    m_underlineAction->setFont(underlineFont);
    m_underlineAction->setCheckable(true);  // 设置为可选中状态
    m_underlineAction->setEnabled(false);   // 初始状态下禁用
    
    m_mainToolbar->addSeparator();
    
    // 添加字体颜色按钮
    m_fontColorButton = new QPushButton(tr("FontColor"));
    m_fontColorButton->setToolTip(tr("FontColor"));
    m_fontColorButton->setFixedWidth(100);
    m_fontColorButton->setEnabled(false);  // 初始状态下禁用
    m_mainToolbar->addWidget(m_fontColorButton);
    m_mainToolbar->addSeparator();

    // 添加对齐设置
    m_alignCombo = new QComboBox();
    m_alignCombo->addItem(tr("Center Aligned"));
    m_alignCombo->addItem(tr("Left Aligned"));
    m_alignCombo->addItem(tr("Right Aligned"));
    m_alignCombo->setFixedWidth(90);
    m_alignCombo->setEnabled(false);  // 初始状态下禁用
    m_mainToolbar->addWidget(m_alignCombo);
    
    m_mainToolbar->addSeparator();

    // 创建填充颜色按钮
    m_fillColorButton = new QPushButton(tr("FillColor"));
    m_fillColorButton->setToolTip(tr("FillColor"));
    m_fillColorButton->setFixedWidth(100);
    m_mainToolbar->addWidget(m_fillColorButton);
    
    // 连接填充颜色按钮的点击信号
    connect(m_fillColorButton, &QPushButton::clicked, this, &MainWindow::onFillColorButtonClicked);
    
    m_mainToolbar->addSeparator();

    // 添加透明度组合框
    QLabel* transparencyLabel = new QLabel(tr("transparency:"));
    m_mainToolbar->addWidget(transparencyLabel);
    
    m_transparencyCombo = new QComboBox();
    // 添加0%-100%的选项，每10%一个档位
    for (int i = 0; i <= 100; i += 10) {
        m_transparencyCombo->addItem(QString("%1%").arg(i));
    }
    m_transparencyCombo->setCurrentIndex(10); // 默认选择100%（完全不透明）
    m_transparencyCombo->setFixedWidth(70);
    m_transparencyCombo->setEnabled(false);  // 初始状态下禁用
    m_mainToolbar->addWidget(m_transparencyCombo);
    
    m_mainToolbar->addSeparator();
    
    // 创建线条颜色按钮
    m_lineColorButton = new QPushButton(tr("LineColor"));
    m_lineColorButton->setToolTip(tr("LineColor"));
    m_lineColorButton->setFixedWidth(100);
    m_mainToolbar->addWidget(m_lineColorButton);
    
    // 连接线条颜色按钮的点击信号
    connect(m_lineColorButton, &QPushButton::clicked, this, &MainWindow::onLineColorButtonClicked);
    
    m_mainToolbar->addSeparator();

    // 添加线条粗细下拉框
    QLabel* lineWidthLabel = new QLabel(tr("LineWidth:"));
    m_mainToolbar->addWidget(lineWidthLabel);
    
    m_lineWidthCombo = new QComboBox();
    // 添加各种线条粗细选项
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
    
    m_lineWidthCombo->setCurrentIndex(3); // 默认选择1.5px
    m_lineWidthCombo->setFixedWidth(90);
    m_lineWidthCombo->setEnabled(false);  // 初始状态下禁用
    m_mainToolbar->addWidget(m_lineWidthCombo);
    
    // 添加线条样式下拉框
    QLabel* lineStyleLabel = new QLabel(tr("LineStyle:"));
    m_mainToolbar->addWidget(lineStyleLabel);
    
    m_lineStyleCombo = new QComboBox();
    // 添加各种线条样式选项
    m_lineStyleCombo->addItem(tr("Solid line"));
    m_lineStyleCombo->addItem(tr("Dashed line (short)"));
    m_lineStyleCombo->addItem(tr("Dashed line (long)"));
    m_lineStyleCombo->addItem(tr("Dashed line"));
    
    m_lineStyleCombo->setCurrentIndex(0); // 默认选择实线
    m_lineStyleCombo->setFixedWidth(90);
    m_lineStyleCombo->setEnabled(false);  // 初始状态下禁用
    m_mainToolbar->addWidget(m_lineStyleCombo);
    
    // 添加页面设置按钮
    m_pageSettingButton = new QPushButton(tr("Page Setting"));
    m_pageSettingButton->setToolTip(tr("Page Setting"));
    m_pageSettingButton->setFixedWidth(110);
    m_mainToolbar->addWidget(m_pageSettingButton);

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
    // 排列工具栏 (对应"排列"选项卡)
    m_arrangeToolbar = new QToolBar(this);
    m_arrangeToolbar->setMovable(false);
    m_arrangeToolbar->setIconSize(QSize(16, 16));
    m_arrangeToolbar->setStyleSheet("QToolBar { background-color: #f9f9f9; border-bottom: 1px solid #e0e0e0; padding: 4px; }");
    m_mainLayout->addWidget(m_arrangeToolbar);
    

    
    QAction* bringToFrontAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("Placed at the top"));
    m_arrangeToolbar->addSeparator();
    QAction* sendToBackAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("Placed at the bottom"));
    m_arrangeToolbar->addSeparator();
    QAction* bringForwardAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("Move up one level"));
    m_arrangeToolbar->addSeparator();
    QAction* sendBackwardAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("Move down one level"));
    m_arrangeToolbar->addSeparator();
}

void MainWindow::createExportAndImportToolbar()
{
    // 导出工具栏 (对应"导出"选项卡)
    m_exportAndImportToolbar = new QToolBar(this);
    m_exportAndImportToolbar->setMovable(false);
    m_exportAndImportToolbar->setIconSize(QSize(16, 16));
    m_exportAndImportToolbar->setStyleSheet("QToolBar { background-color: #f9f9f9; border-bottom: 1px solid #e0e0e0; padding: 4px; }");
    m_mainLayout->addWidget(m_exportAndImportToolbar);
    
    // 添加导出工具栏的按钮和控件
    QPushButton* exportAsPngButton = new QPushButton(tr("导出为PNG"));
    QPushButton* exportAsPdfButton = new QPushButton(tr("导出为PDF"));
    QPushButton* exportAsSvgButton = new QPushButton(tr("导出为SVG"));

    m_exportAndImportToolbar->addWidget(exportAsPngButton);
    m_exportAndImportToolbar->addSeparator();
    m_exportAndImportToolbar->addWidget(exportAsPdfButton);
    m_exportAndImportToolbar->addSeparator();
    m_exportAndImportToolbar->addWidget(exportAsSvgButton);
    m_exportAndImportToolbar->addSeparator();

    // 添加导入工具栏的按钮和控件
    QPushButton* importFromSvgButton = new QPushButton(tr("从SVG导入"));
    m_exportAndImportToolbar->addWidget(importFromSvgButton);

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
    
    // 更新透明度、线条粗细和线条样式下拉框的状态
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
        
        // 更新字体颜色按钮样式
        QColor fontColor = selectedShape->fontColor();
        QString colorStyle = QString("QPushButton { background-color: %1; color: %2 }")
                             .arg(fontColor.name())
                             .arg(fontColor.lightness() < 128 ? "white" : "black");
        m_fontColorButton->setStyleSheet(colorStyle);
        
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
    QColor initialColor = Qt::black;
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        initialColor = selectedShape->fontColor();
    }
    
    QColor color = QColorDialog::getColor(initialColor, this, tr("选择字体颜色"));
    if (color.isValid()) {
        m_drawingArea->setSelectedShapeFontColor(color);
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
    QString colorStyle = QString("QPushButton { background-color: %1; color: %2 }")
                         .arg(color.name())
                         .arg(color.lightness() < 128 ? "white" : "black");
    m_fontColorButton->setStyleSheet(colorStyle);
}

void MainWindow::exportAsPng()
{
    if (!m_drawingArea) return;
    
    // 获取当前日期时间，格式化为字符串作为默认文件名
    QString defaultFileName = QString("FlowChart_%1").arg(
        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    
    // 打开文件保存对话框
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("导出为PNG"),
        QDir::homePath() + "/" + defaultFileName,
        tr("PNG图片 (*.png)"));
    
    // 如果用户取消了对话框，则返回
    if (filePath.isEmpty()) {
        return;
    }
    
    // 确保文件路径以.png结尾
    if (!filePath.endsWith(".png", Qt::CaseInsensitive)) {
        filePath += ".png";
    }
    
    // 调用绘图区域的导出方法
    bool success = m_drawingArea->exportToPng(filePath);
    
    // 显示导出结果提示
    if (success) {
        QMessageBox::information(this, tr("Export successful"), tr("Flowchart has been exported to PNG image successfully!"));
    } else {
        QMessageBox::critical(this, tr("Export failed"), tr("An error occurred while exporting the PNG image. Please check the file path and permissions."));
    }
}

void MainWindow::exportAsSvg()
{
    if (!m_drawingArea) return;
    
    // 获取当前日期时间，格式化为字符串作为默认文件名
    QString defaultFileName = QString("FlowChart_%1").arg(
        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    
    // 打开文件保存对话框
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("导出为SVG"),
        QDir::homePath() + "/" + defaultFileName,
        tr("SVG矢量图 (*.svg)"));
    
    // 如果用户取消了对话框，则返回
    if (filePath.isEmpty()) {
        return;
    }
    
    // 确保文件路径以.svg结尾
    if (!filePath.endsWith(".svg", Qt::CaseInsensitive)) {
        filePath += ".svg";
    }
    
    // 调用绘图区域的导出方法
    bool success = m_drawingArea->exportToSvg(filePath);
    
    // 显示导出结果提示
    if (success) {
        QMessageBox::information(this, tr("Export successful"), tr("Flowchart has been exported to SVG vector image successfully!"));
    } else {
        QMessageBox::critical(this, tr("Export failed"), tr("An error occurred while exporting the SVG file. Please check the file path and permissions."));
    }
}

void MainWindow::importFromSvg()
{
    if (!m_drawingArea) return;
    
    // 打开文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Import from SVG"),
        QDir::homePath(),
        tr("SVG File(*.svg)"));
    
    // 如果用户取消了对话框，则返回
    if (filePath.isEmpty()) {
        return;
    }
    
    // 提示用户确认是否要导入
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        tr("Confirm import"), 
        tr("Importing SVG will clear all content on the current canvas. Do you want to continue?"),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::No) {
        return;
    }
    
    // 调用绘图区域的导入方法
    bool success = m_drawingArea->importFromSvg(filePath);
    
    // 显示导入结果提示
    if (success) {
        QMessageBox::information(this, tr("Import successful"), tr("SVG file has been imported successfully!"));
    } else {
        QMessageBox::critical(this, tr("Import failed"), tr("An error occurred while importing the SVG file. Please ensure the file format is correct."));
    }
}

void MainWindow::createStatusBar()
{
    // 创建状态栏
    m_statusBar = new QStatusBar(this);
    setStatusBar(m_statusBar);
    
    // 创建显示图形数量的标签
    m_shapesCountLabel = new QLabel(tr("Number of shapes: 0"));
    m_statusBar->addWidget(m_shapesCountLabel);
    
    // 添加一个固定宽度的空白区域
    QWidget* spacer = new QWidget();
    spacer->setFixedWidth(20);
    m_statusBar->addWidget(spacer);
    
    // 创建显示缩放比例的标签
    m_zoomLabel = new QLabel(tr("Zoom level: 100%"));
    m_statusBar->addWidget(m_zoomLabel);
    
    // 添加一个固定宽度的空白区域
    QWidget* sliderSpacer = new QWidget();
    sliderSpacer->setFixedWidth(10);
    m_statusBar->addWidget(sliderSpacer);
    
    // 创建缩放滑块
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setMinimumWidth(150);
    m_zoomSlider->setMaximumWidth(200);
    m_zoomSlider->setMinimum(0);    // 这个值将在 updateZoomSlider 中根据 MIN_SCALE 转换
    m_zoomSlider->setMaximum(100);  // 这个值将在 updateZoomSlider 中根据 MAX_SCALE 转换
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
        QColor color = QColorDialog::getColor(initialColor, this, tr("Select fill color"));
        
        if (color.isValid()) {
            m_drawingArea->setSelectedShapeFillColor(color);
        }
    }
}

// 线条颜色按钮的点击槽函数
void MainWindow::onLineColorButtonClicked()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        QColor initialColor = selectedShape->lineColor();
        QColor color = QColorDialog::getColor(initialColor, this, tr("Select line color"));
        
        if (color.isValid()) {
            m_drawingArea->setSelectedShapeLineColor(color);
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
    
    // 填充颜色按钮样式更新
    QColor fillColor = selectedShape->fillColor();
    // 根据填充颜色的亮度决定文字颜色
    QString textColor = fillColor.lightness() < 128 ? "white" : "black";
    QString fillColorStyle = QString("QPushButton { background-color: %1; color: %2; }").arg(fillColor.name()).arg(textColor);
    m_fillColorButton->setStyleSheet(fillColorStyle);
    
    // 线条颜色按钮样式更新
    QColor lineColor = selectedShape->lineColor();
    // 根据线条颜色的亮度决定文字颜色
    textColor = lineColor.lightness() < 128 ? "white" : "black";
    QString lineColorStyle = QString("QPushButton { background-color: %1; color: %2; }").arg(lineColor.name()).arg(textColor);
    m_lineColorButton->setStyleSheet(lineColorStyle);
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
