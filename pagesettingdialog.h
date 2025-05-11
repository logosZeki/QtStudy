#ifndef PAGESETTINGDIALOG_H
#define PAGESETTINGDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QColorDialog>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>

class DrawingArea;

class PageSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PageSettingDialog(QWidget *parent = nullptr, DrawingArea* drawingArea = nullptr);
    ~PageSettingDialog();

    // 获取页面设置参数
    QColor getBackgroundColor() const;
    QSize getPageSize() const;
    bool getShowGrid() const;
    QColor getGridColor() const;
    int getGridSize() const;
    int getGridThickness() const;

signals:
    // 设置被应用时发出的信号
    void settingsApplied();

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onApplyClicked();
    void onSelectColorClicked();
    void onRecentColorSelected(int index);
    void onGridColorClicked();
    void onCustomSizeToggled(bool checked);
    void onShowGridToggled(bool checked);
    void onPaperSizeRadioToggled(int id);
    void onGridSizeComboChanged(int index);
    void onLineThicknessComboChanged(int index);

private:
    void setupUi();
    void initDefaultValues();
    void updateRecentColors(const QColor &color);
    void updatePreview();
    void updatePixelInfoLabel();

    // 绘图区域指针
    DrawingArea* m_drawingArea;

    // 背景颜色设置相关控件
    QGroupBox *m_colorGroupBox;
    QPushButton *m_colorButton;
    QLabel *m_colorPreview;
    QComboBox *m_recentColorsCombo;
    QColorDialog *m_colorDialog;
    QColor m_backgroundColor;
    QList<QColor> m_recentColors;

    // 页面尺寸设置相关控件
    QGroupBox *m_sizeGroupBox;
    QComboBox *m_paperSizeCombo;
    QCheckBox *m_customSizeCheck;
    QSpinBox *m_widthSpin;
    QSpinBox *m_heightSpin;
    QLabel *m_pixelInfoLabel;
    QSize m_pageSize;
    QButtonGroup *m_paperSizeGroup;
    QRadioButton *m_a3Radio;
    QRadioButton *m_a4Radio;
    QRadioButton *m_a5Radio;
    QRadioButton *m_defaultSizeRadio;
    QRadioButton *m_customSizeRadio;

    // 网格设置相关控件
    QGroupBox *m_gridGroupBox;
    QCheckBox *m_showGridCheck;
    QPushButton *m_gridColorButton;
    QLabel *m_gridColorPreview;
    QSpinBox *m_gridSizeSpin;
    QSpinBox *m_gridThicknessSpin;
    QColor m_gridColor;
    int m_gridSize;
    int m_gridThickness;
    bool m_showGrid;
    QComboBox *m_gridSizeCombo;
    QComboBox *m_lineThicknessCombo;

    // 按钮
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QPushButton *m_applyButton;

    // 跟踪用户手动调整状态
    bool m_gridSizeModified;
    bool m_lineThicknessModified;
    bool m_pageSizeModified;
};

#endif // PAGESETTINGDIALOG_H 