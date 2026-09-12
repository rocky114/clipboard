#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QListWidget;
class QListWidgetItem;
class ClipboardStore;
class ClipboardManager;

// 历史浏览窗口:启动即显示。剪贴板监听在后台持续进行,
// 有新记录时由 main.cpp 连接 itemAdded → refreshList() 即时刷新列表。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ClipboardStore *store, ClipboardManager *manager,
               QWidget *parent = nullptr);

    // 从文件重新加载历史并刷新列表,然后显示/置顶窗口
    void showAndRefresh();

public slots:
    void refreshList(); // 从文件重载历史并重建列表

private slots:
    void onItemActivated(QListWidgetItem *item); // 双击/回车:回选复制并置顶
    void copyCurrent();
    void deleteCurrent();
    void clearAll();

private:
    void setupContextMenu();

    ClipboardStore   *m_store;
    ClipboardManager *m_manager;
    QListWidget      *m_list;
};

#endif // MAINWINDOW_H
