#include "mainwindow.h"
#include "toolbar.h"
#include "drawingarea.h"
#include "pagesettingdialog.h"
#include <QAction>
#include <QStyle>
#include <QIcon>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_pageSettingDialog(nullptr)
{
    setupUi();
    
    // 设置窗口标题和大小
    setWindowTitle(tr("流程图设计器"));
    resize(1200, 800);
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
    QComboBox* fontCombo = new QComboBox();
    fontCombo->addItem(tr("微软雅黑"));
    fontCombo->addItem(tr("宋体"));
    fontCombo->setFixedWidth(100);
    m_mainToolbar->addWidget(fontCombo);

    QComboBox* fontSizeCombo = new QComboBox();
    fontSizeCombo->addItem(tr("12 px"));
    fontSizeCombo->addItem(tr("13 px"));
    fontSizeCombo->setFixedWidth(100);
    m_mainToolbar->addWidget(fontSizeCombo);
    
    m_mainToolbar->addSeparator();
    
    // 添加常用格式按钮 - 使用纯文本代替不存在的图标
    QAction* boldAction = m_mainToolbar->addAction(tr("B"));
    boldAction->setToolTip(tr("加粗"));
    boldAction->setFont(QFont("Arial", 9, QFont::Bold));
    
    QAction* italicAction = m_mainToolbar->addAction(tr("I"));
    italicAction->setToolTip(tr("斜体"));
    QFont italicFont("Arial", 9);
    italicFont.setItalic(true);
    italicAction->setFont(italicFont);
    
    QAction* underlineAction = m_mainToolbar->addAction(tr("U"));
    underlineAction->setToolTip(tr("下划线"));
    QFont underlineFont("Arial", 9);
    underlineFont.setUnderline(true);
    underlineAction->setFont(underlineFont);
    
    m_mainToolbar->addSeparator();
    
    // 添加对齐方式按钮 - 使用确定存在的图标
    QAction* alignLeftAction = m_mainToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogListView), tr("左对齐"));
    
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
}

void MainWindow::createArrangeToolbar()
{
    // 排列工具栏 (对应"排列"选项卡)
    m_arrangeToolbar = new QToolBar(this);
    m_arrangeToolbar->setMovable(false);
    m_arrangeToolbar->setIconSize(QSize(16, 16));
    m_arrangeToolbar->setStyleSheet("QToolBar { background-color: #f9f9f9; border-bottom: 1px solid #e0e0e0; padding: 4px; }");
    m_mainLayout->addWidget(m_arrangeToolbar);
    
    // 添加排列工具栏的按钮和控件
    QAction* alignLeftAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_ArrowLeft), tr("左对齐"));
    QAction* alignCenterAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_MediaSeekBackward), tr("居中对齐"));
    QAction* alignRightAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_ArrowRight), tr("右对齐"));
    
    m_arrangeToolbar->addSeparator();
    
    QAction* alignTopAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_ArrowUp), tr("顶部对齐"));
    QAction* alignMiddleAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_MediaSeekBackward), tr("中部对齐"));
    QAction* alignBottomAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_ArrowDown), tr("底部对齐"));
    
    m_arrangeToolbar->addSeparator();
    
    QAction* distributeHorizontalAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_MediaSeekBackward), tr("水平分布"));
    QAction* distributeVerticalAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_MediaSeekBackward), tr("垂直分布"));
    
    m_arrangeToolbar->addSeparator();
    
    QAction* bringToFrontAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("置于顶层"));
    QAction* sendToBackAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("置于底层"));
    QAction* bringForwardAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("上移一层"));
    QAction* sendBackwardAction = m_arrangeToolbar->addAction(style()->standardIcon(QStyle::SP_FileDialogDetailedView), tr("下移一层"));
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
