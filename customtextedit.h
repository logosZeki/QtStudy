#ifndef CUSTOMTEXTEDIT_H
#define CUSTOMTEXTEDIT_H

#include <QTextEdit>
#include <QResizeEvent>

class CustomTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit CustomTextEdit(QWidget *parent = nullptr);
    
protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    
private slots:
    void performCenterCursor();

private:
    void centerCursor();
    bool m_centerPending;
};

#endif // CUSTOMTEXTEDIT_H
