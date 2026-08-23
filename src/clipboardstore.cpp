#include "clipboardstore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

#include <QDebug>

ClipboardStore::ClipboardStore(QObject *parent)
    : QObject(parent)
{
}

void ClipboardStore::addItem(const ClipboardItem &item)
{
    m_items.prepend(item); // 头部 = 最新
    while (m_items.size() > kMaxCount) // 超限淘汰最旧的(尾部)
        m_items.removeLast();
    saveToJson();
}

void ClipboardStore::removeItem(int index)
{
    if (index < 0 || index >= m_items.size())
        return;
    m_items.removeAt(index);
    saveToJson();
}

void ClipboardStore::clear()
{
    m_items.clear();
    saveToJson();
}

int ClipboardStore::count() const
{
    return m_items.size();
}

const QList<ClipboardItem> &ClipboardStore::items() const
{
    return m_items;
}

ClipboardItem ClipboardStore::itemAt(int index) const
{
    return m_items.value(index);
}

QString ClipboardStore::historyFilePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + QStringLiteral("/history.json");
}

bool ClipboardStore::saveToJson() const
{
    const QString path = historyFilePath();
    QFile file(path);
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        qWarning() << "无法创建历史目录:" << QFileInfo(path).absolutePath();
        return false;
    }
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "无法写入历史文件:" << path;
        return false;
    }

    QJsonArray array;
    for (const ClipboardItem &item : m_items) {
        QJsonObject obj;
        obj[QStringLiteral("text")] = item.text;
        // 统一存 UTC 时间,读取时再转本地时区,避免时区/夏令时歧义
        obj[QStringLiteral("timestamp")] =
            item.timestamp.toUTC().toString(Qt::ISODateWithMs);
        array.append(obj);
    }

    QJsonDocument doc(array);
    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

bool ClipboardStore::loadFromJson()
{
    const QString path = historyFilePath();
    QFile file(path);
    if (!file.exists())
        return false; // 首次运行,没有历史文件,不算错误
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开历史文件:" << path;
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
        qWarning() << "历史文件解析失败:" << parseError.errorString();
        return false;
    }

    m_items.clear();
    for (const auto &value : doc.array()) {
        const QJsonObject obj = value.toObject();
        ClipboardItem item;
        item.text = obj[QStringLiteral("text")].toString();
        item.timestamp =
            QDateTime::fromString(obj[QStringLiteral("timestamp")].toString(),
                                  Qt::ISODateWithMs)
                .toLocalTime();
        if (!item.text.isEmpty())
            m_items.append(item);
    }
    while (m_items.size() > kMaxCount)
        m_items.removeLast();

    return true;
}
