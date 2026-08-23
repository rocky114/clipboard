#include "mainwindow.h"

#include <QCloseEvent>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>

#include "clipboardmanager.h"
#include "clipboardstore.h"

MainWindow::MainWindow(ClipboardStore *store, ClipboardManager *manager,
                       QWidget *parent)
    : QMainWindow(parent)
    , m_store(store)
    , m_manager(manager)
{
    setWindowTitle(QStringLiteral("剪贴板历史"));
    resize(520, 480);

    m_list = new QListWidget(this);
    setCentralWidget(m_list);
    connect(m_list, &QListWidget::itemActivated,
            this, &MainWindow::onItemActivated);

    setupContextMenu();
}

void MainWindow::showAndRefresh()
{
    refreshList(); // 每次展示前从文件重新读取最新历史
    show();
    raise();
    activateWindow();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 关闭窗口不退出应用,只是隐藏,继续驻留托盘监听剪贴板
    event->ignore();
    hide();
}

void MainWindow::refreshList()
{
    m_store->loadFromJson();
    m_list->clear();
    for (const ClipboardItem &item : m_store->items()) {
        // 预览:换行折叠成空格,超长省略;tooltip 显示全文
        const QString preview =
            item.text.left(80).replace(QLatin1Char('\n'), QLatin1Char(' '));
        auto *row = new QListWidgetItem(
            QStringLiteral("%1  %2")
                .arg(item.timestamp.toString(QStringLiteral("MM-dd HH:mm:ss")),
                     preview));
        row->setToolTip(item.text);
        m_list->addItem(row);
    }
}

void MainWindow::onItemActivated(QListWidgetItem *item)
{
    const int index = m_list->row(item);
    if (index < 0 || index >= m_store->count())
        return;

    const ClipboardItem clip = m_store->itemAt(index);
    m_manager->copyToClipboard(clip.text); // 回选复制(内部预登记,防回环)

    // 重新复制的内容置顶:删旧位置 + 插入头部,并写回文件
    m_store->removeItem(index);
    m_store->addItem(clip);
    refreshList();
}

void MainWindow::copyCurrent()
{
    const int index = m_list->currentRow();
    if (index < 0)
        return;
    m_manager->copyToClipboard(m_store->itemAt(index).text);
}

void MainWindow::deleteCurrent()
{
    const int index = m_list->currentRow();
    if (index < 0)
        return;
    m_store->removeItem(index);
    refreshList();
}

void MainWindow::clearAll()
{
    if (m_store->count() == 0)
        return;
    if (QMessageBox::question(this, QStringLiteral("清空历史"),
                              QStringLiteral("确定要清空全部剪贴板历史吗?"))
        != QMessageBox::Yes) {
        return;
    }
    m_store->clear();
    refreshList();
}

void MainWindow::setupContextMenu()
{
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_list, &QListWidget::customContextMenuRequested, this,
            [this](const QPoint &pos) {
                QMenu menu(this);
                menu.addAction(QStringLiteral("复制到剪贴板"),
                               this, &MainWindow::copyCurrent);
                menu.addAction(QStringLiteral("删除该项"),
                               this, &MainWindow::deleteCurrent);
                menu.addSeparator();
                menu.addAction(QStringLiteral("清空历史"),
                               this, &MainWindow::clearAll);
                menu.exec(m_list->viewport()->mapToGlobal(pos));
            });
}
