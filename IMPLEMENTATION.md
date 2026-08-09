# 剪贴板历史管理器 — 实现清单

技术栈:Qt 6 + C++17 + Widgets (macOS)
目标:监听系统剪贴板,记录多次复制的内容,可查看/回选历史。

---

## 阶段 0:工程骨架

- [ ] `CMakeLists.txt`
  - Qt6 + Widgets,`CMAKE_CXX_STANDARD 17`,`AUTOMOC ON`
  - `find_package(Qt6 REQUIRED COMPONENTS Widgets)`
  - 目标:`clipboard_history`(GUI 程序)
- [ ] `main.cpp` — 创建 `QApplication`,加载样式可选,显示主窗口
- [ ] `mainwindow.h/.cpp` — 空主窗口,先跑通编译
- [ ] 验证:`cmake -B build && cmake --build build && ./build/clipboard_history`

## 阶段 1:核心 — 剪贴板监听 (ClipboardManager)

- [ ] `clipboardmanager.h/.cpp`,继承 `QObject`
- [ ] 条目结构:
  ```cpp
  struct ClipboardItem {
      QString    text;
      QDateTime  timestamp;
      bool       isImage() const { return !text.isEmpty(); } // 先只做文本
  };
  ```
- [ ] **双通道监听**(macOS 必读):
  - `QClipboard::dataChanged` 信号 — 即时通道
  - `QTimer` 每 250ms 轮询 `clipboard()->text()` — 兜底通道
  - **原因**:实测 macOS 上 `dataChanged` 对后台应用不实时、连续复制会合并触发
    (连按 4 次只收到最后一次),必须轮询才能不丢条目
- [ ] 统一处理函数 `onPollTimer()`(信号和轮询共用,内容比对天然幂等):
  1. 取 `clipboard()->text()`
  2. **空内容跳过** (`isEmpty()`)
  3. **去重**:与最近一条相同则跳过(只去连续重复;重新复制旧内容会再次入列置顶)
  4. 发出 `itemAdded(...)` 信号
- [ ] **防回环**:程序自己写剪贴板时用"预登记"方案 — `copyToClipboard()` 先把内容
  写入 `m_lastRecorded` 再 `setText()`,之后任何通道读到它都会被去重拦下,
  不需要 `m_selfWrite` 标志位
- [ ] 信号 `itemAdded(const ClipboardItem&)` 供 UI 订阅

## 阶段 2:数据管理 (ClipboardStore)

- [ ] `clipboardstore.h/.cpp` — 持有 `QList<ClipboardItem>`
  - `addItem()`(插入头部)、`removeItem(index)`、`clear()`
  - `maxCount = 100`,超限丢弃尾部
  - 建议用信号 `changed()` 通知刷新,而不是让 UI 直接碰列表
- [ ] (可选)持久化:`saveToJson()/loadFromJson()`
  - 存 `~/.clipboard_history.json` 或 `QStandardPaths::AppDataLocation`
  - 启动时加载,退出时保存

## 阶段 3:UI (MainWindow)

- [ ] 界面元素:
  - 中央 `QListWidget`(或 `QListView` + `QAbstractListModel` — MVP 用 ListWidget 即可)
  - 每行显示:时间 + 内容预览(超长省略 `elide` / `setToolTip` 显示全文)
- [ ] 新条目插到顶部(配合阶段 2 的 `changed()` 信号刷新)
- [ ] **回选复制**:双击或回车该行 →
  - 调用 `clipboard()->setText(...)` 之前置 `m_selfWrite = true`
  - 同时把该条目挪到列表头部(置顶)
- [ ] 右键菜单:`复制` / `删除该项` / `清空历史`
- [ ] 状态栏显示当前剪贴板文本(实时跟随,可用单行省略)

## 阶段 4:增强(按需)

- [ ] 系统托盘 `QSystemTrayIcon`,关闭窗口隐藏到托盘,托盘菜单列出最近 5 条
- [ ] 搜索过滤:顶部 `QLineEdit`,按内容过滤显示(注意过滤后序号与真实索引映射)
- [ ] 图片历史:`clipboard()->image()` 非空时存 `QImage`,列表显示缩略图
- [ ] 全局快捷键(如 Cmd+Shift+V):需 macOS 权限 / `QHotkey` 第三方库,放最后

---

## 常见坑(写之前先看)

1. **回环**:`setText` 会立刻触发 `dataChanged`,不拦截会把自己写的内容当"新复制"记录 → 每次自写前设标志位
2. **重复**:同一内容连按 Cmd+C 不产生新条目(和最近一条比较即可)
3. **空剪贴板**:清空剪贴板时不记录
4. **程序退出**:如果用户最后一笔来自你的程序,退出不会丢,因为系统剪贴板本身持有最新内容;历史文件只做备份
5. **macOS 差异**:QClipboard 在 macOS 上正常发 `dataChanged`;若在终端无 GUI 环境测试,需 `app` 正常创建且事件循环运行

## 验收标准

- [ ] 复制 A → 复制 A → 复制 B,历史显示 [B, A](连续重复去重)
- [ ] 复制 A → 复制 B → 复制 A,历史显示 [A, B, A](重新复制旧内容会再次入列置顶)
- [ ] 双击历史里的旧条目,剪贴板变为该内容,且该条目置顶
- [ ] 重启程序后历史还在(阶段 4 完成后)
- [ ] 复制超过 100 条后,最早条目被淘汰
