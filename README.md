# 无人机编队灯光秀模拟系统

华中科技大学 机械科学与工程学院 测控专业 C 语言课程设计

**选题：18. 无人机编队灯光秀模拟**

---

## 功能

| 模块 | 说明 |
|------|------|
| **编队初始化** | 设置无人机数量、初始位置 |
| **图案生成** | 15 种几何图案（圆/方/三角/星形/心形/螺旋…）+ 文字点阵 |
| **轨迹插值** | 关键航点间逐帧线性插值，流畅动画 |
| **灯光控制** | 8 种颜色、常亮/闪烁模式、波浪/交替/流水灯效果 |
| **安全检测** | 边界越界检测 + 无人机间距碰撞预警 |
| **数据回放** | 保存/加载轨迹文件，重新演示 |
| **历史记录** | 最近 5 次编队一键回溯 |

## 操作

| 按键 | 功能 |
|------|------|
| `S` | 开始模拟 |
| `P` | 暂停 / 继续 |
| `Q` | 停止 |
| `← →` | 切换图案（15 种几何图形 + 文字编队） |
| `↑ ↓` | 调节速度 |
| `C` | 切换灯光颜色（红/绿/蓝/白/黄/青/紫/橙） |
| `B` | 闪烁模式开关 |
| `T` | 输入文字编队（英文/数字，≤5 字） |
| `H` | 历史编队回溯（最近 5 次） |
| `ESC` | 退出 |

## 编译

```bash
g++ -std=c++11 -Wall -o drone_show.exe \
    main.cpp \
    src/drone.cpp src/light.cpp src/formation.cpp \
    src/trajectory.cpp src/safety.cpp \
    src/graphics.cpp src/ui.cpp src/file_io.cpp \
    src/controller.cpp \
    -I include
```

**编译环境：** GCC / MinGW-w64  
**依赖：** 无（纯 Windows 控制台 API，无需任何图形库）

## 项目结构

```
├── main.cpp                  # 程序入口
├── commands.txt              # 课程设计任务书
├── include/
│   ├── common.h              # 公共类型、常量、控制台颜色定义
│   ├── drone.h               # 无人机实体模块
│   ├── light.h               # 灯光控制模块
│   ├── formation.h           # 编队与图案生成模块
│   ├── trajectory.h          # 轨迹与插值模块
│   ├── safety.h              # 安全检测模块
│   ├── graphics.h            # 控制台渲染模块
│   ├── ui.h                  # 用户输入模块
│   ├── file_io.h             # 文件读写模块
│   └── controller.h          # 主控制器模块
└── src/
    ├── drone.cpp
    ├── light.cpp
    ├── formation.cpp         # 15 种图案 + 5×7 点阵文字
    ├── trajectory.cpp
    ├── safety.cpp            # 边界检测 + 间距检测 + 碰撞避让
    ├── graphics.cpp          # 帧缓冲 + WriteConsoleOutputW 双缓冲
    ├── ui.cpp
    ├── file_io.cpp
    └── controller.cpp
```

## 技术要点

- **渲染方案：** CHAR_INFO 帧缓冲 + `WriteConsoleOutputW` 一次性写入，零闪烁
- **无外部依赖：** 仅使用 Windows Console API，无需 EasyX 或其他图形库
- **工程化设计：** 模块分离、接口清晰、所有函数带注释、不使用全局变量
- **15 种图案生成器：** 统一调度接口，新增图案只需添加一个生成器函数
- **5×7 点阵字库：** 支持 A-Z、0-9 及常见符号的文字编队

## 作者

- 核心仿真引擎：drone / formation / trajectory / safety / light
- 图形渲染与交互：graphics / ui / file_io / controller
