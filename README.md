# EPD-nRF51 - 智能电子墨水屏控制器

[![GitHub license](https://img.shields.io/github/license/uarix/nRF51-EPD)](https://github.com/uarix/nRF51-EPD/blob/main/LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/uarix/nRF51-EPD)](https://github.com/uarix/nRF51-EPD/stargazers)

一款基于 nRF51 系列 MCU 的电子墨水屏控制器，配备[在线配置工具](https://uarix.github.io/nRF51-EPD/)，支持蓝牙图像传输、多种唤醒方式和灵活的硬件配置。

## ✨ 特性

- 支持全系列 nRF51 MCU
- 内置三款微雪 4.2 寸墨水屏驱动
- 支持 2.13 寸黑白墨水屏驱动（`EPD_2in13`）
- 支持自定义引脚映射
- 多种唤醒方式（NFC/无线充电）
- LED 状态指示配置
- 网页版配置工具，无需安装额外软件

## 📱 支持设备

### 黑白双色版本
![黑白双色版本](images/1.jpg)

**硬件规格**
- MCU：nRF51822 (RAM: 16K, ROM: 128K)
- 驱动：EPD_4in2
- 引脚配置：
  ```
  屏幕：0508090A0B0C0D
  线圈：07
  ```

### 黑白红三色版本
![黑白红三色版本](images/2.jpg)

**硬件规格**
- MCU：nRF51802 (RAM: 16K, ROM: 256K)
- 驱动：EPD_4in2b_V2
- 引脚配置：
  ```
  屏幕：0A0B0C0D0E0F10
  线圈：09
  LED：03/04/05（三选一）
  ```

### 2.13 寸黑白版本

**硬件规格**
- 分辨率：104 x 212
- 驱动：`EPD_2in13`
- 驱动 ID：`04`
- 引脚配置：
  ```
  屏幕：1E000102030405
  ```

引脚配置顺序为 `MOSI/SCLK/CS/DC/RST/BUSY/BS`，上面的配置对应 `MOSI=30, SCLK=0, CS=1, DC=2, RST=3, BUSY=4, BS=5`。

> **注意：** 默认配置适用于黑白双色版本，其他型号需要手动调整驱动和引脚映射。

## 🌐 在线配置工具

访问地址：[https://uarix.github.io/nRF51-EPD/](https://uarix.github.io/nRF51-EPD/)

![配置工具界面](images/web.png)

支持在线配置或本地使用（本地部署 `html/index.html`）

### 更新 2.13 寸图片

1. 烧录固件后给设备重新上电，打开网页工具并点击 `连接`。
2. 驱动选择 `EPD_2in13`。
3. 引脚填写 `1E000102030405`，点击 `确认`。这一步会保存引脚、设置驱动 ID `04` 并初始化屏幕。
4. 选择要发送的图片。建议提前裁剪为 `104 x 212`，否则网页会自动拉伸到该尺寸。
5. 抖动算法先选择 `二值化`，阈值可先保持 `125`。如果黑白效果不理想，再调整阈值。
6. 点击 `发送图片`，等待日志显示发送完成，屏幕会自动刷新。

2.13 寸屏幕发送图片时，网页会自动按顺序发送旧 RAM、新 RAM 和刷新指令：

```text
0310 + 旧 RAM 填充
0313 + 图片数据
05   + 刷新屏幕
```

不要在更新图片前手动发送 `06`，`06` 是屏幕休眠命令。若日志出现 `服务不可用` 或 `已断开连接`，需要先重新上电或点击 `重连`，连接恢复后再发送图片。

## 🛠️ 开发指南

### GCC + Make（推荐）

仓库已包含所需 Nordic SDK 源码，无需 Keil 或额外下载 SDK。
需要 GNU Make 和完整的 Arm GNU Embedded 工具链（包含 newlib-nano）。macOS 安装：

```sh
brew install --cask gcc-arm-embedded
```

在项目根目录运行：

```sh
make -j4           # 发布版：build/release/EPD.{elf,hex,bin,map}
make DEBUG=1 -j4   # 含 RTT 日志和错误现场的调试版：build/debug/
make size         # 查看发布版大小
make clean        # 清理两种构建产物
```

已使用 Arm GNU Toolchain 15.3.Rel1 编译验证。工具链不在 PATH 时，可指定
`make CROSS_COMPILE=/path/to/bin/arm-none-eabi-`。修改源码或头文件后，Make 会增量编译。
调试版仍使用 `-Os` 以适应小容量芯片，并保留 `-g3` 调试信息。

默认对应 Keil 的 nRF51822 xxAB 配置（128 KiB Flash、16 KiB RAM）及 S110 8.0.0：

- 应用 Flash 从 `0x18000` 开始，至 `0x1F800` 之前；最后两页共 2 KiB 留给 pstorage 配置和交换页。
- 应用 RAM 为 `0x20002000`–`0x20004000`，构建预留 2 KiB 栈和 512 B 堆。
- 链接脚本检查 Flash 边界及静态数据、堆、栈的空间；运行时栈用量仍需在硬件上验证。

`EPD.hex` **仅包含应用**。首次刷机仍需先烧录仓库中的
`components/softdevice/s110/hex/s110_nrf51_8.0.0_softdevice.hex`，再烧录应用 HEX。
HEX 自带地址；若使用 BIN，应用烧录地址为 `0x18000`。默认 `make` 只构建；刷写命令见下节。

旧驱动中有未使用代码的警告；newlib 的 `nosys` 文件读写桩也可能产生链接警告，
因为裸机没有文件系统。调试日志使用 RTT。编译通过不等于已验证实机蓝牙及屏幕行为。

### CMSIS-DAP + OpenOCD 刷写（开源）

支持标准 CMSIS-DAP USB 调试器，通过 SWD 连接，默认无需接 RESET。
Sipeed SLogic Combo 8 需按键切换到**绿色指示灯（DAPLink）**，USB 设备名应为
`RV CMSIS-DAP`。按面板左侧 DAPLink 线序接线：`TMS` 接目标 `SWDIO`，
`TCK` 接目标 `SWCLK`，`GND` 共地；SWD 不使用 `TDI/TDO`。
参见 [Sipeed DAPLink 文档](https://en.wiki.sipeed.com/hardware/en/logic_analyzer/combo8/use_daplink_function.html)。

macOS 安装 OpenOCD（配置已在 0.12.0 上验证）：

```sh
brew install openocd
```

接线前断开电源，根据板上丝印或原理图找到焊盘：

| 调试器 | 墨水屏板 |
| --- | --- |
| SWDIO | SWDIO |
| SWCLK / SWCK | SWCLK |
| GND | GND |
| VTref（如果有） | 目标板 VDD，作为电平参考 |

目标板需要供电，SWD 电平必须与板上 VDD 相容。VTref 通常是输入，不能当作供电输出。
如用调试器的 3.3V 输出供电，先确认板子支持 3.3V，并断开电池或其他电源；不要接 5V。

```sh
make probe          # 检查调试器和目标连接，不擦写 Flash
make flash-all      # 首次安装：全片擦除，然后依次烧录并校验 S110 和应用
make flash          # 日后更新：只擦写应用占用的扇区，保留 S110 和配置存储
make DEBUG=1 flash  # 烧录调试版
```

**`flash-all` 会清除原固件、UICR 和保存的屏幕/引脚配置。** 如需保留原固件或设置，先备份。
默认构建仍按 128 KiB Flash / 16 KiB RAM 配置；不要单独烧录应用来替代首次安装。
成功烧录后会校验并复位运行，必要时重新上电。

连接不稳定时可降低 SWD 频率，例如 `make probe SWD_SPEED=100` 或
`make flash SWD_SPEED=100`（单位 kHz）。若提示 `unable to find a matching CMSIS-DAP device`，
先检查 USB 数据线、调试器模式和系统是否识别调试器；这一步还未连接到目标 MCU。
若调试器已识别但 MCU 连接失败，再检查供电、共地、SWD 接线及芯片读保护状态。

配置使用 OpenOCD 自带的 `interface/cmsis-dap.cfg` 和 `target/nrf51.cfg`；
刷写采用 HEX 自带地址，无需手填地址。

### Keil（可选）

保留原工程，使用带 ARM Compiler 5 的 Keil（如 ≤ 5.36）。这个限制来自现有 Keil
工程配置，并非 Nordic SDK 只能使用 ARM Compiler 5。

### 项目配置
提供三个编译目标：
- `EPD`：生产环境固件
- `EPD-Debug`：开发环境（含日志和 RTT）
- `flash_softdevice`：蓝牙协议栈

### 刷机步骤
1. 擦除全部固件
2. 切换到 `flash_softdevice` 刷入蓝牙协议栈（仅需一次）
3. 切换到 `EPD` 编译并下载

> 在 [Releases](https://github.com/uarix/nRF51-EPD/releases) 页面提供预编译固件

## 🙏 致谢

本项目基于以下开源项目开发：
- 屏幕驱动：[微雪 E-Paper Shield](https://www.waveshare.net/wiki/E-Paper_Shield)
- 蓝牙控制：[atc1441/ATC_TLSR_Paper](https://github.com/atc1441/ATC_TLSR_Paper)
- 原始项目：[tsl0922/EPD-nRF51](https://github.com/tsl0922/EPD-nRF51)

## 📄 许可证

本项目采用 [GPL-3.0 许可证](LICENSE)
