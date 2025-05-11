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
    
    // 导出功能相关槽函数
    void exportAsPng();

private:
    void createTopToolbar();
    void createMainToolbar();
    void createArrangeToolbar();
    void createExportAndImportToolbar();
    void setupUi();
    
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
    QComboBox *m_alignCombo;
};

#endif // MAINWINDOW_H
