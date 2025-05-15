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
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setupUi();
    setWindowTitle(tr("Flowchart Designer"));
    resize(1900, 1000);
    connect(m_drawingArea, &DrawingArea::shapeSelectionChanged, this, &MainWindow::updateFontControls);
    connect(m_drawingArea, &DrawingArea::shapeSelectionChanged, this, &MainWindow::updateArrangeControls);
    connect(m_drawingArea, &DrawingArea::fontColorChanged, this, &MainWindow::updateColorButtons);
    connect(m_drawingArea, &DrawingArea::fillColorChanged, this, &MainWindow::updateColorButtons);
    connect(m_drawingArea, &DrawingArea::lineColorChanged, this, &MainWindow::updateColorButtons);
    connect(m_drawingArea, &DrawingArea::shapeSelectionChanged, this, &MainWindow::updateColorButtons);
    connect(m_drawingArea, &DrawingArea::selectionChanged, this, &MainWindow::updateStatusBarInfo);
    connect(m_drawingArea, &DrawingArea::scaleChanged, this, &MainWindow::updateZoomSlider);
    connect(m_drawingArea, &DrawingArea::shapesCountChanged, this, &MainWindow::updateStatusBarInfo);
    updateZoomSlider();
    updateFontControls();
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
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
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
    createTitleBar();
    m_toolBar = new ToolBar(this);
    m_drawingArea = new DrawingArea(this);
    createTopToolbar();
    createMainToolbar();       
    createArrangeToolbar();    
    createExportAndImportToolbar();     
    m_arrangeToolbar->hide();
    m_exportAndImportToolbar->hide();
    m_contentLayout = new QHBoxLayout();
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);
    m_mainLayout->addLayout(m_contentLayout);
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(m_drawingArea);
    scrollArea->setWidgetResizable(false);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #f5f5f7; }");
    m_toolBar->setFixedWidth(249);
    m_toolBar->setStyleSheet("QWidget { background-color: #f5f5f7; border-right: 1px solid #e0e0e0; }");
    m_contentLayout->addWidget(m_toolBar);
    m_contentLayout->addWidget(scrollArea, 1);
    createStatusBar();
}
void MainWindow::createTitleBar()
{
    QWidget* titleBar = new QWidget(this);
    titleBar->setFixedHeight(30);
    titleBar->setStyleSheet("background-color: #f5f5f7; border-bottom: 1px solid #e5e5e5;");
    QHBoxLayout* layout = new QHBoxLayout(titleBar);
    layout->setContentsMargins(10, 0, 10, 0);
    QLabel* titleLabel = new QLabel(tr("Flowchart Designer"), this);
    titleLabel->setStyleSheet("font-size: 13px; font-weight: 500; color: #333;");
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
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addWidget(closeButton);
    buttonLayout->addWidget(minButton);
    buttonLayout->addWidget(maxButton);
    layout->addLayout(buttonLayout);
    layout->addWidget(titleLabel, 0, Qt::AlignCenter);
    layout->addStretch();
    m_mainLayout->addWidget(titleBar);
    connect(minButton, &QPushButton::clicked, this, &MainWindow::showMinimized);
    connect(maxButton, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) {
            showNormal();
        } else {
            showMaximized();
        }
    });
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::close);
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
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            if (isMaximized()) {
                showNormal();
            } else {
                showMaximized();
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}
void MainWindow::createTopToolbar()
{
    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);
    QWidget *containerWidget = new QWidget(this);
    containerWidget->setStyleSheet("background-color: #f5f5f7;");
    containerWidget->setLayout(centerLayout);
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
    connect(m_tabBar, &QTabBar::currentChanged, this, &MainWindow::onTabBarClicked);
    centerLayout->addStretch();
    centerLayout->addWidget(m_tabBar);
    centerLayout->addStretch();
    m_mainLayout->addWidget(containerWidget);
}
void MainWindow::onTabBarClicked(int index)
{
    m_mainToolbar->hide();
    m_arrangeToolbar->hide();
    m_exportAndImportToolbar->hide();
    switch (index) {
    case 0: 
        m_mainToolbar->show();
        break;
    case 1: 
        m_arrangeToolbar->show();
        break;
    case 2: 
        m_exportAndImportToolbar->show();
        break;
    default:
        m_mainToolbar->show(); 
        break;
    }
}
void MainWindow::createMainToolbar()
{
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
    QWidget* leftSpacer = new QWidget();
    leftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_mainToolbar->addWidget(leftSpacer);
    m_fontCombo = new QComboBox();
    m_fontCombo->addItem(tr("San Francisco"));
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
    m_mainToolbar->addAction(m_boldAction);
    m_mainToolbar->addAction(m_italicAction);
    m_mainToolbar->addAction(m_underlineAction);
    m_mainToolbar->addSeparator();
    m_fontColorButton = Utils::getAutoChangeColorsButton(this, tr("Font"), 54, 38, 28, 8);
    m_fontColorIndicator = m_fontColorButton->findChild<QFrame*>("colorIndicator");
    m_mainToolbar->addWidget(m_fontColorButton);
    Utils::updateColorButton(m_fontColorButton, QColor(0, 0, 0)); 
    m_mainToolbar->addSeparator();
    m_alignCombo = new QComboBox();
    m_alignCombo->addItem(tr("Center"));
    m_alignCombo->addItem(tr("Left"));
    m_alignCombo->addItem(tr("Right"));
    m_alignCombo->setFixedWidth(90);
    m_alignCombo->setEnabled(false);
    m_mainToolbar->addWidget(m_alignCombo);
    m_mainToolbar->addSeparator();
    m_fillColorButton = Utils::getAutoChangeColorsButton(this, tr("Fill"), 54, 38, 28, 8);
    m_fillColorIndicator = m_fillColorButton->findChild<QFrame*>("colorIndicator");
    m_mainToolbar->addWidget(m_fillColorButton);
    connect(m_fillColorButton, &QPushButton::clicked, this, &MainWindow::onFillColorButtonClicked);
    m_mainToolbar->addSeparator();
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
    m_lineColorButton = Utils::getAutoChangeColorsButton(this, tr("Line"), 54, 38, 28, 8);
    m_lineColorIndicator = m_lineColorButton->findChild<QFrame*>("colorIndicator");
    m_mainToolbar->addWidget(m_lineColorButton);
    connect(m_lineColorButton, &QPushButton::clicked, this, &MainWindow::onLineColorButtonClicked);
    m_mainToolbar->addSeparator();
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
    QWidget* rightSpacer = new QWidget();
    rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_mainToolbar->addWidget(rightSpacer);
    connect(m_pageSettingButton, &QPushButton::clicked, this, &MainWindow::showPageSettingDialog);
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
    QWidget* leftSpacer = new QWidget();
    leftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_arrangeToolbar->addWidget(leftSpacer);
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
    m_sendBackwardButton = new QPushButton(tr("Move Shape Down"));
    m_sendBackwardButton->setIcon(QIcon::fromTheme("go-down"));
    m_sendBackwardButton->setFixedHeight(30);
    m_sendBackwardButton->setStyleSheet(
        "QPushButton { padding-left: 8px; padding-right: 8px; border-radius: 4px; background-color: #f5f5f7; }"
        "QPushButton:hover { background-color: #e5e5e5; }"
        "QPushButton:disabled { color: rgba(0, 0, 0, 0.25); }"
    );
    m_arrangeToolbar->addWidget(m_sendBackwardButton);
    m_arrangeToolbar->addSeparator();
    QLabel* xLabel = new QLabel("X");
    xLabel->setAlignment(Qt::AlignCenter);
    xLabel->setFixedWidth(20);
    m_arrangeToolbar->addWidget(xLabel);
    m_xSpinBox = new QSpinBox();
    m_xSpinBox->setRange(0, 9999);
    m_xSpinBox->setSuffix("px");
    m_xSpinBox->setFixedWidth(80);
    m_xSpinBox->setEnabled(false);
    m_xSpinBox->setSingleStep(10); 
    m_xSpinBox->setStyleSheet(
        "QSpinBox { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }"
        "QSpinBox:disabled { color: rgba(0, 0, 0, 0.25); background-color: #f8f8f8; }"
    );
    m_arrangeToolbar->addWidget(m_xSpinBox);
    QLabel* yLabel = new QLabel("Y");
    yLabel->setAlignment(Qt::AlignCenter);
    yLabel->setFixedWidth(20);
    m_arrangeToolbar->addWidget(yLabel);
    m_ySpinBox = new QSpinBox();
    m_ySpinBox->setRange(0, 9999);
    m_ySpinBox->setSuffix("px");
    m_ySpinBox->setFixedWidth(80);
    m_ySpinBox->setEnabled(false);
    m_ySpinBox->setSingleStep(10); 
    m_ySpinBox->setStyleSheet(
        "QSpinBox { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }"
        "QSpinBox:disabled { color: rgba(0, 0, 0, 0.25); background-color: #f8f8f8; }"
    );
    m_arrangeToolbar->addWidget(m_ySpinBox);
    QLabel* wLabel = new QLabel("W");
    wLabel->setAlignment(Qt::AlignCenter);
    wLabel->setFixedWidth(20);
    m_arrangeToolbar->addWidget(wLabel);
    m_widthSpinBox = new QSpinBox();
    m_widthSpinBox->setRange(1, 9999);
    m_widthSpinBox->setSuffix("px");
    m_widthSpinBox->setFixedWidth(80);
    m_widthSpinBox->setEnabled(false);
    m_widthSpinBox->setSingleStep(10); 
    m_widthSpinBox->setStyleSheet(
        "QSpinBox { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }"
        "QSpinBox:disabled { color: rgba(0, 0, 0, 0.25); background-color: #f8f8f8; }"
    );
    m_arrangeToolbar->addWidget(m_widthSpinBox);
    QLabel* hLabel = new QLabel("H");
    hLabel->setAlignment(Qt::AlignCenter);
    hLabel->setFixedWidth(20);
    m_arrangeToolbar->addWidget(hLabel);
    m_heightSpinBox = new QSpinBox();
    m_heightSpinBox->setRange(1, 9999);
    m_heightSpinBox->setSuffix("px");
    m_heightSpinBox->setFixedWidth(80);
    m_heightSpinBox->setEnabled(false);
    m_heightSpinBox->setSingleStep(10); 
    m_heightSpinBox->setStyleSheet(
        "QSpinBox { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }"
        "QSpinBox:disabled { color: rgba(0, 0, 0, 0.25); background-color: #f8f8f8; }"
    );
    m_arrangeToolbar->addWidget(m_heightSpinBox);
    QWidget* rightSpacer = new QWidget();
    rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_arrangeToolbar->addWidget(rightSpacer);
    connect(m_bringToFrontButton, &QPushButton::clicked, m_drawingArea, &DrawingArea::moveShapeToTop);
    connect(m_sendToBackButton, &QPushButton::clicked, m_drawingArea, &DrawingArea::moveShapeToBottom);
    connect(m_bringForwardButton, &QPushButton::clicked, m_drawingArea, &DrawingArea::moveShapeUp);
    connect(m_sendBackwardButton, &QPushButton::clicked, m_drawingArea, &DrawingArea::moveShapeDown);
    m_bringToFrontButton->setEnabled(false);
    m_sendToBackButton->setEnabled(false);
    m_bringForwardButton->setEnabled(false);
    m_sendBackwardButton->setEnabled(false);
    connect(m_xSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MainWindow::onXCoordChanged);
    connect(m_ySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MainWindow::onYCoordChanged);
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MainWindow::onWidthChanged);
    connect(m_heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MainWindow::onHeightChanged);
    connect(m_drawingArea, &DrawingArea::shapePositionChanged, 
            this, &MainWindow::updateShapePositionSizeControls);
    connect(m_drawingArea, &DrawingArea::shapeSizeChanged, 
            this, &MainWindow::updateShapePositionSizeControls);
}
void MainWindow::createExportAndImportToolbar()
{
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
    QWidget* leftSpacer = new QWidget();
    leftSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_exportAndImportToolbar->addWidget(leftSpacer);
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
    QWidget* rightSpacer = new QWidget();
    rightSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_exportAndImportToolbar->addWidget(rightSpacer);
    connect(exportAsPngButton, &QPushButton::clicked, this, &MainWindow::exportAsPng);
    connect(exportAsSvgButton, &QPushButton::clicked, this, &MainWindow::exportAsSvg);
    connect(importFromSvgButton, &QPushButton::clicked, this, &MainWindow::importFromSvg);
}
void MainWindow::showPageSettingDialog()
{
    if (!m_pageSettingDialog) {
        m_pageSettingDialog = new PageSettingDialog(this, m_drawingArea);
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
        connect(m_pageSettingDialog, &PageSettingDialog::settingsApplied, 
                this, &MainWindow::applyPageSettings);
    }
    m_pageSettingDialog->setModal(true);
    m_pageSettingDialog->exec();
}
void MainWindow::applyPageSettings()
{
    if (!m_pageSettingDialog || !m_drawingArea) return;
    m_drawingArea->setBackgroundColor(m_pageSettingDialog->getBackgroundColor());
    m_drawingArea->setPageSize(m_pageSettingDialog->getPageSize());
    m_drawingArea->setShowGrid(m_pageSettingDialog->getShowGrid());
    m_drawingArea->setGridColor(m_pageSettingDialog->getGridColor());
    m_drawingArea->setGridSize(m_pageSettingDialog->getGridSize());
    m_drawingArea->setGridThickness(m_pageSettingDialog->getGridThickness());
    m_drawingArea->applyPageSettings();
}
void MainWindow::updateFontControls()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    bool hasSelection = (selectedShape != nullptr);
    m_fontCombo->setEnabled(hasSelection);
    m_fontSizeCombo->setEnabled(hasSelection);
    m_boldAction->setEnabled(hasSelection);
    m_italicAction->setEnabled(hasSelection);
    m_underlineAction->setEnabled(hasSelection);
    m_fontColorButton->setEnabled(hasSelection);
    m_alignCombo->setEnabled(hasSelection);
    m_fillColorButton->setEnabled(hasSelection);
    m_lineColorButton->setEnabled(hasSelection);
    if (hasSelection) {
        updateColorButtons();
    } else {
        Utils::updateColorButton(m_fontColorButton, QColor(255, 255, 255));
        Utils::updateColorButton(m_fillColorButton, QColor(255, 255, 255));
        Utils::updateColorButton(m_lineColorButton, QColor(255, 255, 255));
    }
    m_transparencyCombo->setEnabled(hasSelection);
    m_lineWidthCombo->setEnabled(hasSelection);
    m_lineStyleCombo->setEnabled(hasSelection);
    if (hasSelection) {
        QString fontFamily = selectedShape->fontFamily();
        int fontFamilyIndex = m_fontCombo->findText(fontFamily);
        if (fontFamilyIndex >= 0) {
            m_fontCombo->setCurrentIndex(fontFamilyIndex);
        }
        int fontSize = selectedShape->fontSize();
        int fontSizeIndex = m_fontSizeCombo->findText(QString("%1 px").arg(fontSize));
        if (fontSizeIndex >= 0) {
            m_fontSizeCombo->setCurrentIndex(fontSizeIndex);
        }
        m_boldAction->setChecked(selectedShape->isFontBold());
        m_italicAction->setChecked(selectedShape->isFontItalic());
        m_underlineAction->setChecked(selectedShape->isFontUnderline());
        Qt::Alignment alignment = selectedShape->textAlignment();
        if (alignment & Qt::AlignCenter) {
            m_alignCombo->setCurrentIndex(0);
        } else if (alignment & Qt::AlignLeft) {
            m_alignCombo->setCurrentIndex(1);
        } else if (alignment & Qt::AlignRight) {
            m_alignCombo->setCurrentIndex(2);
        }
        int transparency = selectedShape->transparency();
        int transparencyIndex = transparency / 10; 
        m_transparencyCombo->setCurrentIndex(transparencyIndex);
        qreal lineWidth = selectedShape->lineWidth();
        int lineWidthIndex = 0;
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
        int lineStyle = selectedShape->lineStyle();
        m_lineStyleCombo->setCurrentIndex(lineStyle);
    }
}
void MainWindow::onFontFamilyChanged(const QString& family)
{
    m_drawingArea->setSelectedShapeFontFamily(family);
}
void MainWindow::onFontSizeChanged(const QString& sizeText)
{
    bool ok;
    int size = sizeText.split(" ").first().toInt(&ok);
    if (ok) {
        m_drawingArea->setSelectedShapeFontSize(size);
    }
}
void MainWindow::onBoldActionTriggered()
{
    m_drawingArea->setSelectedShapeFontBold(m_boldAction->isChecked());
}
void MainWindow::onItalicActionTriggered()
{
    m_drawingArea->setSelectedShapeFontItalic(m_italicAction->isChecked());
}
void MainWindow::onUnderlineActionTriggered()
{
    m_drawingArea->setSelectedShapeFontUnderline(m_underlineAction->isChecked());
}
void MainWindow::onFontColorButtonClicked()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        QColor initialColor = selectedShape->fontColor();
        QColor color = QColorDialog::getColor(initialColor, this, tr("Select Font Color"));
        if (color.isValid()) {
            m_drawingArea->setSelectedShapeFontColor(color);
            Utils::updateColorButton(m_fontColorButton, color);
        }
    }
}
void MainWindow::onAlignmentChanged(int index)
{
    Qt::Alignment alignment;
    switch (index) {
    case 0:  
        alignment = Qt::AlignCenter;
        break;
    case 1:  
        alignment = Qt::AlignLeft | Qt::AlignVCenter; 
        break;
    case 2:  
        alignment = Qt::AlignRight | Qt::AlignVCenter;
        break;
    default:
        alignment = Qt::AlignCenter;
        break;
    }
    m_drawingArea->setSelectedShapeTextAlignment(alignment);
}
void MainWindow::updateFontColorButton(const QColor& color)
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (!selectedShape) {
        Utils::updateColorButton(m_fontColorButton, color);
        return;
    }
    updateColorButtons();
}
void MainWindow::exportAsPng()
{
    if (!m_drawingArea) return;
    QString defaultFileName = QString("FlowChart_%1").arg(
        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QFileDialog dialog(this, tr("Export as PNG"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("PNG Images (*.png)"));
    dialog.setDirectory(QDir::homePath());
    dialog.selectFile(defaultFileName);
    dialog.setDefaultSuffix("png");
    dialog.setStyleSheet(
        "QFileDialog { background-color: #f5f5f7; }"
        "QPushButton { border-radius: 4px; padding: 6px 12px; }"
        "QPushButton[text=\"Save\"], QPushButton[text=\"OK\"] { background-color: #007aff; color: white; }"
        "QPushButton[text=\"Cancel\"] { background-color: #f5f5f7; border: 1px solid #ccc; }"
        "QComboBox, QLineEdit { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }"
    );
    if (!dialog.exec()) {
        return;
    }
    QString filePath = dialog.selectedFiles().first();
    if (!filePath.endsWith(".png", Qt::CaseInsensitive)) {
        filePath += ".png";
    }
    bool success = m_drawingArea->exportToPng(filePath);
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
    QString defaultFileName = QString("FlowChart_%1").arg(
        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QFileDialog dialog(this, tr("Export as SVG"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("SVG Files (*.svg)"));
    dialog.setDirectory(QDir::homePath());
    dialog.selectFile(defaultFileName);
    dialog.setDefaultSuffix("svg");
    dialog.setStyleSheet(
        "QFileDialog { background-color: #f5f5f7; }"
        "QPushButton { border-radius: 4px; padding: 6px 12px; }"
        "QPushButton[text=\"Save\"], QPushButton[text=\"OK\"] { background-color: #007aff; color: white; }"
        "QPushButton[text=\"Cancel\"] { background-color: #f5f5f7; border: 1px solid #ccc; }"
        "QComboBox, QLineEdit { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }"
    );
    if (!dialog.exec()) {
        return;
    }
    QString filePath = dialog.selectedFiles().first();
    if (!filePath.endsWith(".svg", Qt::CaseInsensitive)) {
        filePath += ".svg";
    }
    bool success = m_drawingArea->exportToSvg(filePath);
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
    QFileDialog dialog(this, tr("Import from SVG"));
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("SVG Files (*.svg)"));
    dialog.setDirectory(QDir::homePath());
    dialog.setStyleSheet(
        "QFileDialog { background-color: #f5f5f7; }"
        "QPushButton { border-radius: 4px; padding: 6px 12px; }"
        "QPushButton[text=\"Open\"], QPushButton[text=\"OK\"] { background-color: #007aff; color: white; }"
        "QPushButton[text=\"Cancel\"] { background-color: #f5f5f7; border: 1px solid #ccc; }"
        "QComboBox, QLineEdit { border: 1px solid #ccc; border-radius: 4px; padding: 4px; }"
    );
    if (!dialog.exec()) {
        return;
    }
    QString filePath = dialog.selectedFiles().first();
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
    bool success = m_drawingArea->importFromSvg(filePath);
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
    m_statusBar = new QStatusBar(this);
    m_statusBar->setStyleSheet(
        "QStatusBar { background-color: #f5f5f7; border-top: 1px solid #e0e0e0; padding: 3px; }"
        "QStatusBar QLabel { color: #555; }"
        "QStatusBar::item { border: none; }" 
    );
    setStatusBar(m_statusBar);
    m_shapesCountLabel = new QLabel(tr("Number of shapes: 0"));
    m_shapesCountLabel->setStyleSheet("font-size: 12px;");
    m_statusBar->addWidget(m_shapesCountLabel);
    QWidget* spacer = new QWidget();
    spacer->setFixedWidth(20);
    m_statusBar->addWidget(spacer);
    m_zoomLabel = new QPushButton(tr("Zoom level:100%"));
    m_zoomLabel->setStyleSheet(
        "QPushButton { font-size: 12px; color: #555; background-color: #f5f5f7; border: none; text-align: left; padding: 0px 5px; }"
        "QPushButton:hover { background-color: #e0e0e0; }"
    );
    m_zoomLabel->setToolTip(tr("Click to reset zoom"));
    m_zoomLabel->setCursor(Qt::PointingHandCursor);
    m_statusBar->addWidget(m_zoomLabel);
    connect(m_zoomLabel, &QPushButton::clicked, this, [this]() {
        if (m_drawingArea) {
            m_drawingArea->setScale(1.0);
            updateZoomSlider();
        }
    });
    QWidget* sliderSpacer = new QWidget();
    sliderSpacer->setFixedWidth(10);
    m_statusBar->addWidget(sliderSpacer);
    m_zoomSlider = new QSlider(Qt::Horizontal);
    m_zoomSlider->setMinimumWidth(150);
    m_zoomSlider->setMaximumWidth(200);
    m_zoomSlider->setMinimum(0);
    m_zoomSlider->setMaximum(100);
    m_zoomSlider->setTickInterval(10);
    m_zoomSlider->setTickPosition(QSlider::TicksBelow);
    m_statusBar->addWidget(m_zoomSlider);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &MainWindow::onZoomSliderValueChanged);
}
void MainWindow::updateStatusBarInfo()
{
    if (m_drawingArea) {
        int shapeCount = m_drawingArea->getShapesCount();
        m_shapesCountLabel->setText(tr("Number of shapes: %1").arg(shapeCount));
        double zoomPercent = m_drawingArea->getScale() * 100.0;
        m_zoomLabel->setText(tr("Zoom level: %1%").arg(zoomPercent, 0, 'f', 1));
    }
}
void MainWindow::updateZoomSlider()
{
    if (m_drawingArea && m_zoomSlider) {
        qreal currentScale = m_drawingArea->getScale();
        qreal minScale = m_drawingArea->MIN_SCALE;
        qreal maxScale = m_drawingArea->MAX_SCALE;
        int sliderPos = static_cast<int>((currentScale - minScale) / (maxScale - minScale) * 100);
        m_zoomSlider->blockSignals(true);
        m_zoomSlider->setValue(sliderPos);
        m_zoomSlider->blockSignals(false);
        double zoomPercent = currentScale * 100.0;
        m_zoomLabel->setText(tr("Zoom level: %1%").arg(zoomPercent, 0, 'f', 1));
    }
}
void MainWindow::onZoomSliderValueChanged(int value)
{
    if (m_drawingArea) {
        qreal minScale = m_drawingArea->MIN_SCALE;
        qreal maxScale = m_drawingArea->MAX_SCALE;
        qreal newScale = minScale + (value / 100.0) * (maxScale - minScale);
        m_drawingArea->setScale(newScale);
        double zoomPercent = newScale * 100.0;
        m_zoomLabel->setText(tr("Zoom level: %1%").arg(zoomPercent, 0, 'f', 1));
    }
}
void MainWindow::onFillColorButtonClicked()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        QColor initialColor = selectedShape->fillColor();
        QColor color = QColorDialog::getColor(initialColor, this, tr("Select Fill Color"));
        if (color.isValid()) {
            m_drawingArea->setSelectedShapeFillColor(color);
            Utils::updateColorButton(m_fillColorButton, color);
        }
    }
}
void MainWindow::onLineColorButtonClicked()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        QColor initialColor = selectedShape->lineColor();
        QColor color = QColorDialog::getColor(initialColor, this, tr("Select Line Color"));
        if (color.isValid()) {
            m_drawingArea->setSelectedShapeLineColor(color);
            Utils::updateColorButton(m_lineColorButton, color);
        }
    }
}
void MainWindow::updateColorButtons()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (!selectedShape) {
        return;
    }
    QColor fontColor = selectedShape->fontColor();
    Utils::updateColorButton(m_fontColorButton, fontColor);
    QColor fillColor = selectedShape->fillColor();
    Utils::updateColorButton(m_fillColorButton, fillColor);
    QColor lineColor = selectedShape->lineColor();
    Utils::updateColorButton(m_lineColorButton, lineColor);
}
void MainWindow::onTransparencyChanged(int index)
{
    int transparency = index * 10;
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        selectedShape->setTransparency(transparency);
        m_drawingArea->update();
    }
}
void MainWindow::onLineWidthChanged(int index)
{
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
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        selectedShape->setLineWidth(lineWidth);
        m_drawingArea->update();
    }
}
void MainWindow::onLineStyleChanged(int index)
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    if (selectedShape) {
        selectedShape->setLineStyle(index);
        m_drawingArea->update();
    }
}
void MainWindow::updateArrangeControls()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    bool hasSelection = (selectedShape != nullptr);
    m_bringToFrontButton->setEnabled(hasSelection);
    m_sendToBackButton->setEnabled(hasSelection);
    m_bringForwardButton->setEnabled(hasSelection);
    m_sendBackwardButton->setEnabled(hasSelection);
    updateShapePositionSizeControls();
}
void MainWindow::updateShapePositionSizeControls()
{
    Shape* selectedShape = m_drawingArea->getSelectedShape();
    bool hasSelection = (selectedShape != nullptr);
    m_xSpinBox->setEnabled(hasSelection);
    m_ySpinBox->setEnabled(hasSelection);
    m_widthSpinBox->setEnabled(hasSelection);
    m_heightSpinBox->setEnabled(hasSelection);
    if (hasSelection) {
        QRect rect = selectedShape->getRect();
        m_xSpinBox->blockSignals(true);
        m_ySpinBox->blockSignals(true);
        m_widthSpinBox->blockSignals(true);
        m_heightSpinBox->blockSignals(true);
        m_xSpinBox->setValue(rect.x());
        m_ySpinBox->setValue(rect.y());
        m_widthSpinBox->setValue(rect.width());
        m_heightSpinBox->setValue(rect.height());
        m_xSpinBox->blockSignals(false);
        m_ySpinBox->blockSignals(false);
        m_widthSpinBox->blockSignals(false);
        m_heightSpinBox->blockSignals(false);
    }
}
void MainWindow::onXCoordChanged(int value)
{
    m_drawingArea->setSelectedShapeX(value);
}
void MainWindow::onYCoordChanged(int value)
{
    m_drawingArea->setSelectedShapeY(value);
}
void MainWindow::onWidthChanged(int value)
{
    m_drawingArea->setSelectedShapeWidth(value);
}
void MainWindow::onHeightChanged(int value)
{
    m_drawingArea->setSelectedShapeHeight(value);
}
