#include "clipboardmanager.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QTimer>

namespace {
constexpr int kPollIntervalMs = 250; // 轮询间隔:越短越不易漏,250ms 足够日常使用
}

ClipboardManager::ClipboardManager(QObject *parent)
    : QObject(parent)
{
    // 双通道:信号即时 + 轮询兜底(macOS 后台场景信号不可靠)
    connect(QGuiApplication::clipboard(), &QClipboard::dataChanged,
            this, &ClipboardManager::onClipboardChanged);
    m_pollTimer.setInterval(kPollIntervalMs);
    connect(&m_pollTimer, &QTimer::timeout, this, &ClipboardManager::onPollTimer);
    m_pollTimer.start();
}

void ClipboardManager::copyToClipboard(const QString &text)
{
    // 预登记:内容先标记为"已见过",之后任何路径读到它都会被去重跳过,
    // 天然防止把自己写入的内容记录成新复制。
    m_lastRecorded = text;
    QGuiApplication::clipboard()->setText(text);
}

void ClipboardManager::onClipboardChanged()
{
    onPollTimer(); // 信号与轮询共用同一处理逻辑(内容比对天然幂等)
}

void ClipboardManager::onPollTimer()
{
    const QString text = QGuiApplication::clipboard()->text();

    // 1. 空内容不记录(用户清空剪贴板时)
    if (text.isEmpty())
        return;

    // 2. 与最近一条重复不记录(连按 Cmd+C 不产生新条目;自写内容也被这里拦下)
    if (text == m_lastRecorded)
        return;

    m_lastRecorded = text;
    emit itemAdded(ClipboardItem{text, QDateTime::currentDateTime()});
}
