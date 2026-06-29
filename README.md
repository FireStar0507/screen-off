# ScreenOff - 一键息屏静音工具

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

## 📖 介绍
**ScreenOff** 是一个轻量级的 Windows 后台工具，通过全局热键（`Ctrl+Alt+O`）一键将屏幕变黑并静音，模拟“关机”效果，再次按下或按 `ESC` 即可恢复。适合在 Mac 一体机（BootCamp）或任何 Windows 电脑上使用。

## ✨ 特性
- 全屏黑色遮罩（置顶、无边框）
- 系统主音量控制（COM 接口，兼容 Win7/10）
- 系统托盘图标，右键菜单切换/退出
- 单实例运行（防止重复启动）
- 纯后台（无控制台窗口）

## ⚙️ 编译与使用
### 依赖
- MinGW-w64（GCC 16+）
- Windows SDK（系统自带）

### 构建
双击运行 `build.bat`，将在 `bin/` 下生成 32 位和 64 位两个 exe。

### 使用
- 运行 `bin\screen_off_64.exe`（或 32 位）
- 系统托盘出现图标，按 `Ctrl+Alt+O` 切换状态
- 右键托盘可手动切换或退出

## 📦 发布
- 直接复制对应 exe 到目标机器即可（静态链接，无额外依赖）
- 建议开机自启：将 exe 快捷方式放入 `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup`

## 🛠️ 技术细节
- 纯 Win32 API + COM（音量控制）
- 静态编译（`-static`），兼容 Win7 SP1 及以上
- 通用指令集（`-march=x86-64` / `-march=i686`）

## 📄 许可证
MIT License
