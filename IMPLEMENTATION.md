# 剪贴板历史管理器 — 实现清单

技术栈:Qt 6 + C++17 + Widgets (macOS)
目标:监听系统剪贴板,记录多次复制的内容,可查看/回选历史。

命名约定(Qt 惯例):
- 类名 PascalCase,带模块前缀:`ClipboardManager` / `ClipboardStore` / `MainWindow`
  —— Qt 生态到处都是 `QWindow` / `QClipboard` / `QListWidgetItem`,自己的类不加前缀会糊在一起
- 文件名 = 类名全小写(`clipboardmanager.h`),和 Qt 自己的 `qclipboard.h` 一致
- 成员变量 `m_` 前缀,函数 camelCase,信号用 `itemAdded` 这类动词短语
- 不用 `namespace`:类名前缀已经承担了消歧,再加一层是冗余

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

- [x] `clipboardstore.h/.cpp` — 持有 `QList<ClipboardItem>`
  - `addItem()`(插入头部)、`removeItem(index)`、`clear()`
  - `maxCount = 100`,超限丢弃尾部
  - 构造时 `loadFromJson()` 读回已有历史(原因见常见坑 6)
- [x] 持久化:`saveToJson()/loadFromJson()`
  - 存 `QStandardPaths::AppDataLocation/history.json`
    (macOS 实际路径:`~/Library/Application Support/clipboard/clipboard/history.json`)
  - 每次增删清后自动保存(崩溃安全),启动时加载
  - 时间戳存 UTC ISO8601,加载时转本地时区

## 阶段 3:UI(窗口版,不含托盘)

托盘驻留方案已移除(代码在 git `3b1721a`,需要时可取回),改为普通窗口应用:
启动即显示历史窗口,后台持续监听并写入历史文件。

- [x] `MainWindow`:`QListWidget` 展示历史
  - 每行:时间(`MM-dd HH:mm:ss`)+ 内容预览(换行折叠、超长省略,tooltip 显示全文)
- [x] `showAndRefresh()`:启动时 `loadFromJson()` 重载 → 刷新列表 → 显示置顶
- [x] 新记录即时刷新:`main.cpp` 连接 `itemAdded` → `MainWindow::refreshList()`
  (窗口常驻可见,原来"展示前才拉取"的时机不存在了)
- [x] **回选复制**:双击/回车 → `manager->copyToClipboard()`(内部预登记防回环)+ 置顶(删旧位+插头部)
- [x] 右键菜单:`复制` / `删除该项` / `清空历史`(带确认弹窗)
- [x] 关闭窗口 = 退出程序(没有托盘,只隐藏的话就再也没法叫回来了)

已移除的部分(后续想恢复时参考):
- `QSystemTrayIcon` 驻留 + 右键菜单(显示历史 / 退出)、`setQuitOnLastWindowClosed(false)`
- `MainWindow::closeEvent` 忽略关闭只隐藏的行为

## 阶段 4:增强(按需)

- [ ] 回选复制时更新时间戳(现在会置顶,但显示的仍是旧时间)
- [ ] 搜索过滤:顶部 `QLineEdit`,按内容过滤显示(注意过滤后序号与真实索引映射)
- [ ] 图片历史:`clipboard()->image()` 非空时存 `QImage`,列表显示缩略图
- [ ] 全局快捷键(如 Cmd+Shift+V):需 macOS 权限 / `QHotkey` 第三方库,放最后
- [ ] 正式应用图标(资源文件替换占位图标)
- [ ] 托盘驻留(可选):含"托盘菜单列出最近 5 条可直接复制",见阶段 3 已移除的部分

---

## 常见坑(写之前先看)

1. **回环**:`setText` 会立刻触发 `dataChanged`,不拦截会把自己写的内容当"新复制"记录 → 每次自写前设标志位
2. **重复**:同一内容连按 Cmd+C 不产生新条目(和最近一条比较即可)
3. **空剪贴板**:清空剪贴板时不记录
4. **程序退出**:如果用户最后一笔来自你的程序,退出不会丢,因为系统剪贴板本身持有最新内容;历史文件只做备份
5. **macOS 差异**:QClipboard 在 macOS 上正常发 `dataChanged`;若在终端无 GUI 环境测试,需 `app` 正常创建且事件循环运行
6. **启动必须先加载历史**:`ClipboardStore` 构造时不读文件 → 内存里是空列表 → 第一条新记录
   落盘时把上次运行的历史整个覆盖掉(不打开窗口的纯后台运行必现,已修)
7. **启动基线**:`ClipboardManager` 构造时把当前剪贴板内容记为"已见",否则每次重启都会
   把剪贴板里残留的旧内容当成新复制再记一遍(历史出现重复行,时间戳还失真,已修)

## 验收标准

- [x] 复制 A → 复制 A → 复制 B,历史显示 [B, A](连续重复去重)
- [x] 复制 A → 复制 B → 复制 A,历史显示 [A, B, A](重新复制旧内容会再次入列置顶)
- [x] 重启程序后历史还在
- [ ] 复制超过 100 条后,最早条目被淘汰(未实测)
- [ ] 双击历史里的旧条目,剪贴板变为该内容,且该条目置顶
