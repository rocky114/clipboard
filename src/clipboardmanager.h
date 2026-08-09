#ifndef CLIPBOARDMANAGER_H
#define CLIPBOARDMANAGER_H

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QTimer>

// 一条剪贴板历史记录
struct ClipboardItem {
    QString   text;      // 复制的内容(当前阶段只处理文本)
    QDateTime timestamp; // 复制时间
};

// 监听系统剪贴板,过滤噪声后发出 itemAdded 信号
//
// macOS 注意:QClipboard::dataChanged 对后台应用不实时、且会合并触发
// (连续复制只收到最后一次),所以这里采用 QTimer 轮询为主、
// dataChanged 信号为辅的双通道方案。
class ClipboardManager : public QObject
{
    Q_OBJECT

public:
    explicit ClipboardManager(QObject *parent = nullptr);

    // 程序自己回写剪贴板(阶段 3 的"回选复制"会用到)。
    // 统一走这里:先把内容预登记为"已见过",轮询/信号路径都会因去重而跳过,
    // 从而避免把自己写入的内容当成新复制记录。
    void copyToClipboard(const QString &text);

signals:
    // 捕获到一条"新的"外部复制
    void itemAdded(const ClipboardItem &item);

private slots:
    void onClipboardChanged();
    void onPollTimer();

private:
    QTimer   m_pollTimer;   // 轮询定时器,保证后台也能捕获
    QString  m_lastRecorded; // 最近一条已记录内容,用于去重
};

#endif // CLIPBOARDMANAGER_H
