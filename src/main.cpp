#include <QApplication>
#include <QMainWindow>
#include <QDebug>

#include "clipboardmanager.h"
#include "clipboardstore.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("clipboard");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("clipboard");

    
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("My Qt Application");
    mainWindow.resize(800, 600);
    mainWindow.show();

    // 阶段 2:历史数据管理 + 持久化
    ClipboardStore store;
    store.loadFromJson(); // 启动时加载历史
    qDebug().noquote() << "历史文件:" << store.historyFilePath();

    // 阶段 1:监听剪贴板 → 存入历史。控制台输出保留,阶段 3 接到 UI 上
    ClipboardManager manager;
    QObject::connect(&manager, &ClipboardManager::itemAdded,
                     [&store](const ClipboardItem &item) {
                         store.addItem(item);
                         qDebug().noquote()
                             << item.timestamp.toString(Qt::ISODate)
                             << "|" << item.text.left(50);
                     });

    return app.exec();
}