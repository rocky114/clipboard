#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QSystemTrayIcon>

#include "clipboardmanager.h"
#include "clipboardstore.h"
#include "mainwindow.h"

// 生成简易占位图标(剪贴板卡片造型),正式图标后续用资源文件替换
static QIcon makeTrayIcon()
{
    QPixmap pm(32, 32);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0x3d, 0x8b, 0xfd)); // 蓝色卡片
    p.drawRoundedRect(4, 4, 24, 28, 4, 4);
    p.setBrush(Qt::white);
    p.drawRoundedRect(10, 9, 12, 17, 2, 2); // 纸面
    p.setBrush(QColor(0x2b, 0x6c, 0xd4));   // 模拟文字行
    p.drawRoundedRect(12, 13, 8, 2, 1, 1);
    p.drawRoundedRect(12, 17, 5, 2, 1, 1);
    p.end();
    return QIcon(pm);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("clipboard");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("clipboard");
    app.setQuitOnLastWindowClosed(false); // 关闭窗口只是隐藏,应用驻留托盘

    ClipboardStore store;
    ClipboardManager manager;
    // 后台持续监听并写入历史文件(窗口不在线也不影响记录)
    QObject::connect(&manager, &ClipboardManager::itemAdded,
                     &store, &ClipboardStore::addItem);

    MainWindow window(&store, &manager);

    // 默认不显示窗口,驻留系统托盘;托盘菜单点击才展示
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        QSystemTrayIcon tray(makeTrayIcon());
        QMenu trayMenu;
        trayMenu.addAction(QStringLiteral("显示剪贴板历史"),
                           &window, &MainWindow::showAndRefresh);
        trayMenu.addSeparator();
        trayMenu.addAction(QStringLiteral("退出"), &app, &QCoreApplication::quit);
        tray.setContextMenu(&trayMenu);
        // 部分平台单左键直接触发展示(有菜单的平台弹菜单)
        QObject::connect(&tray, &QSystemTrayIcon::activated, &window,
                         [&window](QSystemTrayIcon::ActivationReason reason) {
                             if (reason == QSystemTrayIcon::Trigger)
                                 window.showAndRefresh();
                         });
        tray.show();
    } else {
        // 无托盘环境(如开发调试),直接显示窗口
        window.showAndRefresh();
    }

    qDebug().noquote() << "历史文件:" << store.historyFilePath();
    return app.exec();
}
