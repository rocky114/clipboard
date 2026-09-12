#ifndef CLIPBOARDSTORE_H
#define CLIPBOARDSTORE_H

#include <QList>
#include <QObject>
#include <QString>

#include "clipboardmanager.h" // ClipboardItem

// 历史数据管理:持有剪贴板历史列表,负责增删清、上限淘汰与 JSON 持久化。
// 构造时从文件加载已有历史;每次增删清之后自动落盘。
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

private:
    QList<ClipboardItem> m_items;
};

#endif // CLIPBOARDSTORE_H
