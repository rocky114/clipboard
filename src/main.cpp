#include <QApplication>
#include <QDebug>

#include "clipboardmanager.h"
#include "clipboardstore.h"
#include "mainwindow.h"

// 窗口应用:启动即显示历史窗口,后台持续监听剪贴板并写入历史文件。
// (托盘驻留方案已移除,见 IMPLEMENTATION.md 阶段 3)
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("clipboard");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("clipboard");

    ClipboardStore store; // 构造时先把已有历史读回内存,避免被新记录覆盖
    ClipboardManager manager;
    QObject::connect(&manager, &ClipboardManager::itemAdded,
                     &store, &ClipboardStore::addItem);

    MainWindow window(&store, &manager);
    // 窗口一直开着,新记录到达要即时刷新(不能再靠"展示前拉取"那套)
    QObject::connect(&manager, &ClipboardManager::itemAdded,
                     &window, &MainWindow::refreshList);
    window.showAndRefresh();

    qDebug().noquote() << "历史文件:" << store.historyFilePath();
    return app.exec();
}
