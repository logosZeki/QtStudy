#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTabBar>
#include <QToolBar>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QAction>
#include <QStatusBar>
#include <QSlider>
#include <QFrame>

class ToolBar;
class DrawingArea;
class PageSettingDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTabBarClicked(int index);
    void showPageSettingDialog();
    void applyPageSettings();
    
    // 字体设置相关槽函数
    void updateFontControls();
    void onFontFamilyChanged(const QString& family);
    void onFontSizeChanged(const QString& sizeText);
    void onBoldActionTriggered();
    void onItalicActionTriggered();
    void onUnderlineActionTriggered();
    void onFontColorButtonClicked();
    void onAlignmentChanged(int index);
    void updateFontColorButton(const QColor& color);
    
    // 填充颜色和线条颜色按钮槽函数
    void onFillColorButtonClicked();
    void onLineColorButtonClicked();
    void updateColorButtons();
    
    // 图形排列相关槽函数
    void updateArrangeControls();
    
    // 透明度、线条粗细和线条样式相关槽函数
    void onTransparencyChanged(int index);
    void onLineWidthChanged(int index);
    void onLineStyleChanged(int index);
    
    // 导出功能相关槽函数
    void exportAsPng();
    // SVG导出与导入功能相关槽函数
    void exportAsSvg();
    void importFromSvg();
    
    // 状态栏相关槽函数
    void updateStatusBarInfo();
    void onZoomSliderValueChanged(int value);
    void updateZoomSlider();
    
    // 图形属性位置和尺寸控制槽函数
    void onXCoordChanged(int value);
    void onYCoordChanged(int value);
    void onWidthChanged(int value);
    void onHeightChanged(int value);
    void updateShapePositionSizeControls();

private:
    void createTopToolbar();
    void createMainToolbar();
    void createArrangeToolbar();
    void createExportAndImportToolbar();
    void createStatusBar();
    void createTitleBar();
    void setupUi();
    
    bool eventFilter(QObject *watched, QEvent *event) override;
    
private:
    ToolBar *m_toolBar;
    DrawingArea *m_drawingArea;
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_contentLayout;
    
    // 顶部工具栏
    QToolBar *m_topToolbar;
    QTabBar *m_tabBar;
    
    // 主工具栏 (对应"开始"选项卡)
    QToolBar *m_mainToolbar;
    
    // 排列工具栏 (对应"排列"选项卡)
    QToolBar *m_arrangeToolbar;
    
    // 排列栏目按钮
    QPushButton* m_bringToFrontButton;
    QPushButton* m_sendToBackButton;
    QPushButton* m_bringForwardButton;
    QPushButton* m_sendBackwardButton;
    
    // 导出工具栏 (对应"导出"选项卡)
    QToolBar *m_exportAndImportToolbar;
    
    // 页面设置对话框
    PageSettingDialog *m_pageSettingDialog;
    QPushButton *m_pageSettingButton;
    
    // 字体设置控件
    QComboBox *m_fontCombo;
    QComboBox *m_fontSizeCombo;
    QAction *m_boldAction;
    QAction *m_italicAction;
    QAction *m_underlineAction;
    QPushButton *m_fontColorButton;
    QFrame *m_fontColorIndicator;
    QPushButton  *m_fillColorButton;
    QFrame *m_fillColorIndicator;
    QPushButton *m_lineColorButton;
    QFrame *m_lineColorIndicator;
    QComboBox *m_alignCombo;
    
    // 透明度、线条粗细和线条样式控件
    QComboBox *m_transparencyCombo;
    QComboBox *m_lineWidthCombo;
    QComboBox *m_lineStyleCombo;
    
    // 状态栏控件
    QStatusBar *m_statusBar;
    QLabel *m_shapesCountLabel;
    QLabel *m_zoomLabel;
    QSlider *m_zoomSlider;
    
    // 图形位置和尺寸控制
    QSpinBox* m_xSpinBox;
    QSpinBox* m_ySpinBox;
    QSpinBox* m_widthSpinBox;
    QSpinBox* m_heightSpinBox;
};

#endif // MAINWINDOW_H
