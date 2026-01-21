# yt-dlp GUI (Qt6 Edition)

一个基于 C++ 和 Qt 6 构建的现代化 `yt-dlp` 图形界面前端。

本项目旨在为强大的命令行工具 `yt-dlp` 提供一个简洁、美观且易于使用的图形化操作界面。采用了类似 VS Code 的深色主题设计风格。

## ✨ 功能特性 (Features)

*   **直观的参数配置**：无需记忆复杂的命令行参数，通过勾选框和输入框即可完成配置。
    *   **格式控制**：支持分辨率限制 (1080p)、格式合并 (MP4)、视频/音频分离。
    *   **音频提取**：一键提取音频并转换为 MP3，支持质量设置。
    *   **后期处理**：嵌入元数据、缩略图、字幕 (支持正则语言匹配)。
    *   **网络设置**：支持 HTTP/SOCKS 代理、自定义 User-Agent、浏览器 Cookies 导入。
    *   **SponsorBlock**：集成 SponsorBlock 功能，自动跳过或标记赞助片段。
*   **实时命令预览**：在修改配置时，实时生成并展示对应的 `yt-dlp` 命令，方便学习和复制。
*   **内置终端控制台**：
    *   实时显示下载进度和日志。
    *   支持任务运行状态监控。
    *   **支持中途停止任务**。
*   **智能交互**：
    *   文件保存路径选择器。
    *   一键检查 `yt-dlp` 版本更新。
    *   自动记忆常用的设置参数。
*   **提供预编译文件**：安装即用的文件

## 🛠️ 构建环境 (Prerequisites)

在编译本项目之前，请确保您的开发环境已安装以下工具：

1.  **C++ 编译器**: 支持 C++17 标准 (Windows 推荐 MSVC 2019/2022, Linux 推荐 GCC/Clang)。
2.  **Qt 6 SDK**: 需要安装 `Qt6 Core`, `Qt6 Gui`, `Qt6 Widgets` 模块。
3.  **CMake**: 版本 >= 3.16。

## 🚀 编译指南 (Build)

### 使用 Qt Creator (推荐)
1.  打开 `CMakeLists.txt` 文件。
2.  配置项目 (Configure Project)，选择 Release 模式。
3.  点击左下角的 **构建 (Build)** 按钮。

### 使用命令行
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## 📦 运行依赖 (Runtime Requirements)

**重要**：本程序仅为图形界面外壳，实际下载功能依赖于以下核心组件。

请确保将以下可执行文件放置在程序运行目录（与 `YtDlpGui.exe` 同级），或者添加到系统的 `PATH` 环境变量中：

1.  **[yt-dlp](https://github.com/yt-dlp/yt-dlp/releases)**: 核心下载引擎。
2.  **[ffmpeg](https://ffmpeg.org/download.html)**: 用于音视频合并、格式转换 (包含 `ffmpeg.exe` 和 `ffprobe.exe`)。

如果缺少这些文件，程序启动任务时会提示 "无法启动程序"。

## 💿 打包发布 (Windows)

如果您需要将程序分发给其他用户，请按照以下步骤操作：

1.  **编译 Release 版本**：确保生成的是 Release 版的 `.exe`。
2.  **部署 Qt 依赖**：
    创建一个新文件夹，放入 `YtDlpGui.exe`，然后运行：
    ```powershell
    windeployqt YtDlpGui.exe --compiler-runtime
    ```
3.  **添加核心组件**：
    将 `yt-dlp.exe`, `ffmpeg.exe`, `ffprobe.exe` 复制到该文件夹中。
4.  **生成安装包 (可选)**：
    使用 **Inno Setup** 编写脚本，将上述文件夹打包为单一的安装程序。

## 📂 项目结构

*   `main.cpp`: 程序入口。
*   `MainWindow.cpp/h`: 主界面逻辑，处理 UI 交互、进程调用。
*   `ParamData.h`: **核心配置中心**。定义了所有支持的参数、分类、默认值及参数类型（布尔/字符串/路径/动作）。
*   `CMakeLists.txt`: CMake 构建脚本。

## 📄 License

MIT License.
