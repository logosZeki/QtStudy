#include "pagesettingdialog.h"
#include <QPainter>
#include <QPixmap>
#include <QDialogButtonBox>
#include "util/Utils.h"

// 定义A4尺寸（像素）
constexpr int A4_WIDTH = 1050;    // A4宽度为1050像素
constexpr int A4_HEIGHT = 1500;   // A4高度为1500像素

// 定义A3尺寸（像素）
constexpr int A3_WIDTH = 1500;    // A3宽度为1500像素
constexpr int A3_HEIGHT = 2100;   // A3高度为2100像素

// 定义A5尺寸（像素）
constexpr int A5_WIDTH = 750;    // A5宽度为750像素
constexpr int A5_HEIGHT = 1050;   // A5高度为1050像素

constexpr int Default_WIDTH = Utils::Default_WIDTH; 
constexpr int Default_HEIGHT = Utils::Default_HEIGHT;

PageSettingDialog::PageSettingDialog(QWidget *parent)
    : QDialog(parent)
    , m_colorDialog(nullptr)
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
    connect(m_paperSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PageSettingDialog::onPaperSizeChanged);
    connect(m_gridColorButton, &QPushButton::clicked, this, &PageSettingDialog::onGridColorClicked);
    connect(m_customSizeCheck, &QCheckBox::toggled, this, &PageSettingDialog::onCustomSizeToggled);
    connect(m_showGridCheck, &QCheckBox::toggled, this, &PageSettingDialog::onShowGridToggled);
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
    
    // 默认页面尺寸为A4
    m_pageSize = QSize(Default_WIDTH, Default_HEIGHT);
    
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
    m_colorGroupBox = new QGroupBox(tr("页面背景颜色"));
    QGridLayout *colorLayout = new QGridLayout(m_colorGroupBox);
    
    m_colorButton = new QPushButton(tr("选择颜色..."));
    m_colorPreview = new QLabel();
    m_colorPreview->setFixedSize(40, 20);
    m_colorPreview->setAutoFillBackground(true);
    QPalette pal = m_colorPreview->palette();
    pal.setColor(QPalette::Window, m_backgroundColor);
    m_colorPreview->setPalette(pal);
    
    m_recentColorsCombo = new QComboBox();
    m_recentColorsCombo->addItem(tr("最近使用的颜色"));
    for (const QColor &color : m_recentColors) {
        QPixmap pixmap(16, 16);
        pixmap.fill(color);
        m_recentColorsCombo->addItem(QIcon(pixmap), color.name());
    }
    
    colorLayout->addWidget(new QLabel(tr("当前颜色:")), 0, 0);
    colorLayout->addWidget(m_colorPreview, 0, 1);
    colorLayout->addWidget(m_colorButton, 0, 2);
    colorLayout->addWidget(new QLabel(tr("最近使用:")), 1, 0);
    colorLayout->addWidget(m_recentColorsCombo, 1, 1, 1, 2);
    
    // 2. 页面尺寸设置组
    m_sizeGroupBox = new QGroupBox(tr("页面尺寸"));
    QGridLayout *sizeLayout = new QGridLayout(m_sizeGroupBox);
    
    m_paperSizeCombo = new QComboBox();
    m_paperSizeCombo->addItem(tr("A3 (297mm x 420mm)"), QSize(A3_WIDTH, A3_HEIGHT));
    m_paperSizeCombo->addItem(tr("A4 (210mm x 297mm)"), QSize(A4_WIDTH, A4_HEIGHT));
    m_paperSizeCombo->addItem(tr("A5 (148mm x 210mm)"), QSize(A5_WIDTH, A5_HEIGHT));
    m_paperSizeCombo->setCurrentIndex(1); // 默认选择A4
    
    m_customSizeCheck = new QCheckBox(tr("自定义尺寸"));
    
    m_widthSpin = new QSpinBox();
    m_widthSpin->setRange(100, 2000);
    m_widthSpin->setValue(m_pageSize.width());
    m_widthSpin->setSuffix(tr(" px"));
    m_widthSpin->setEnabled(false);
    
    m_heightSpin = new QSpinBox();
    m_heightSpin->setRange(100, 2000);
    m_heightSpin->setValue(m_pageSize.height());
    m_heightSpin->setSuffix(tr(" px"));
    m_heightSpin->setEnabled(false);
    
    m_pixelInfoLabel = new QLabel(tr("像素值参考: A4 = 1050 x 1500 px"));
    
    sizeLayout->addWidget(new QLabel(tr("预设尺寸:")), 0, 0);
    sizeLayout->addWidget(m_paperSizeCombo, 0, 1, 1, 2);
    sizeLayout->addWidget(m_customSizeCheck, 1, 0, 1, 3);
    sizeLayout->addWidget(new QLabel(tr("宽度:")), 2, 0);
    sizeLayout->addWidget(m_widthSpin, 2, 1);
    sizeLayout->addWidget(new QLabel(tr("高度:")), 3, 0);
    sizeLayout->addWidget(m_heightSpin, 3, 1);
    sizeLayout->addWidget(m_pixelInfoLabel, 4, 0, 1, 3);
    
    // 3. 网格控制设置组
    m_gridGroupBox = new QGroupBox(tr("页面网格控制"));
    QGridLayout *gridLayout = new QGridLayout(m_gridGroupBox);
    
    m_showGridCheck = new QCheckBox(tr("显示网格"));
    m_showGridCheck->setChecked(m_showGrid);
    
    m_gridColorButton = new QPushButton(tr("网格颜色..."));
    m_gridColorPreview = new QLabel();
    m_gridColorPreview->setFixedSize(40, 20);
    m_gridColorPreview->setAutoFillBackground(true);
    QPalette gridPal = m_gridColorPreview->palette();
    gridPal.setColor(QPalette::Window, m_gridColor);
    m_gridColorPreview->setPalette(gridPal);
    
    m_gridSizeSpin = new QSpinBox();
    m_gridSizeSpin->setRange(5, 100);
    m_gridSizeSpin->setValue(m_gridSize);
    m_gridSizeSpin->setSuffix(tr(" px"));
    
    m_gridThicknessSpin = new QSpinBox();
    m_gridThicknessSpin->setRange(1, 5);
    m_gridThicknessSpin->setValue(m_gridThickness);
    m_gridThicknessSpin->setSuffix(tr(" px"));
    
    gridLayout->addWidget(m_showGridCheck, 0, 0, 1, 3);
    gridLayout->addWidget(new QLabel(tr("网格颜色:")), 1, 0);
    gridLayout->addWidget(m_gridColorPreview, 1, 1);
    gridLayout->addWidget(m_gridColorButton, 1, 2);
    gridLayout->addWidget(new QLabel(tr("网格大小:")), 2, 0);
    gridLayout->addWidget(m_gridSizeSpin, 2, 1, 1, 2);
    gridLayout->addWidget(new QLabel(tr("线条粗细:")), 3, 0);
    gridLayout->addWidget(m_gridThicknessSpin, 3, 1, 1, 2);
    
    // 将设置组添加到左侧布局
    settingsLayout->addWidget(m_colorGroupBox);
    settingsLayout->addWidget(m_sizeGroupBox);
    settingsLayout->addWidget(m_gridGroupBox);
    settingsLayout->addStretch();
    
    // 将设置布局添加到内容布局
    contentLayout->addLayout(settingsLayout);
    
    // 创建按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton(tr("确定"));
    m_cancelButton = new QPushButton(tr("取消"));
    m_applyButton = new QPushButton(tr("应用"));
    
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
    // 网格设置
    m_showGrid = m_showGridCheck->isChecked();
    m_gridSize = m_gridSizeSpin->value();
    m_gridThickness = m_gridThicknessSpin->value();
    
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

void PageSettingDialog::onPaperSizeChanged(int index)
{
    if (index >= 0) {
        QSize selectedSize = m_paperSizeCombo->itemData(index).toSize();
        
        // 更新像素值参考标签
        m_pixelInfoLabel->setText(
            tr("像素值参考: %1 = %2 x %3 px")
            .arg(m_paperSizeCombo->currentText().section(" ", 0, 0))
            .arg(selectedSize.width())
            .arg(selectedSize.height())
        );
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
        int index = m_paperSizeCombo->currentIndex();
        m_pageSize = m_paperSizeCombo->itemData(index).toSize();
    }
}

void PageSettingDialog::onShowGridToggled(bool checked)
{
    m_showGrid = checked;
    m_gridColorButton->setEnabled(checked);
    m_gridSizeSpin->setEnabled(checked);
    m_gridThicknessSpin->setEnabled(checked);
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