#include <QApplication>
#include <QMainWindow>
#include <QDebug>

#include "clipboardmanager.h"

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

    // 阶段 1:监听剪贴板,暂用控制台输出验证,后续阶段 3 再接到 UI 上
    ClipboardManager manager;
    QObject::connect(&manager, &ClipboardManager::itemAdded,
                     [](const ClipboardItem &item) {
                         qDebug().noquote()
                             << item.timestamp.toString(Qt::ISODate)
                             << "|" << item.text.left(50);
                     });

    return app.exec();
}