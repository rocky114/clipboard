#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QListWidget;
class QListWidgetItem;
class ClipboardStore;
class ClipboardManager;

// 历史浏览窗口:默认隐藏,点击托盘菜单时才展示。
// 剪贴板监听是后台进行的,窗口每次展示前从文件重新加载历史,不做实时刷新。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ClipboardStore *store, ClipboardManager *manager,
               QWidget *parent = nullptr);

    // 从文件重新加载历史并刷新列表,然后显示/置顶窗口
    void showAndRefresh();

protected:
    void closeEvent(QCloseEvent *event) override; // 点关闭 = 隐藏,应用留在托盘

private slots:
    void onItemActivated(QListWidgetItem *item); // 双击/回车:回选复制并置顶
    void copyCurrent();
    void deleteCurrent();
    void clearAll();

private:
    void refreshList();    // 从文件重载历史并重建列表
    void setupContextMenu();

    ClipboardStore   *m_store;
    ClipboardManager *m_manager;
    QListWidget      *m_list;
};

#endif // MAINWINDOW_H
