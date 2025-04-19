#include "componentlistwidget.h"
#include <QApplication> // for QApplication::startDragDistance()
#include <QDrag>
#include <QMimeData>
#include <QDebug>

// 再次定义 MIME 类型，确保一致
extern const QString FLOWCHART_ITEM_MIME_TYPE; // 使用在 view 中定义的常量

ComponentListWidget::ComponentListWidget(QWidget *parent)
    : QListWidget(parent)
{
    // 基本设置
    setIconSize(QSize(32, 32)); // 设置图标大小
    setDragEnabled(false);      // 我们手动处理拖动逻辑
    setDropIndicatorShown(false); // 不显示放置指示器（我们只拖出，不拖入）
    setViewMode(QListView::IconMode); // 可以设置为 IconMode 或 ListMode
    setResizeMode(QListView::Adjust); // 自动调整
    setMovement(QListView::Static);   // 禁止内部拖动排序
}

void ComponentListWidget::addComponent(const QString &name, const QString &typeId, const QIcon &icon)
{
    QListWidgetItem *item = new QListWidgetItem(icon, name, this);
    // 将组件的类型 ID 存储在 item 的 data 中，以便拖动时获取
    item->setData(Qt::UserRole, typeId);
}

void ComponentListWidget::mousePressEvent(QMouseEvent *event)
{
    // 只处理左键按下
    if (event->button() == Qt::LeftButton) {
        startDragPos = event->pos(); // 记录起始位置
    }
    QListWidget::mousePressEvent(event); // 调用基类处理选中等
}

void ComponentListWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 确保是左键按下的移动，并且移动距离超过系统阈值
    if (!(event->buttons() & Qt::LeftButton)) {
        return; // 不是左键拖动则忽略
    }
    if ((event->pos() - startDragPos).manhattanLength() < QApplication::startDragDistance()) {
        return; // 移动距离不够则忽略
    }

    // 获取当前鼠标下的 Item
    QListWidgetItem *currentItem = itemAt(startDragPos); // 使用起始位置获取，避免移动时取错
    if (!currentItem) {
        return; // 如果没点中 item 则忽略
    }

    // 创建拖动对象
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    // 从 Item 中获取存储的类型 ID
    QString typeId = currentItem->data(Qt::UserRole).toString();
    QByteArray itemData = typeId.toUtf8(); // 转换为字节数组

    // 设置 MIME 数据
    mimeData->setData(FLOWCHART_ITEM_MIME_TYPE, itemData);
    mimeData->setText(currentItem->text()); // 可以附带文本信息

    drag->setMimeData(mimeData);

    // 设置拖动时显示的小图标 (可以是 Item 的图标或截图)
    drag->setPixmap(currentItem->icon().pixmap(32, 32)); // 使用 Item 图标
    drag->setHotSpot(QPoint(16, 16)); // 设置拖动图标的热点 (鼠标指针位置)

    qDebug() << "Starting drag for type:" << typeId;

    // 执行拖动操作 (CopyAction 表示复制一个新的组件到编辑区)
    drag->exec(Qt::CopyAction);

    // 注意: QDrag::exec 是阻塞的，代码会在这里暂停直到拖放操作完成或取消
}
