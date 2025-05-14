#include "pagesettingdialog.h"
#include <QPainter>
#include <QPixmap>
#include <QDialogButtonBox>
#include "util/Utils.h"
#include "drawingarea.h"

// 定义A4尺寸（像素）
constexpr int A4_WIDTH = Utils::A4_WIDTH;    // A4宽度为1200像素
constexpr int A4_HEIGHT = Utils::A4_HEIGHT;   // A4高度为1700像素

// 定义A3尺寸（像素）
constexpr int A3_WIDTH = Utils::A3_WIDTH;    // A3宽度为1700像素
constexpr int A3_HEIGHT = Utils::A3_HEIGHT;   // A3高度为2400像素

// 定义A5尺寸（像素）
constexpr int A5_WIDTH =Utils::A5_WIDTH;
constexpr int A5_HEIGHT = Utils::A5_HEIGHT;

constexpr int Default_WIDTH = Utils::Default_WIDTH; 
constexpr int Default_HEIGHT = Utils::Default_HEIGHT;

PageSettingDialog::PageSettingDialog(QWidget *parent, DrawingArea* drawingArea)
    : QDialog(parent)
    , m_colorDialog(nullptr)
    , m_gridSizeModified(false)
    , m_lineThicknessModified(false)
    , m_pageSizeModified(false)
    , m_drawingArea(drawingArea)
{
    setWindowTitle(tr("页面设置"));
    setMinimumSize(400, 500);

    initDefaultValues();
    setupUi();

    // 连接信号槽
    connect(m_okButton, &QPushButton::clicked, this, &PageSettingDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &PageSettingDialog::onCancelClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &PageSettingDialog::onApplyClicked);
    connect(m_colorButton, &QPushButton::clicked, this, &PageSettingDialog::onSelectColorClicked);
    connect(m_recentColorsCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &PageSettingDialog::onRecentColorSelected);
    connect(m_gridColorButton, &QPushButton::clicked, this, &PageSettingDialog::onGridColorClicked);
    connect(m_customSizeCheck, &QCheckBox::toggled, this, &PageSettingDialog::onCustomSizeToggled);
    connect(m_showGridCheck, &QCheckBox::toggled, this, &PageSettingDialog::onShowGridToggled);
    connect(m_paperSizeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), 
            this, &PageSettingDialog::onPaperSizeRadioToggled);
    connect(m_gridSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &PageSettingDialog::onGridSizeComboChanged);
    connect(m_lineThicknessCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &PageSettingDialog::onLineThicknessComboChanged);
}

PageSettingDialog::~PageSettingDialog()
{
    if (m_colorDialog) {
        delete m_colorDialog;
    }
}

void PageSettingDialog::initDefaultValues()
{
    // 默认背景颜色为白色
    m_backgroundColor = Qt::white;
    
    // 从DrawingArea获取当前页面尺寸
    m_pageSize = m_drawingArea->getDrawingAreaSize();
    
    // 默认显示网格
    m_showGrid = true;
    
    // 默认网格颜色为浅灰色
    m_gridColor = QColor(220, 220, 220);
    
    // 默认网格大小为20像素
    m_gridSize = 20;
    
    // 默认网格线条粗细为1像素
    m_gridThickness = 1;
    
    // 初始化最近使用颜色列表
    m_recentColors.append(Qt::white);
    m_recentColors.append(QColor(240, 240, 240));
    m_recentColors.append(QColor(245, 245, 245));
    m_recentColors.append(QColor(250, 250, 250));
}

void PageSettingDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建左侧设置区域和右侧预览区域的水平布局
    QHBoxLayout *contentLayout = new QHBoxLayout();
    
    // 左侧设置区域
    QVBoxLayout *settingsLayout = new QVBoxLayout();
    
    // 1. 背景颜色设置组
    m_colorGroupBox = new QGroupBox(tr("Page background color"));
    QGridLayout *colorLayout = new QGridLayout(m_colorGroupBox);
    
    m_colorButton = new QPushButton(tr("Choose color..."));
    m_colorPreview = new QLabel();
    m_colorPreview->setFixedSize(40, 20);
    m_colorPreview->setAutoFillBackground(true);
    QPalette pal = m_colorPreview->palette();
    pal.setColor(QPalette::Window, m_backgroundColor);
    m_colorPreview->setPalette(pal);
    
    m_recentColorsCombo = new QComboBox();
    m_recentColorsCombo->addItem(tr("Recently used colors"));
    for (const QColor &color : m_recentColors) {
        QPixmap pixmap(16, 16);
        pixmap.fill(color);
        m_recentColorsCombo->addItem(QIcon(pixmap), color.name());
    }
    
    colorLayout->addWidget(new QLabel(tr("Current color:")), 0, 0);
    colorLayout->addWidget(m_colorPreview, 0, 1);
    colorLayout->addWidget(m_colorButton, 0, 2);
    colorLayout->addWidget(new QLabel(tr("Recently used:")), 1, 0);
    colorLayout->addWidget(m_recentColorsCombo, 1, 1, 1, 2);
    
    // 2. 页面尺寸设置组
    m_sizeGroupBox = new QGroupBox(tr("Page size"));
    QGridLayout *sizeLayout = new QGridLayout(m_sizeGroupBox);
    
    // 替换下拉框为单选按钮组
    m_paperSizeGroup = new QButtonGroup(this);
    m_a3Radio = new QRadioButton(tr("A3"));
    m_a4Radio = new QRadioButton(tr("A4"));
    m_a5Radio = new QRadioButton(tr("A5"));
    m_defaultSizeRadio = new QRadioButton(tr("Default size"));
    m_customSizeRadio = new QRadioButton(tr("Custom size"));
    
    m_paperSizeGroup->addButton(m_a3Radio, 0);
    m_paperSizeGroup->addButton(m_a4Radio, 1);
    m_paperSizeGroup->addButton(m_a5Radio, 2);
    m_paperSizeGroup->addButton(m_defaultSizeRadio, 3);
    m_paperSizeGroup->addButton(m_customSizeRadio, 4);
    
    // 默认选择A4
    m_a4Radio->setChecked(true);
    
    m_widthSpin = new QSpinBox();
    m_widthSpin->setRange(100, 2000);
    m_widthSpin->setValue(m_pageSize.width());
    m_widthSpin->setSuffix(tr(" px"));
    m_widthSpin->setEnabled(false);
    
    m_heightSpin = new QSpinBox();
    m_heightSpin->setRange(100, 2200);
    m_heightSpin->setValue(m_pageSize.height());
    m_heightSpin->setSuffix(tr(" px"));
    m_heightSpin->setEnabled(false);
    
    m_pixelInfoLabel = new QLabel();
    updatePixelInfoLabel();
    
    QHBoxLayout *paperSizeLayout = new QHBoxLayout();
    paperSizeLayout->addWidget(m_a3Radio);
    paperSizeLayout->addWidget(m_a4Radio);
    paperSizeLayout->addWidget(m_a5Radio);
    paperSizeLayout->addWidget(m_defaultSizeRadio);
    
    sizeLayout->addWidget(new QLabel(tr("Preset size:")), 0, 0);
    sizeLayout->addLayout(paperSizeLayout, 0, 1, 1, 2);
    sizeLayout->addWidget(m_customSizeRadio, 1, 0, 1, 3);
    sizeLayout->addWidget(new QLabel(tr("Width:")), 2, 0);
    sizeLayout->addWidget(m_widthSpin, 2, 1);
    sizeLayout->addWidget(new QLabel(tr("Height:")), 3, 0);
    sizeLayout->addWidget(m_heightSpin, 3, 1);
    sizeLayout->addWidget(m_pixelInfoLabel, 4, 0, 1, 3);
    
    // 保留旧的CheckBox，但隐藏，以便兼容旧代码
    m_customSizeCheck = new QCheckBox(tr("Custom size"));
    m_customSizeCheck->setVisible(false);
    connect(m_customSizeRadio, &QRadioButton::toggled, m_customSizeCheck, &QCheckBox::setChecked);
    
    // 3. 网格控制设置组
    m_gridGroupBox = new QGroupBox(tr("Page grid control"));
    QGridLayout *gridLayout = new QGridLayout(m_gridGroupBox);
    
    m_showGridCheck = new QCheckBox(tr("Show grid"));
    m_showGridCheck->setChecked(m_showGrid);
    
    m_gridColorButton = new QPushButton(tr("Grid color..."));
    m_gridColorPreview = new QLabel();
    m_gridColorPreview->setFixedSize(40, 20);
    m_gridColorPreview->setAutoFillBackground(true);
    QPalette gridPal = m_gridColorPreview->palette();
    gridPal.setColor(QPalette::Window, m_gridColor);
    m_gridColorPreview->setPalette(gridPal);
    
    // 替换网格大小Spinner为下拉框
    m_gridSizeCombo = new QComboBox();
    m_gridSizeCombo->addItem(tr("Small (10 px)"), 10);
    m_gridSizeCombo->addItem(tr("Normal (20 px)"), 20);
    m_gridSizeCombo->addItem(tr("Large (30 px)"), 30);
    m_gridSizeCombo->addItem(tr("Very large (40 px)"), 40);
    
    // 根据当前值设置默认选项
    if (m_gridSize == 10) {
        m_gridSizeCombo->setCurrentIndex(0);
    } else if (m_gridSize == 20) {
        m_gridSizeCombo->setCurrentIndex(1);
    } else if (m_gridSize == 30) {
        m_gridSizeCombo->setCurrentIndex(2);
    } else if (m_gridSize == 40) {
        m_gridSizeCombo->setCurrentIndex(3);
    } else {
        // 如果是其他值，设为正常
        m_gridSizeCombo->setCurrentIndex(1);
    }
    
    // 保留旧的Spinner，但隐藏，以便兼容旧代码
    m_gridSizeSpin = new QSpinBox();
    m_gridSizeSpin->setRange(5, 100);
    m_gridSizeSpin->setValue(m_gridSize);
    m_gridSizeSpin->setSuffix(tr(" px"));
    m_gridSizeSpin->setVisible(false);
    
    // 替换线条粗细Spinner为下拉框
    m_lineThicknessCombo = new QComboBox();
    m_lineThicknessCombo->addItem(tr("Thin (0.5 px)"), 0.5);
    m_lineThicknessCombo->addItem(tr("Normal (1 px)"), 1);
    m_lineThicknessCombo->addItem(tr("Thick (2 px)"), 2);
    
    // 根据当前值设置默认选项
    if (m_gridThickness == 0.5) {
        m_lineThicknessCombo->setCurrentIndex(0);
    } else if (m_gridThickness == 1) {
        m_lineThicknessCombo->setCurrentIndex(1);
    } else if (m_gridThickness == 2) {
        m_lineThicknessCombo->setCurrentIndex(2);
    } else {
        // 如果是其他值，设为正常
        m_lineThicknessCombo->setCurrentIndex(1);
    }
    
    // 保留旧的Spinner，但隐藏，以便兼容旧代码
    m_gridThicknessSpin = new QSpinBox();
    m_gridThicknessSpin->setRange(1, 5);
    m_gridThicknessSpin->setValue(m_gridThickness);
    m_gridThicknessSpin->setSuffix(tr(" px"));
    m_gridThicknessSpin->setVisible(false);
    
    gridLayout->addWidget(m_showGridCheck, 0, 0, 1, 3);
    gridLayout->addWidget(new QLabel(tr("Grid color:")), 1, 0);
    gridLayout->addWidget(m_gridColorPreview, 1, 1);
    gridLayout->addWidget(m_gridColorButton, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Grid size:")), 2, 0);
    gridLayout->addWidget(m_gridSizeCombo, 2, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("Line thickness:")), 3, 0);
    gridLayout->addWidget(m_lineThicknessCombo, 3, 1, 1, 2);
    
    // 将设置组添加到左侧布局
    settingsLayout->addWidget(m_colorGroupBox);
    settingsLayout->addWidget(m_sizeGroupBox);
    settingsLayout->addWidget(m_gridGroupBox);
    settingsLayout->addStretch();
    
    // 将设置布局添加到内容布局
    contentLayout->addLayout(settingsLayout);
    
    // 创建按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton(tr("OK"));
    m_cancelButton = new QPushButton(tr("Cancel"));
    m_applyButton = new QPushButton(tr("Apply"));
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_applyButton);
    
    // 将内容和按钮添加到主布局
    mainLayout->addLayout(contentLayout, 1);
    mainLayout->addLayout(buttonLayout);
}

void PageSettingDialog::updatePreview()
{
    // 预览功能已被移除，此方法不再需要
}

void PageSettingDialog::updatePixelInfoLabel()
{
    // 使用DrawingArea的实际尺寸(如果可用)
    if (m_drawingArea) {
        QSize drawingAreaSize = m_drawingArea->getDrawingAreaSize();
        m_pixelInfoLabel->setText(tr("Current size: %1 x %2 px").arg(drawingAreaSize.width()).arg(drawingAreaSize.height()));
    } else {
        m_pixelInfoLabel->setText(tr("Current size: %1 x %2 px").arg(m_pageSize.width()).arg(m_pageSize.height()));
    }
}

QColor PageSettingDialog::getBackgroundColor() const
{
    return m_backgroundColor;
}

QSize PageSettingDialog::getPageSize() const
{
    return m_pageSize;
}

bool PageSettingDialog::getShowGrid() const
{
    return m_showGrid;
}

QColor PageSettingDialog::getGridColor() const
{
    return m_gridColor;
}

int PageSettingDialog::getGridSize() const
{
    return m_gridSize;
}

int PageSettingDialog::getGridThickness() const
{
    return m_gridThickness;
}

void PageSettingDialog::onOkClicked()
{
    // 应用设置并关闭对话框
    onApplyClicked();
    accept();
}

void PageSettingDialog::onCancelClicked()
{
    // 关闭对话框，不应用设置
    reject();
}

void PageSettingDialog::onApplyClicked()
{
    // 应用设置
    // 页面尺寸 - 只有在用户手动调整过时才应用新设置
    if (m_pageSizeModified) {
        if (m_customSizeRadio->isChecked()) {
            m_pageSize = QSize(m_widthSpin->value(), m_heightSpin->value());
        } else {
            // 根据选择的预设尺寸设置页面大小
            if (m_a3Radio->isChecked()) {
                m_pageSize = QSize(A3_WIDTH, A3_HEIGHT);
            } else if (m_a4Radio->isChecked()) {
                m_pageSize = QSize(A4_WIDTH, A4_HEIGHT);
            } else if (m_a5Radio->isChecked()) {
                m_pageSize = QSize(A5_WIDTH, A5_HEIGHT);
            } else if (m_defaultSizeRadio->isChecked()) {
                m_pageSize = QSize(Default_WIDTH, Default_HEIGHT);
            }
        }
        
        // 应用页面尺寸到绘图区域
        if (m_drawingArea) {
            m_drawingArea->setPageSize(m_pageSize);
            m_drawingArea->setDrawingAreaSize(m_pageSize); // 同时更新绘图区域尺寸
            // 不需要显式调用centerDrawingArea()，因为setDrawingAreaSize已经会调用它
        }
    }
    
    // 应用背景颜色
    if (m_drawingArea) {
        m_drawingArea->setBackgroundColor(m_backgroundColor);
    }
    
    // 网格设置 - 只有在用户手动调整过时才应用新设置
    m_showGrid = m_showGridCheck->isChecked();
    if (m_drawingArea) {
        m_drawingArea->setShowGrid(m_showGrid);
        m_drawingArea->setGridColor(m_gridColor);
    }
    
    if (m_gridSizeModified) {
        m_gridSize = m_gridSizeCombo->currentData().toInt();
        if (m_drawingArea) {
            m_drawingArea->setGridSize(m_gridSize);
        }
    }
    
    if (m_lineThicknessModified) {
        m_gridThickness = m_lineThicknessCombo->currentData().toDouble();
        if (m_drawingArea) {
            m_drawingArea->setGridThickness(m_gridThickness);
        }
    }
    
    // 发出设置已应用信号
    emit settingsApplied();
}

void PageSettingDialog::onSelectColorClicked()
{
    if (!m_colorDialog) {
        m_colorDialog = new QColorDialog(this);
    }
    
    m_colorDialog->setCurrentColor(m_backgroundColor);
    if (m_colorDialog->exec() == QDialog::Accepted) {
        m_backgroundColor = m_colorDialog->currentColor();
        updateRecentColors(m_backgroundColor);
        
        // 更新颜色预览
        QPalette pal = m_colorPreview->palette();
        pal.setColor(QPalette::Window, m_backgroundColor);
        m_colorPreview->setPalette(pal);
    }
}

void PageSettingDialog::onRecentColorSelected(int index)
{
    if (index > 0 && index <= m_recentColors.size()) {
        m_backgroundColor = m_recentColors.at(index - 1);
        
        // 更新颜色预览
        QPalette pal = m_colorPreview->palette();
        pal.setColor(QPalette::Window, m_backgroundColor);
        m_colorPreview->setPalette(pal);
    }
}

void PageSettingDialog::onGridColorClicked()
{
    if (!m_colorDialog) {
        m_colorDialog = new QColorDialog(this);
    }
    
    m_colorDialog->setCurrentColor(m_gridColor);
    if (m_colorDialog->exec() == QDialog::Accepted) {
        m_gridColor = m_colorDialog->currentColor();
        
        // 更新网格颜色预览
        QPalette pal = m_gridColorPreview->palette();
        pal.setColor(QPalette::Window, m_gridColor);
        m_gridColorPreview->setPalette(pal);
    }
}

void PageSettingDialog::onCustomSizeToggled(bool checked)
{
    m_widthSpin->setEnabled(checked);
    m_heightSpin->setEnabled(checked);
    
    if (checked) {
        // 如果启用自定义尺寸，则使用微调框的值
        m_widthSpin->setValue(m_pageSize.width());
        m_heightSpin->setValue(m_pageSize.height());
    } else {
        // 如果禁用自定义尺寸，则使用预设尺寸
        if (m_a3Radio->isChecked()) {
            m_pageSize = QSize(A3_WIDTH, A3_HEIGHT);
        } else if (m_a4Radio->isChecked()) {
            m_pageSize = QSize(A4_WIDTH, A4_HEIGHT);
        } else if (m_a5Radio->isChecked()) {
            m_pageSize = QSize(A5_WIDTH, A5_HEIGHT);
        } else if (m_defaultSizeRadio->isChecked()) {
            m_pageSize = QSize(Default_WIDTH, Default_HEIGHT);
        }
    }
    
    m_pageSizeModified = true;
}

void PageSettingDialog::onShowGridToggled(bool checked)
{
    m_showGrid = checked;
    m_gridColorButton->setEnabled(checked);
    m_gridSizeCombo->setEnabled(checked);
    m_lineThicknessCombo->setEnabled(checked);
}

void PageSettingDialog::updateRecentColors(const QColor &color)
{
    // 检查颜色是否已在列表中
    int existingIndex = -1;
    for (int i = 0; i < m_recentColors.size(); ++i) {
        if (m_recentColors[i] == color) {
            existingIndex = i;
            break;
        }
    }
    
    // 如果颜色已存在，则将其移到列表前面
    if (existingIndex >= 0) {
        m_recentColors.move(existingIndex, 0);
    } else {
        // 否则，将颜色添加到列表前面
        m_recentColors.prepend(color);
        
        // 保持列表最多10个颜色
        if (m_recentColors.size() > 10) {
            m_recentColors.removeLast();
        }
    }
    
    // 更新下拉框
    m_recentColorsCombo->clear();
    m_recentColorsCombo->addItem(tr("最近使用的颜色"));
    for (const QColor &recentColor : m_recentColors) {
        QPixmap pixmap(16, 16);
        pixmap.fill(recentColor);
        m_recentColorsCombo->addItem(QIcon(pixmap), recentColor.name());
    }
}

void PageSettingDialog::onPaperSizeRadioToggled(int id)
{
    switch (id) {
        case 0: // A3
            m_pageSize = QSize(A3_WIDTH, A3_HEIGHT);
            m_widthSpin->setValue(A3_WIDTH);
            m_heightSpin->setValue(A3_HEIGHT);
            m_widthSpin->setEnabled(false);
            m_heightSpin->setEnabled(false);
            break;
        
        case 1: // A4
            m_pageSize = QSize(A4_WIDTH, A4_HEIGHT);
            m_widthSpin->setValue(A4_WIDTH);
            m_heightSpin->setValue(A4_HEIGHT);
            m_widthSpin->setEnabled(false);
            m_heightSpin->setEnabled(false);
            break;
        
        case 2: // A5
            m_pageSize = QSize(A5_WIDTH, A5_HEIGHT);
            m_widthSpin->setValue(A5_WIDTH);
            m_heightSpin->setValue(A5_HEIGHT);
            m_widthSpin->setEnabled(false);
            m_heightSpin->setEnabled(false);
            break;
            
        case 3: // 默认尺寸
            m_pageSize = QSize(Default_WIDTH, Default_HEIGHT);
            m_widthSpin->setValue(Default_WIDTH);
            m_heightSpin->setValue(Default_HEIGHT);
            m_widthSpin->setEnabled(false);
            m_heightSpin->setEnabled(false);
            break;
            
        case 4: // 自定义尺寸
            // 使用当前设置的SpinBox值
            m_pageSize = QSize(m_widthSpin->value(), m_heightSpin->value());
            m_widthSpin->setEnabled(true);
            m_heightSpin->setEnabled(true);
            break;
    }
    
    m_pageSizeModified = true;
    updatePixelInfoLabel();
    
    // 立即预览尺寸变化（如果有绘图区域指针）
    if (m_drawingArea) {
        m_drawingArea->setPageSize(m_pageSize);
        m_drawingArea->setDrawingAreaSize(m_pageSize); // 同时更新绘图区域尺寸
        // 不需要显式调用centerDrawingArea()，因为setDrawingAreaSize已经会调用它
    }
}

void PageSettingDialog::onGridSizeComboChanged(int index)
{
    // 用户修改网格大小，更新对应的SpinBox值并标记为已修改
    m_gridSizeSpin->setValue(m_gridSizeCombo->currentData().toInt());
    m_gridSizeModified = true;
}

void PageSettingDialog::onLineThicknessComboChanged(int index)
{
    // 用户修改线条粗细，更新对应的SpinBox值并标记为已修改
    m_gridThicknessSpin->setValue(m_lineThicknessCombo->currentData().toDouble());
    m_lineThicknessModified = true;
} 