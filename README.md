# 无人机编队灯光秀模拟系统

华中科技大学 机械科学与工程学院 测控专业 C 语言课程设计

**选题 18：无人机编队灯光秀模拟**

---

## 功能

| 模块 | 说明 |
|------|------|
| **编队初始化** | 500架无人机池，按需激活，初始随机散布保证最小间距 |
| **图案生成** | 15种几何图案 + GDI文字点阵(中英文) + BMP图片像素阵列 |
| **轨迹插值** | 航点→逐帧线性插值，到达精确吸附，随机出发延迟减少交叉 |
| **灯光控制** | 8色常亮/闪烁 + 4种动态特效(波浪/流水灯/交替/彩虹流水) |
| **安全检测** | 越界检测 + 间距碰撞预警 + 高度感知警告日志 |
| **历史记录** | 最近5次编队一键回溯 |
| **数据回放** | 保存/加载轨迹文件框架(桩实现) |

## 操作键

| 按键 | 功能 |
|------|------|
| `S` | 开始模拟 |
| `P` | 暂停 / 继续 |
| `Q` | 停止 |
| `← →` | 切换图案（15种几何 + 文字 + 图片） |
| `↑ ↓` | 调节速度 |
| `C` | 切换灯光颜色 |
| `B` | 闪烁开关 |
| `E` | 切换灯光特效（无/波浪/流水灯/交替/彩虹流水） |
| `T` | 输入文字编队（中英文，≤5字） |
| `I` | 加载BMP图片编队 |
| `H` | 历史编队回溯 |
| `ESC` | 退出 |

## 编译

```bash
g++ -std=c++11 -Wall -o drone_show.exe \
    main.cpp \
    src/drone.cpp src/light.cpp src/formation.cpp \
    src/trajectory.cpp src/safety.cpp \
    src/graphics.cpp src/ui.cpp src/file_io.cpp \
    src/controller.cpp \
    -I include -lm -lgdi32
```

**环境：** GCC / MinGW-w64，仅用 Windows Console API + GDI，无需第三方图形库。

## 项目结构

```
├── main.cpp              程序入口
├── commands.txt           课程设计任务书
├── DESIGN_REPORT.md       软件系统需求分析与设计报告
├── flowchart.md           系统流程图（Mermaid格式，8张图）
├── README.md              本文件
├── include/
│   ├── common.h           公共类型、常量、控制台颜色
│   ├── drone.h            无人机实体模块
│   ├── light.h            灯光控制模块
│   ├── formation.h        编队与图案生成模块
│   ├── trajectory.h       轨迹与插值模块
│   ├── safety.h           安全检测模块
│   ├── graphics.h         控制台渲染模块
│   ├── ui.h               用户输入模块
│   ├── file_io.h          文件读写模块
│   └── controller.h       主控制器模块
└── src/                   源文件实现
```

## 技术要点

- **渲染：** CHAR_INFO 帧缓冲 + WriteConsoleOutputW，零闪烁（需最大化窗口避免Terminal视口偏移）
- **文字：** GDI 逐字渲染到 12×12 像素格 → 逐像素采样（与PCtoLCD2002同原理）
- **图片：** GDI LoadImage → StretchBlt → 逐像素采样
- **仿真：** 离散时间步长驱动，轨迹线性插值，随机出发延迟减小同步交叉
- **安全：** 每帧O(n)越界检测 + O(n²)间距检测，高度感知警告
- **UI：** ASCII面板避免CJK全角/半角边框错位，双缓冲控制台渲染

## 已知限制

- Win11 Terminal 小窗口下可能出现画面重复，建议最大化窗口使用
- 面板使用纯英文，因为控制台不支持全角框线字符
- 鼠标交互在 Win11 Terminal 下与键盘输入冲突，暂未启用

## 标签

`v1.0` — 全功能可用稳定版本
