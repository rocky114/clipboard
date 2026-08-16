#ifndef CLIPBOARDSTORE_H
#define CLIPBOARDSTORE_H

#include <QList>
#include <QObject>
#include <QString>

#include "clipboardmanager.h" // ClipboardItem

// 历史数据管理:持有剪贴板历史列表,负责增删清、上限淘汰与 JSON 持久化。
// 所有变更通过 changed() 信号通知 UI,UI 不直接操作内部列表。
class ClipboardStore : public QObject
{
    Q_OBJECT

public:
    static constexpr int kMaxCount = 100; // 历史上限,超出淘汰最旧的

    explicit ClipboardStore(QObject *parent = nullptr);

    void addItem(const ClipboardItem &item); // 插入头部,超限淘汰尾部
    void removeItem(int index);
    void clear();

    int count() const;
    const QList<ClipboardItem> &items() const;
    ClipboardItem itemAt(int index) const;

    // 持久化(JSON)
    bool saveToJson() const;
    bool loadFromJson();
    QString historyFilePath() const;

signals:
    void changed(); // 任何增删清操作后发出

private:
    QList<ClipboardItem> m_items;
};

#endif // CLIPBOARDSTORE_H
