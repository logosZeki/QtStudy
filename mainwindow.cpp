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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_pageSettingDialog(nullptr)
{
    setupUi();
    
    // 设置窗口标题和大小
    setWindowTitle(tr("流程图设计器"));
    resize(1200, 800);
    
    // 连接DrawingArea的shapeSelectionChanged信号到updateFontControls槽
    connect(m_drawingArea, &DrawingArea::shapeSelectionChanged, this, &MainWindow::updateFontControls);
    
    // 连接DrawingArea的fontColorChanged信号到updateFontColorButton槽
    connect(m_drawingArea, &DrawingArea::fontColorChanged, this, &MainWindow::updateFontColorButton);
    
    // 初始化字体控件状态
    updateFontControls();
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
    createExportToolbar();     // 导出
    
    // 默认只显示"开始"选项卡对应的工具栏
    m_arrangeToolbar->hide();
    m_exportToolbar->hide();

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
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #f0f0f0; }");

    // 设置工具栏固定宽度
    m_toolBar->setFixedWidth(220);
    m_toolBar->setStyleSheet("QWidget { background-color: white; border-right: 1px solid #e0e0e0; }");

    // 添加到布局
    m_contentLayout->addWidget(m_toolBar);
    m_contentLayout->addWidget(scrollArea, 1);
}

void MainWindow::createTopToolbar()
{
    // 创建顶部工具栏
    m_topToolbar = new QToolBar(this);
    m_topToolbar->setMovable(false);
    m_topToolbar->setIconSize(QSize(16, 16));
    m_topToolbar->setStyleSheet("QToolBar { background-color: white; border-bottom: 1px solid #e0e0e0; padding: 2px; }");
    m_mainLayout->addWidget(m_topToolbar);

    // 添加常用按钮
    QAction* homeAction = m_topToolbar->addAction(style()->standardIcon(QStyle::SP_DirHomeIcon), tr("首页"));
    QAction* newAction = m_topToolbar->addAction(style()->standardIcon(QStyle::SP_FileIcon), tr("新建"));
    QAction* menuAction = m_topToolbar->addAction(style()->standardIcon(QStyle::SP_TitleBarMenuButton), tr("菜单"));
    
    m_topToolbar->addSeparator();
    
    // 添加文件名标签
    QLabel* fileNameLabel = new QLabel(tr("未命名文件"));
    m_topToolbar->addWidget(fileNameLabel);
    
    QAction* starAction = m_topToolbar->addAction(style()->standardIcon(QStyle::SP_DialogApplyButton), tr("收藏"));
    
    // 添加伸缩空间，保持后面的控件在右侧
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_topToolbar->addWidget(spacer);
    
    // 添加分享和用户按钮 - 使用确定存在的图标
    QAction* shareAction = m_topToolbar->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), tr("分享"));
    QAction* userAction = m_topToolbar->addAction(style()->standardIcon(QStyle::SP_DesktopIcon), tr("用户"));
    
    // 创建标签栏
    m_tabBar = new QTabBar(this);
    m_tabBar->addTab(tr("开始"));
    m_tabBar->addTab(tr("排列"));
    m_tabBar->addTab(tr("导出"));
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
    m_exportToolbar->hide();
    
    // 根据选中的标签显示对应的工具栏
    switch (index) {
    case 0: // 开始
        m_mainToolbar->show();
        break;
    case 1: // 排列
        m_arrangeToolbar->show();
        break;
    case 2: // 导出 (由于删除了"页面"选项卡，原来的索引3变成了2)
        m_exportToolbar->show();
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
    QAction* undoAction = m_mainToolbar->addAction(style()->standardIcon(QStyle::SP_ArrowBack), tr("撤销"));
    QAction* redoAction = m_mainToolbar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), tr("重做"));
    
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
    m_boldAction->setToolTip(tr("加粗"));
    m_boldAction->setFont(QFont("Arial", 9, QFont::Bold));
    m_boldAction->setCheckable(true);  // 设置为可选中状态
    m_boldAction->setEnabled(false);   // 初始状态下禁用
    
    m_italicAction = m_mainToolbar->addAction(tr("I"));
    m_italicAction->setToolTip(tr("斜体"));
    QFont italicFont("Arial", 9);
    italicFont.setItalic(true);
    m_italicAction->setFont(italicFont);
    m_italicAction->setCheckable(true);  // 设置为可选中状态
    m_italicAction->setEnabled(false);   // 初始状态下禁用
    
    m_underlineAction = m_mainToolbar->addAction(tr("U"));
    m_underlineAction->setToolTip(tr("下划线"));
    QFont underlineFont("Arial", 9);
    underlineFont.setUnderline(true);
    m_underlineAction->setFont(underlineFont);
    m_underlineAction->setCheckable(true);  // 设置为可选中状态
    m_underlineAction->setEnabled(false);   // 初始状态下禁用
    
    m_mainToolbar->addSeparator();
    
    // 添加字体颜色按钮
    m_fontColorButton = new QPushButton(tr("字体颜色"));
    m_fontColorButton->setToolTip(tr("字体颜色"));
    m_fontColorButton->setFixedWidth(100);
    m_fontColorButton->setEnabled(false);  // 初始状态下禁用
    m_mainToolbar->addWidget(m_fontColorButton);
    m_mainToolbar->addSeparator();

    // 添加对齐设置
    m_alignCombo = new QComboBox();
    m_alignCombo->addItem(tr("居中对齐"));
    m_alignCombo->addItem(tr("左对齐"));
    m_alignCombo->addItem(tr("右对齐"));
    m_alignCombo->setFixedWidth(90);
    m_alignCombo->setEnabled(false);  // 初始状态下禁用
    m_mainToolbar->addWidget(m_alignCombo);
    
    m_mainToolbar->addSeparator();
    
    // 添加缩放控件
    QComboBox* zoomCombo = new QComboBox();
    zoomCombo->addItem(tr("100%"));
    zoomCombo->setFixedWidth(70);
    m_mainToolbar->addWidget(zoomCombo);
    
    m_mainToolbar->addSeparator();
    
    // 添加样式设置
    QComboBox* lineColorCombo = new QComboBox();
    lineColorCombo->addItem(tr("线条颜色"));
    lineColorCombo->setFixedWidth(90);
    m_mainToolbar->addWidget(lineColorCombo);
    
    QComboBox* lineWidthCombo = new QComboBox();
    lineWidthCombo->addItem(tr("线条粗度"));
    lineWidthCombo->setFixedWidth(90);
    m_mainToolbar->addWidget(lineWidthCombo);
    
    QComboBox* lineStyleCombo = new QComboBox();
    lineStyleCombo->addItem(tr("线条样式"));
    lineStyleCombo->setFixedWidth(90);
    m_mainToolbar->addWidget(lineStyleCombo);
    
    QComboBox* lineTypeCombo = new QComboBox();
    lineTypeCombo->addItem(tr("线条类型"));
    lineTypeCombo->setFixedWidth(90);
    m_mainToolbar->addWidget(lineTypeCombo);

    // 添加页面设置按钮
    m_pageSettingButton = new QPushButton(tr("Page Setting"));
    m_pageSettingButton->setToolTip(tr("页面设置"));
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
}

void MainWindow::createArrangeToolbar()
{
    // 排列工具栏 (对应"排列"选项卡)
    m_arrangeToolbar = new QToolBar(this);
    m_arrangeToolbar->setMovable(false);
    m_arrangeToolbar->setIconSize(QSize(16, 16));
    m_arrangeToolbar->setStyleSheet("QToolBar { background-color: #f9f9f9; border-bottom: 1px solid #e0e0e0; padding: 4px; }");
    m_mainLayout->addWidget(m_arrangeToolbar);
    

    
    QAction* bringToFrontAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("置于顶层"));
    m_arrangeToolbar->addSeparator();
    QAction* sendToBackAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("置于底层"));
    m_arrangeToolbar->addSeparator();
    QAction* bringForwardAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("上移一层"));
    m_arrangeToolbar->addSeparator();
    QAction* sendBackwardAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("下移一层"));
    m_arrangeToolbar->addSeparator();
}

void MainWindow::createExportToolbar()
{
    // 导出工具栏 (对应"导出"选项卡)
    m_exportToolbar = new QToolBar(this);
    m_exportToolbar->setMovable(false);
    m_exportToolbar->setIconSize(QSize(16, 16));
    m_exportToolbar->setStyleSheet("QToolBar { background-color: #f9f9f9; border-bottom: 1px solid #e0e0e0; padding: 4px; }");
    m_mainLayout->addWidget(m_exportToolbar);
    
    // 添加导出工具栏的按钮和控件
    QAction* exportAsPngAction = m_exportToolbar->addAction(style()->standardIcon(QStyle::SP_FileIcon), tr("导出为PNG"));
    QAction* exportAsJpgAction = m_exportToolbar->addAction(style()->standardIcon(QStyle::SP_FileIcon), tr("导出为JPG"));
    QAction* exportAsPdfAction = m_exportToolbar->addAction(style()->standardIcon(QStyle::SP_FileIcon), tr("导出为PDF"));
    
    m_exportToolbar->addSeparator();
    
    QAction* printAction = m_exportToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("打印"));
}

void MainWindow::showPageSettingDialog()
{
    // 如果对话框不存在，则创建
    if (!m_pageSettingDialog) {
        m_pageSettingDialog = new PageSettingDialog(this);
        
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
    // 获取当前选中的图形
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    
    // 根据是否有选中图形来启用或禁用字体控件
    bool hasSelection = (selectedShape != nullptr);
    m_fontCombo->setEnabled(hasSelection);
    m_fontSizeCombo->setEnabled(hasSelection);
    m_boldAction->setEnabled(hasSelection);
    m_italicAction->setEnabled(hasSelection);
    m_underlineAction->setEnabled(hasSelection);
    m_fontColorButton->setEnabled(hasSelection);
    m_alignCombo->setEnabled(hasSelection);
    
    // 如果有选中图形，更新控件显示当前字体设置
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
