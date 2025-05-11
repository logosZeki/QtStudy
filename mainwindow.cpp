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
    setWindowTitle(tr("流程图设计器"));
    resize(1800, 1000);

    
    // 连接DrawingArea的shapeSelectionChanged信号到updateFontControls槽
    connect(m_drawingArea, &DrawingArea::shapeSelectionChanged, this, &MainWindow::updateFontControls);
    
    // 连接DrawingArea的fontColorChanged信号到updateFontColorButton槽
    connect(m_drawingArea, &DrawingArea::fontColorChanged, this, &MainWindow::updateFontColorButton);
    
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
    m_tabBar->addTab(tr("导出和导入"));
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
    QPushButton* exportAsSvgButton = new QPushButton(tr("导出为svg"));

    m_exportAndImportToolbar->addWidget(exportAsPngButton);
    m_exportAndImportToolbar->addSeparator();
    m_exportAndImportToolbar->addWidget(exportAsPdfButton);
    m_exportAndImportToolbar->addSeparator();
    m_exportAndImportToolbar->addWidget(exportAsSvgButton);
    m_exportAndImportToolbar->addSeparator();

    // 添加导入工具栏的按钮和控件
    QPushButton* importAsSvgButton = new QPushButton(tr("从svg导入"));
    m_exportAndImportToolbar->addWidget(importAsSvgButton);

    // 连接导出按钮的信号到相应的槽函数
    connect(exportAsPngButton, &QPushButton::clicked, this, &MainWindow::exportAsPng);
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
        QMessageBox::information(this, tr("导出成功"), tr("流程图已成功导出为PNG图片！"));
    } else {
        QMessageBox::critical(this, tr("导出失败"), tr("导出PNG图片时发生错误，请检查文件路径和权限。"));
    }
}

void MainWindow::createStatusBar()
{
    // 创建状态栏
    m_statusBar = new QStatusBar(this);
    setStatusBar(m_statusBar);
    
    // 创建显示图形数量的标签
    m_shapesCountLabel = new QLabel(tr("图形数量: 0"));
    m_statusBar->addWidget(m_shapesCountLabel);
    
    // 添加一个固定宽度的空白区域
    QWidget* spacer = new QWidget();
    spacer->setFixedWidth(20);
    m_statusBar->addWidget(spacer);
    
    // 创建显示缩放比例的标签
    m_zoomLabel = new QLabel(tr("缩放比例: 100%"));
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
        m_shapesCountLabel->setText(tr("图形数量: %1").arg(shapeCount));
        
        // 更新缩放比例标签
        double zoomPercent = m_drawingArea->getScale() * 100.0;
        m_zoomLabel->setText(tr("缩放比例: %1%").arg(zoomPercent, 0, 'f', 1));
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
        m_zoomLabel->setText(tr("缩放比例: %1%").arg(zoomPercent, 0, 'f', 1));
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
        m_zoomLabel->setText(tr("缩放比例: %1%").arg(zoomPercent, 0, 'f', 1));
    }
}
