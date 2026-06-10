# 无人机编队灯光秀模拟系统 — 软件系统需求分析与设计报告

> **选题编号**: 18  
> **课程**: C语言课程设计  
> **院校**: 华中科技大学 · 机械科学与工程学院 · 测控专业 2025级  
> **指导教师**: 周凯波  
> **总代码量**: ~4400行 (C/C++)  
> **编译环境**: GCC / MinGW-w64  
> **依赖**: 无第三方库，仅 Windows Console API  

---

## 目录

1. [软件系统需求分析](#1-软件系统需求分析)
2. [系统架构设计](#2-系统架构设计)
3. [模块详细设计](#3-模块详细设计)
4. [数据结构设计](#4-数据结构设计)
5. [核心算法与流程图](#5-核心算法与流程图)
6. [函数接口文档](#6-函数接口文档)
7. [用户操作指南](#7-用户操作指南)

---

## 1. 软件系统需求分析

### 1.1 功能需求

根据课程设计任务书（选题18），系统需实现以下功能：

| 编号 | 功能模块 | 需求描述 | 实现状态 |
|------|---------|---------|---------|
| F1 | 编队初始化 | 设置无人机数量、初始位置和飞行高度 | ✅ 已完成 |
| F2 | 轨迹设计 | 输入关键坐标点，无人机按直线自动移动，完成队形变换 | ✅ 已完成 |
| F3 | 灯光控制 | 控制灯光开关、颜色切换（8种）和闪烁效果 | ✅ 已完成 |
| F4 | 实时模拟 | 在终端动态显示无人机位置和灯光状态 | ✅ 已完成 |
| F5 | 安全检测 | 判断越界和间距过近，给出提示 | ✅ 已完成 |
| F6 | 数据回放 | 保存/读取轨迹数据，重新演示灯光秀 | ✅ 已完成 |

### 1.2 非功能需求

| 编号 | 类别 | 需求描述 |
|------|------|---------|
| NF1 | 图形界面 | 图形化界面操作，不依赖贴图（EasyX等），纯控制台字符画 |
| NF2 | 工程化 | 模块化设计，头文件/源文件分离，函数注释完整 |
| NF3 | 无全局变量 | 所有状态封装在结构体中，通过指针传递 |
| NF4 | 代码量 | 每人≥1000行有效代码 |
| NF5 | 编码规范 | 统一命名、注释风格，可读性和可维护性 |

### 1.3 扩展功能（超出任务书要求）

| 编号 | 功能 | 描述 |
|------|------|---------|
| E1 | 15种几何图案 | 圆形/方形/三角/菱形/五角星/五边形/六边形/心形/螺旋/直线/箭头/十字/弧形/网格/随机 |
| E2 | 文字编队 | 英文/数字5×7点阵，支持A-Z、0-9 |
| E3 | 灯光特效 | 波浪灯、交替闪烁、流水灯循环 |
| E4 | 编队历史 | 最近5次编队一键回溯 |
| E5 | 动态速度 | 实时调节模拟速度 |
| E6 | 碰撞避让 | 自动推开过近的无人机 |

---

## 2. 系统架构设计

### 2.1 项目文件结构

```
Drone/
├── main.cpp               # 程序入口（61行）
├── commands.txt            # 课程设计任务书
├── include/
│   ├── common.h            # 公共类型、常量、控制台颜色定义（270行）
│   ├── drone.h             # 无人机实体模块接口
│   ├── light.h             # 灯光控制模块接口
│   ├── formation.h         # 编队与图案生成模块接口
│   ├── trajectory.h        # 轨迹与插值模块接口
│   ├── safety.h            # 安全检测模块接口
│   ├── graphics.h          # 控制台渲染模块接口
│   ├── ui.h                # 用户输入模块接口
│   ├── file_io.h           # 文件读写模块接口
│   └── controller.h        # 主控制器模块接口
└── src/
    ├── drone.cpp            # 无人机生命周期与状态操作（236行）
    ├── light.cpp            # 编队级灯光效果（138行）
    ├── formation.cpp        # 15种图案生成 + 5×7点阵文字（914行）
    ├── trajectory.cpp       # 航点管理与逐帧线性插值（214行）
    ├── safety.cpp           # 边界/间距检测 + 碰撞避让（180行）
    ├── graphics.cpp         # CHAR_INFO帧缓冲 + WriteConsoleOutputW（879行）
    ├── ui.cpp               # 键盘输入处理（150行）
    ├── file_io.cpp          # 轨迹数据持久化（105行）
    └── controller.cpp       # 主循环与模块调度（648行）
```

### 2.2 模块依赖关系

```
                        ┌─────────────┐
                        │   main.cpp  │
                        └──────┬──────┘
                               │
                    ┌──────────▼──────────┐
                    │   controller.h/cpp  │  ← 顶层调度器
                    └──────────┬──────────┘
                               │
        ┌──────────┬───────────┼───────────┬──────────┬──────────┐
        │          │           │           │          │          │
   ┌────▼───┐ ┌───▼────┐ ┌───▼────┐ ┌───▼────┐ ┌───▼───┐ ┌───▼───┐
   │ drone  │ │ light  │ │traject │ │ safety │ │graphics│ │  ui   │
   │.h/.cpp │ │.h/.cpp │ │.h/.cpp │ │.h/.cpp │ │.h/.cpp│ │.h/.cpp│
   └────┬───┘ └───┬────┘ └───┬────┘ └───┬────┘ └───┬───┘ └───┬───┘
        │         │          │          │          │         │
        └─────────┴──────────┴──────────┴──────────┴─────────┘
                               │
                    ┌──────────▼──────────┐
                    │   common.h          │  ← 所有模块共享
                    └─────────────────────┘
```

### 2.3 系统运行流程

```
启动程序
    │
    ▼
controller_create()         ← 分配内存、创建300架无人机、初始化安全区域
    │
    ▼
controller_run()            ← 主入口
    │
    ├─ graphics_init()      ← 设置控制台120×45、UTF-8、隐藏光标
    ├─ graphics_show_welcome() ← 显示欢迎界面（按任意键继续）
    │
    ▼
┌────────── 主循环 (20fps) ──────────┐
│                                    │
│  ① ui_poll_input()  检查按键       │
│     └→ ctrl_handle_command() 处理  │
│                                    │
│  ② ctrl_update_frame() 轨迹更新    │
│     └→ traj_update_fleet() 移动    │
│                                    │
│  ③ safety_check_all()  安全检测    │
│     ├→ 边界越界检测                │
│     └→ 间距碰撞检测                │
│                                    │
│  ④ ctrl_render_frame()  渲染       │
│     ├→ graphics_clear()  清帧缓冲  │
│     ├→ graphics_draw_title_bar()   │
│     ├→ graphics_draw_stage()       │
│     ├→ graphics_draw_all_drones()  │
│     ├→ graphics_draw_warnings()    │
│     ├→ graphics_draw_panel()       │
│     ├→ graphics_draw_bottom_bar()  │
│     └→ graphics_flush()  输出      │
│                                    │
│  ⑤ Sleep(50ms) 帧率控制           │
│                                    │
└────────────────────────────────────┘
    │
    ▼
graphics_close()            ← 恢复光标、退出
controller_destroy()        ← 释放所有资源
```

---

## 3. 模块详细设计

### 3.1 公共模块 (common.h)

**职责**: 为所有模块提供统一的数据类型、枚举和常量定义。

**关键设计决策**:
- 所有头文件 `#include "common.h"`，确保类型一致性
- 使用 `#define` 常量而非魔术数字
- 结构体封装一切状态（无全局变量）

**核心常量**:

| 常量 | 值 | 说明 |
|------|---|------|
| `CONSOLE_WIDTH` | 120 | 控制台总列数 |
| `CONSOLE_HEIGHT` | 45 | 控制台总行数 |
| `STAGE_COLS` | 80 | 表演区宽度 |
| `STAGE_ROWS` | 40 | 表演区高度 |
| `STAGE_LEFT` | 2 | 表演区左边距 |
| `STAGE_TOP` | 2 | 表演区上边距 |
| `PANEL_LEFT` | 84 | 面板起始列 |
| `PANEL_WIDTH` | 34 | 面板宽度 |
| `MAX_DRONE_COUNT` | 300 | 最大无人机数量 |
| `MAX_WAYPOINTS` | 200 | 最大航点数 |
| `FRAME_INTERVAL_MS` | 50 | 帧间隔（20fps） |
| `SAFETY_MIN_DISTANCE` | 3 | 安全间距（字符格） |

**核心枚举**:
- `LightColor` (9种): OFF/RED/GREEN/BLUE/WHITE/YELLOW/CYAN/PURPLE/ORANGE
- `LightMode` (3种): STEADY/BLINK/OFF
- `PatternType` (17种): NONE/CIRCLE/SQUARE/TRIANGLE/DIAMOND/STAR/PENTAGON/HEXAGON/HEART/SPIRAL/LINE/ARROW/CROSS/ARC/GRID/RANDOM/TEXT
- `SimState` (4种): IDLE/RUNNING/PAUSED/REPLAY

---

### 3.2 无人机实体模块 (drone.h/cpp)

**职责**: 单架无人机的生命周期管理和状态操作。

**数据流**:
```
drone_create(id, x, y, height)
    │
    ├─ malloc(Drone) → 堆分配
    ├─ 初始化 id, position, height, is_active=1
    └─ 初始化 light: COLOR_WHITE, LIGHT_STEADY

drone_get_current_color(drone, delta_ms)
    │
    ├─ LIGHT_OFF → CON_BLACK
    ├─ LIGHT_STEADY → color_table[light.color]
    └─ LIGHT_BLINK → 累加 blink_timer, 切换 is_visible
```

**核心函数**:

| 函数 | 参数 | 返回值 | 功能 |
|------|------|--------|------|
| `drone_create` | id, x, y, height | `Drone*` | 在堆上创建无人机 |
| `drone_destroy` | `Drone*` | void | 释放单架无人机 |
| `drone_create_fleet` | count, start_id | `Drone**` | 批量创建编队 |
| `drone_set_position` | drone, x, y, h | void | 瞬移（初始化） |
| `drone_move` | drone, dx, dy, dh | void | 增量移动（逐帧） |
| `drone_get_current_color` | drone, delta_ms | `ConsoleColor` | 计算当前颜色（含闪烁） |
| `drone_set_light_color` | drone, color | void | 设置灯光颜色 |

---

### 3.3 灯光控制模块 (light.h/cpp)

**职责**: 编队级的灯光效果控制。

**灯光效果**:

| 效果 | 函数 | 算法 |
|------|------|------|
| 全队统一 | `light_fleet_set_color/mode` | 遍历编队，逐架设置 |
| 波浪灯 | `light_wave_effect` | 按索引延迟：`lit_count = elapsed_ms / delay_ms`，前N架亮灯 |
| 交替闪烁 | `light_alternate` | 奇数偶数组相位翻转：phase 0→1 每 interval_ms |
| 流水灯 | `light_flow` | 滑动窗口：`[offset, offset+window_size)` 区间的无人机亮灯 |
| 颜色渐变 | `light_color_lerp` | 阈值切换（16色无法RGB渐变）：t<0.5→color_from, t≥0.5→color_to |

**颜色映射表** (drone.cpp 内部):
```
COLOR_RED    → CON_RED     (FOREGROUND_RED | FOREGROUND_INTENSITY)
COLOR_GREEN  → CON_GREEN   (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
COLOR_BLUE   → CON_BLUE    (FOREGROUND_BLUE | FOREGROUND_INTENSITY)
COLOR_WHITE  → CON_WHITE   (R|G|B|INTENSITY)
COLOR_YELLOW → CON_YELLOW  (R|G|INTENSITY)
...
```

---

### 3.4 编队与图案生成模块 (formation.h/cpp)

**职责**: 15种几何图案 + 文字点阵的生成器架构。

**设计模式**: **策略模式** — 每种图案由独立的生成器函数实现，通过 `pattern_generate()` 统一调度。

```
            pattern_generate(type, center, scale, rotation, count, out, text)
                                  │
                ┌─────────────────┼─────────────────────┐
                │                 │                     │
        ┌───────▼──────┐  ┌──────▼──────┐      ┌──────▼──────┐
        │ gen_circle   │  │ gen_square  │ ...  │ gen_text    │
        │ 等角圆周分布  │  │ 等距边框分布 │      │ 5×7点阵字库 │
        └──────────────┘  └─────────────┘      └─────────────┘
```

**15种图案生成算法**:

| 图案 | 算法 | 关键参数 |
|------|------|---------|
| 圆形 | `angle = 360°/count * i`，等距分布圆周 | radius (scale) |
| 正方形 | `步长 = 周长/count`，沿四条边分布 | side (scale) |
| 正三角形 | 三条边等分，顶点位于重心上方 | size (边长) |
| 菱形 | 四条边等分，对角轴水平/垂直 | size (对角长) |
| 五角星 | 10条边等分：外圈R + 内圈r=0.382R交替 | R (外圈半径) |
| 正五/六边形 | N条边等分，等角顶点 | size (外接圆半径) |
| 心形 | 参数方程：x=16sin³t, y=13cost-5cos2t-2cos3t-cos4t | scale/16 |
| 螺旋 | 阿基米德螺旋：r = b*θ, θ∈[0, 3*2π] | size/最大角度 |
| 直线 | 等间距水平排列 | length |
| 箭头 | 箭身(70%) + 三角形箭头(30%) | size |
| 十字 | 竖直臂(50%) + 水平臂(50%) | size (臂长) |
| 弧形 | 等角圆弧，起始-150°扫300° | radius |
| 网格 | ceil(sqrt(N))列，等间距 | size/(cols+1) |
| 随机 | 均匀随机坐标 | size (散布范围) |

**文字编队 (5×7点阵)**:
- 字库：ASCII 32-90（空格到Z），每个字符7字节（每行1字节5列）
- 密度自适应：可用无人机多 → density=3（9架/像素簇），少 → density=1（骨架）
- 布局：6×char_size宽度/字符，水平居中

**扩展新图案**: 只需添加一个生成器函数 + 在 `pattern_generate()` 的 switch 中注册。

---

### 3.5 轨迹与插值模块 (trajectory.h/cpp)

**职责**: 关键航点管理与逐帧线性插值。

**核心算法**:
```
每帧执行 (delta_ms):
    1. 计算到目标航点的距离: dist = sqrt(dx² + dy²)
    2. 移动步长: step = speed * delta_ms / 1000.0
    3. 如果 dist < 0.5: 到达航点 → 同步灯光 → 切到下一航点
    4. 否则: 沿方向向量移动一步（不超过目标）
       drone.x += (dx/dist) * step
       drone.y += (dy/dist) * step
```

**批量操作**:
- `traj_from_formation()`: 为编队中每架无人机创建"当前位置→编队目标"的航点
- `traj_update_fleet()`: 逐帧批量更新所有无人机的轨迹

**流程图**:
```
开始一帧
    │
    ▼
current_index < waypoint_count? ─── No ──→ 返回0（轨迹结束）
    │ Yes
    ▼
计算到目标距离 dist
    │
    ▼
dist < 0.5? ── Yes ──→ 同步灯光状态 → current_index++ → 还有航点? ── No → 返回0
    │ No                                                    │ Yes
    ▼                                                       └──→ 返回1
计算方向向量 (nx, ny) = (dx/dist, dy/dist)
    │
    ▼
移动 step = min(speed*dt/1000, dist)
drone_move(drone, nx*step, ny*step, 0)
    │
    ▼
返回1（仍在运动）
```

---

### 3.6 安全检测模块 (safety.h/cpp)

**职责**: 每帧自动执行边界检测 + 间距检测。

**边界检测** (O(n)):
```
遍历所有活跃无人机:
    if (x < zone.x_min || x > zone.x_max || y < zone.y_min || y > zone.y_max):
        记录越界 → boundary_violations++
```

**间距检测** (O(n²)):
```
两两遍历 (i from 0..n, j from i+1..n):
    dist = DISTANCE(fleet[i], fleet[j])
    if (dist < zone.min_distance):
        记录违规对 → distance_violations++
```

**碰撞避让** (O(n²)):
```
两两遍历:
    if (dist < min_distance/2 && dist > 0.001):
        force = (avoid_dist - dist) / avoid_dist * strength
        排斥力推开两架无人机:
        fleet[i] += (nx*force, ny*force)
        fleet[j] -= (nx*force, ny*force)
```

---

### 3.7 图形渲染模块 (graphics.h/cpp)

**职责**: 控制台字符画渲染，使用帧缓冲消除闪烁。

**渲染方案**:
```
  ┌─────────────────────────────────────┐
  │  CHAR_INFO g_fb[45][120]            │  ← 内存中的虚拟屏幕
  │  - fb_clear()    : 全部填空格       │
  │  - fb_put_wchar(): 写单个WCHAR      │
  │  - fb_puts()     : 写UTF-8字符串    │
  │  - fb_printf()   : 格式化写字符串   │
  │  - fb_flush()    : 一次性写入控制台 │
  └─────────────────────────────────────┘
         │  WriteConsoleOutputW(hOut, g_fb, ...)
         ▼
  ┌─────────────────────────────────────┐
  │  控制台缓冲区 (120×45)              │
  │  用户可见                           │
  └─────────────────────────────────────┘
```

**坐标映射**:
- 舞台逻辑坐标: `(stage_x, stage_y)` ∈ `[0, 79] × [0, 39]`
- 控制台屏幕坐标: `(STAGE_LEFT + stage_x, STAGE_TOP + stage_y)`
- 无人机显示: `(int)(drone.position.x + 0.5)` 四舍五入

**屏幕布局**:
```
Row 0:  ─────── 标题栏 ────────────────────────────────
Row 1:  ─────── 分隔线 ────────────────────────────────
Row 2:  ╔═══ 表演区域 80×40 ═══╗   ╔══ 面板 34列 ══╗
Row 3:  ║ ● 无人机编队          ║   ║ 无人机灯光秀模拟 ║
 ...    ║                      ║   ║ 状态 ○ 待命中.. ║
Row 41: ╚══════════════════════╝   ╚════════════════╝
Row 42: (安全告警区域)
Row 43:
Row 44: ─────── 底部状态栏 ────────────────────────────
```

**双缓冲机制**: `fb_clear()` → 全部绘制 → `fb_flush()`，帧缓冲内部完成所有修改，最后一次性输出，零闪烁。

---

### 3.8 用户输入模块 (ui.h/cpp)

**职责**: 键盘输入检测与操作码映射。

**按键映射表**:

| 按键 | 操作码 | 功能 |
|------|--------|------|
| `S` | `UI_CMD_START` | 开始模拟 |
| `P` / `Space` | `UI_CMD_PAUSE` | 暂停/继续 |
| `Q` | `UI_CMD_STOP` | 停止模拟 |
| `←` / `→` | `UI_CMD_PREV/NEXT_PATTERN` | 切换图案 |
| `↑` / `↓` | `UI_CMD_SPEED_UP/DOWN` | 调节速度 |
| `C` | `UI_CMD_CHANGE_COLOR` | 切换颜色 |
| `B` | `UI_CMD_TOGGLE_BLINK` | 闪烁开关 |
| `T` | `UI_CMD_TEXT_INPUT` | 输入文字编队 |
| `H` | `UI_CMD_HISTORY` | 历史回溯 |
| `ESC` | `UI_CMD_EXIT` | 退出 |

**实现**: 非阻塞 `_kbhit()` + `_getch()`，扩展键两字节（0xE0 + 键码）。

---

### 3.9 文件读写模块 (file_io.h/cpp)

**职责**: 轨迹数据持久化。

**文件格式** (二进制):
```
Header (16B):
  magic    : 4B ("DRON" = 0x44524F4E)
  count    : 4B (无人机数量)
  frames   : 4B (总帧数)
  interval : 4B (帧间隔ms)

Frame 0, 1, ... N:
  drone[i].x     : 4B (float)
  drone[i].y     : 4B (float)
  drone[i].color : 1B (LightColor enum)
  ... (共 count 架)
```

---

### 3.10 主控制器模块 (controller.h/cpp)

**职责**: 系统顶层调度，所有子系统的粘合剂。

**状态机**:
```
                    START
        IDLE ────────────────→ RUNNING
          ↑                      │
          │ STOP            PAUSE│
          │                 ═════╝
          │                      │
          └──────────────────────┘
              (PAUSED时按P恢复)
```

**Controller 结构体** (核心状态容器):
```c
typedef struct {
    Drone*      fleet[300];        // 无人机编队
    int         drone_count;       // 实际数量
    Formation*  current_formation; // 当前编队
    Trajectory* trajectories[300];// 轨迹数组
    SafetyZone* safety_zone;      // 安全区域
    SafetyResult safety_result;   // 检测结果
    SimState    sim_state;        // 模拟状态
    float       sim_speed;        // 速度倍率
    LightColor  selected_color;   // 当前颜色
    LightMode   selected_light_mode;
    FormationHistory history[5];  // 编队历史
    int         history_count;
} Controller;
```

---

## 4. 数据结构设计

### 4.1 核心结构体关系图

```
Controller
    │
    ├─ fleet: Drone*[300] ──────→ Drone {id, position(x,y), height, LightState, is_active}
    │                                    │
    │                                    └─ LightState {LightColor color, LightMode mode,
    │                                                    blink_interval, blink_timer, is_visible}
    │
    ├─ current_formation: Formation* → {formation_id, name, PatternType pattern,
    │     Point2f center, float scale, float rotation,
    │     Point2f targets[300], int drone_count, char display_text[32]}
    │
    ├─ trajectories: Trajectory*[300] → {WayPoint waypoints[200], waypoint_count,
    │     int current_index, float total_progress}
    │     │
    │     └─ WayPoint {Point2f position, LightColor, LightMode, int hold_ms}
    │
    ├─ safety_zone: SafetyZone* → {x_min, y_min, x_max, y_max, min_distance}
    │
    └─ safety_result: SafetyResult → {boundary_ids[300], pair_a[300], pair_b[300],
          boundary_violations, distance_violations}
```

### 4.2 关键数据结构

**Point2f / Point2i** — 二维坐标:
```c
typedef struct { float x; float y; } Point2f;  // 精确浮点（计算用）
typedef struct { int   x; int   y; } Point2i;   // 整数显示（渲染用）
```

**无人机 (Drone)** — 所有状态封装:
```c
typedef struct {
    int         id;             // 唯一标识
    Point2f     position;       // 当前坐标
    float       height;         // 飞行高度
    LightState  light;          // 灯光状态（嵌套结构体）
    int         is_active;      // 是否活跃
} Drone;
```

**编队 (Formation)**:
```c
typedef struct {
    int         formation_id;
    char        name[32];                   // 如"圆形编队"
    PatternType pattern;                    // 图案类型
    Point2f     center;                     // 中心坐标
    float       scale;                      // 缩放因子
    float       rotation_deg;               // 旋转角度
    Point2f     targets[MAX_DRONE_COUNT];   // 无人机目标位置
    int         drone_count;                // 实际无人机数
    char        display_text[32];           // 文字编队文本
} Formation;
```

---

## 5. 核心算法与流程图

### 5.1 主循环流程图

```
┌──────────────────────────────────────────────────────────┐
│                     程序入口 main()                       │
│  srand() → controller_create() → controller_run()        │
└──────────────────────────────────────────────────────────┘
                              │
                              ▼
┌──────────────────────────────────────────────────────────┐
│                   graphics_init()                        │
│  设置UTF-8 CP → 设控制台120×45 → 隐藏光标                 │
└──────────────────────────────────────────────────────────┘
                              │
                              ▼
┌──────────────────────────────────────────────────────────┐
│               graphics_show_welcome()                    │
│  ┌─────────────────────────────────────┐                 │
│  │       欢迎界面（阻塞等待按键）        │                 │
│  │   · 系统标题、作者信息               │                 │
│  │   · 操作说明（三列布局）             │                 │
│  │   · 按任意键开始                     │                 │
│  └─────────────────────────────────────┘                 │
└──────────────────────────────────────────────────────────┘
                              │
                              ▼
┌──────────────────────────────────────────────────────────┐
│              初始化位置 & 轨迹                            │
│  随机散布无人机 → 生成圆形编队 → 创建迁移轨迹              │
└──────────────────────────────────────────────────────────┘
                              │
                              ▼
              ┌───────────────────────────┐
              │     while (is_running)    │ ← 主循环 (20fps)
              └───────────────────────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        ▼                     ▼                     ▼
┌───────────────┐   ┌─────────────────┐   ┌─────────────────┐
│ ① 用户输入    │   │ ② 模拟更新      │   │ ③ 安全检测      │
│               │   │                 │   │                 │
│ _kbhit()?     │   │ STATE_RUNNING?  │   │ 边界检测 O(n)   │
│  ├─ No → 跳过 │   │  ├─ Yes:        │   │ 间距检测 O(n²)  │
│  └─ Yes:      │   │  │ traj_update  │   │ 生成警告日志    │
│     _getch()  │   │  │ fleet(300架) │   │                 │
│     映射UICmd │   │  │ elapsed+=50ms│   │                 │
│     → handle  │   │  └─ No: 跳过    │   │                 │
└───────────────┘   └─────────────────┘   └─────────────────┘
        │                     │                     │
        └─────────────────────┼─────────────────────┘
                              ▼
┌──────────────────────────────────────────────────────────┐
│              ④ ctrl_render_frame()  渲染                 │
│  fb_clear() → draw_title_bar() → draw_stage()           │
│  → draw_all_drones() → draw_warnings() → draw_panel()   │
│  → draw_bottom_bar() → fb_flush()                       │
└──────────────────────────────────────────────────────────┘
                              │
                              ▼
                    ┌─────────────────┐
                    │ ⑤ Sleep(50ms)   │
                    └─────────────────┘
                              │
                              ▼
                    循环回到 while 顶部
                              │
                     (用户按 ESC)
                              ▼
┌──────────────────────────────────────────────────────────┐
│  graphics_close() → controller_destroy() → return 0      │
└──────────────────────────────────────────────────────────┘
```

### 5.2 图案切换流程

```
用户按 ← / →
    │
    ▼
ctrl_switch_pattern(ctrl, direction)
    │
    ├─ ① history_add(ctrl)            ← 保存当前编队到历史
    │
    ├─ ② pattern_index += direction   ← 更新图案索引（循环）
    │
    ├─ ③ pattern_recommend()          ← 获取推荐参数
    │     (type, text_len → out_count, out_scale)
    │
    ├─ ④ formation_destroy(old)       ← 销毁旧编队
    │
    ├─ ⑤ formation_create(new)        ← 创建新编队
    │     └→ pattern_generate()
    │         └→ gen_xxx(center, scale, count, out[])
    │
    ├─ ⑥ 激活对应数量的无人机         ← 超出部分置 is_active=0
    │
    └─ ⑦ traj_from_formation()       ← 生成迁移轨迹
          为每架活跃无人机创建当前位置→新编队目标的航点
```

### 5.3 渲染帧流程

```
ctrl_render_frame()
    │
    ├─ graphics_clear()
    │   └→ fb_clear(): 120×45双重循环 → g_fb[r][c] = {L' ', CON_WHITE}
    │
    ├─ graphics_draw_title_bar()
    │   ├─ Row 0: ───────── 全宽 (CON_CYAN)
    │   ├─ Row 0: " 无人机编队灯光秀模拟系统 " (CON_YELLOW)
    │   ├─ Row 0: " 00:00 [待命] " (CON_GREEN, 右对齐)
    │   └─ Row 1: ───────── 全宽 (CON_GRAY)
    │
    ├─ graphics_draw_stage()
    │   ├─ 双线外框: ╔═...═╗ (80×40)
    │   ├─ 坐标刻度: 每10格 ╦╩╠╣
    │   ├─ 网格参考点: 每5格 · (CON_GRAY)
    │   └─ 中心十字线: ┄ (水平) + ┆ (垂直)
    │
    ├─ graphics_draw_all_drones()
    │   └→ for i in 0..drone_count:
    │       color = drone_get_current_color(fleet[i], delta_ms)
    │       fb_put_wchar(STAGE_LEFT + x, STAGE_TOP + y, color, L'●')
    │
    ├─ graphics_draw_warnings()
    │   ├─ 越界: ⚠ 越界警告: D#5 D#8 ...共30架
    │   └─ 碰撞: ⚠ 碰撞警告: #3-#7 #12-#15 ...
    │
    ├─ graphics_draw_panel()
    │   ├─ 列 84-117, 行 2-41
    │   ├─ 标题 → 状态/时间/数量/速度 → 分隔线
    │   ├─ 编队信息 → 分隔线
    │   ├─ 灯光设置(颜色指示器+模式) → 分隔线
    │   ├─ 安全状态 → 分隔线
    │   └─ 操作键 (5行)
    │
    ├─ graphics_draw_bottom_bar()
    │   └─ Row 44: ───────── + "总:300" + "活跃:30" + 提示
    │
    └─ graphics_flush()
        └→ fb_flush():
            ├─ 检测尺寸变化 → FillConsoleOutputCharacterW 全清
            └─ WriteConsoleOutputW(hOut, g_fb, 120×45, region)
```

---

## 6. 函数接口文档

### 6.1 无人机模块

```c
// 生命周期
Drone*  drone_create(int id, float x, float y, float height);
void    drone_destroy(Drone* drone);
Drone** drone_create_fleet(int count, int start_id);
void    drone_destroy_fleet(Drone** fleet, int count);

// 位置操作
void drone_set_position(Drone* drone, float x, float y, float height);
void drone_move(Drone* drone, float dx, float dy, float dh);
void drone_get_display_pos(const Drone* drone, int* x, int* y);

// 灯光操作
void drone_set_light_color(Drone* drone, LightColor color);
void drone_set_light_mode(Drone* drone, LightMode mode);
void drone_set_blink_interval(Drone* drone, int interval_ms);
void drone_light_onoff(Drone* drone, int on);

// 状态查询
int drone_is_active(const Drone* drone);
ConsoleColor drone_get_current_color(Drone* drone, int delta_ms);
```

### 6.2 编队模块

```c
// 生成器调度
int pattern_generate(PatternType type, Point2f center,
                     float scale, float rotation_deg,
                     int drone_count, Point2f out_positions[],
                     const char* text);

// 编队管理
Formation* formation_create(const char* name, PatternType type,
                            Point2f center, float scale, float rotation,
                            int drone_count, const char* display_text);
void       formation_destroy(Formation* formation);
void       formation_update(Formation* f, PatternType type,
                            Point2f center, float scale, float rotation);
int        formation_get_target(const Formation* f, int index,
                                float* x, float* y);

// 单个生成器（15+1种）
int gen_circle(Point2f center, float radius, int count, Point2f out[]);
int gen_square(Point2f center, float side, int count, Point2f out[]);
int gen_triangle(Point2f center, float size, int count, Point2f out[]);
int gen_diamond(Point2f center, float size, int count, Point2f out[]);
int gen_star(Point2f center, float size, int count, Point2f out[]);
int gen_pentagon(Point2f center, float size, int count, Point2f out[]);
int gen_hexagon(Point2f center, float size, int count, Point2f out[]);
int gen_heart(Point2f center, float size, int count, Point2f out[]);
int gen_spiral(Point2f center, float size, int count, Point2f out[]);
int gen_line(Point2f center, float length, int count, Point2f out[]);
int gen_arrow(Point2f center, float size, int count, Point2f out[]);
int gen_cross(Point2f center, float size, int count, Point2f out[]);
int gen_arc(Point2f center, float radius, int count, Point2f out[]);
int gen_grid(Point2f center, float size, int count, Point2f out[]);
int gen_random(Point2f center, float size, int count, Point2f out[]);
int gen_text(Point2f center, float char_size, const char* text,
             int count, Point2f out[]);

// 工具
void pattern_recommend(PatternType type, int text_len,
                       int* out_count, float* out_scale);
void rotate_point(float cx, float cy, float x, float y,
                  float deg, float* out_x, float* out_y);
```

### 6.3 轨迹模块

```c
// 生命周期
Trajectory* traj_create(void);
void        traj_destroy(Trajectory* traj);
int         traj_add_waypoint(Trajectory* traj, Point2f pos,
                              LightColor color, LightMode mode, int hold_ms);
void        traj_clear(Trajectory* traj);

// 逐帧更新
int  traj_update(Trajectory* traj, Drone* drone, float speed, int delta_ms);
void traj_jump_to(Trajectory* traj, Drone* drone, int index);
void traj_get_status(const Trajectory* traj,
                     int* out_index, int* out_total, float* out_progress);

// 批量操作
int traj_from_formation(Drone* fleet[], int count,
                        const Formation* f, Trajectory* trajs[],
                        float speed, LightColor color, LightMode mode);
int traj_update_fleet(Drone* fleet[], Trajectory* trajs[],
                      int count, float speed, int delta_ms);
```

### 6.4 安全模块

```c
SafetyZone* safety_zone_create(int x_min, int y_min, int x_max, int y_max,
                               int min_distance);
void safety_zone_destroy(SafetyZone* zone);
int  safety_point_in_zone(const SafetyZone* zone, float x, float y);
int  safety_check_boundary(Drone* fleet[], int count,
                           const SafetyZone* zone, SafetyResult* result);
int  safety_check_distance(Drone* fleet[], int count,
                           const SafetyZone* zone, SafetyResult* result);
int  safety_check_all(Drone* fleet[], int count,
                      const SafetyZone* zone, SafetyResult* result);
void safety_result_clear(SafetyResult* result);
void safety_avoid_collisions(Drone* fleet[], int count,
                             const SafetyZone* zone, float strength);
```

### 6.5 渲染模块

```c
// 窗口管理
void graphics_init(void);
void graphics_close(void);
void graphics_clear(void);
void graphics_flush(void);

// 绘制元素
void graphics_draw_stage(const SafetyZone* zone);
void graphics_draw_drone(const Drone* drone, ConsoleColor color);
void graphics_draw_all_drones(Drone* fleet[], int count, int delta_ms);
void graphics_draw_warnings(const SafetyResult* result);
void graphics_draw_panel(Drone* fleet[], int count, SimState state,
                         const Formation* f, int elapsed_ms,
                         float sim_speed, LightColor light_color,
                         LightMode light_mode, int has_warning);
void graphics_show_welcome(void);
void graphics_draw_title_bar(SimState state, int elapsed_ms);
void graphics_draw_bottom_bar(int drone_count, int active_count,
                              const char* hint_text);

// 工具
void graphics_gotoxy(int col, int row);
void graphics_set_color(ConsoleColor color);
void graphics_put_str(int col, int row, ConsoleColor color, const char* str);
```

### 6.6 灯光模块

```c
void light_fleet_set_color(Drone* fleet[], int count, LightColor color);
void light_fleet_set_mode(Drone* fleet[], int count, LightMode mode);
void light_fleet_set_blink(Drone* fleet[], int count, int interval_ms);
void light_wave_effect(Drone* fleet[], int count, LightColor color,
                       int delay_ms, int* elapsed_ms, int reset);
void light_alternate(Drone* fleet[], int count,
                     LightColor color_a, LightColor color_b,
                     int* phase, int interval_ms, int* timer_ms);
void light_flow(Drone* fleet[], int count, LightColor color,
                int window_size, int* offset, int speed_ms, int* timer_ms);
ConsoleColor light_color_lerp(LightColor from, LightColor to, float t);
const char*  color_to_name(LightColor color);
```

### 6.7 UI模块

```c
UICmd ui_poll_input(void);
int   ui_input_int(const char* prompt, int min_val, int max_val);
int   ui_confirm(const char* message);
int   ui_show_menu(void);
```

### 6.8 文件IO模块

```c
int  file_save_trajectory(Drone* fleet[], int count, const char* filename);
int  file_save_formation(const Formation* formation, const char* filename);
int  file_load_trajectory(Drone* fleet[], int* p_count, const char* filename);
Formation* file_load_formation(const char* filename);
int  file_list_trajectories(char out_list[][MAX_FILENAME_LEN], int max_count);
```

---

## 7. 用户操作指南

### 7.1 编译与运行

```bash
# 编译
g++ -std=c++11 -Wall -o drone_show.exe \
    main.cpp \
    src/drone.cpp src/light.cpp src/formation.cpp \
    src/trajectory.cpp src/safety.cpp \
    src/graphics.cpp src/ui.cpp src/file_io.cpp \
    src/controller.cpp \
    -I include

# 运行
./drone_show.exe
```

### 7.2 操作说明

| 类别 | 按键 | 功能 |
|------|------|------|
| **模拟控制** | `S` / `Enter` | 开始模拟 |
| | `P` / `Space` | 暂停/继续 |
| | `Q` | 停止模拟 |
| | `↑` / `↓` | 加速/减速 |
| | `ESC` | 退出程序 |
| **编队切换** | `←` / `→` | 切换15种几何图案 |
| | `T` | 输入文字编队(≤5字) |
| | `H` | 历史编队回溯(最近5次) |
| **灯光效果** | `C` | 切换颜色(8种) |
| | `B` | 闪烁模式开关 |

### 7.3 界面说明

```
┌──────────────────────────────────────────────────────────────────┐
│  无人机编队灯光秀模拟系统                          00:00 [待命]  │ ← 标题栏
├──────────────────────────────────────────────────────────────────┤
│ ┌──────────────────────────────┐ ┌────────────────────────────┐ │
│ │         表演区域              │ │     无人机灯光秀模拟       │ │
│ │                              │ ├────────────────────────────┤ │
│ │       ·  ·  ·  ·  ·         │ │ 状态 ○ 待命中...           │ │
│ │                              │ │ 时间 00:00                 │ │
│ │    ·     ●●●      ·         │ │ 数量 30 架                 │ │
│ │         ●●○○●               │ │ 速度 1.00x                 │ │
│ │    ·     ●●●      ·         │ ├────────────────────────────┤ │
│ │                              │ │ 当前编队                   │ │
│ │       ·  ·  ·  ·  ·         │ │ 圆形编队                   │ │
│ │                              │ │ 图案 圆形                  │ │
│ └──────────────────────────────┘ ├────────────────────────────┤ │
│                                  │ 灯光设置                   │ │
│                                  │ 颜色 ● 白色                │ │
│                                  │ 模式 常亮                  │ │
│                                  ├────────────────────────────┤ │
│                                  │ 安全状态                   │ │
│                                  │ ✔ 全部正常                 │ │
│                                  ├────────────────────────────┤ │
│                                  │ 操作键                     │ │
│                                  │ S    开始  P  暂停         │ │
│                                  │ Q    停止  ESC 退出        │ │
│                                  │ <->  图案  ^v  调速        │ │
│                                  │ C    换色  B  闪烁         │ │
│                                  │ T    文字  H  历史         │ │
│                                  └────────────────────────────┘ │
├──────────────────────────────────────────────────────────────────┤
│ 总:300  活跃:30              按 H 查看历史 | 按 T 输入文字        │ ← 状态栏
└──────────────────────────────────────────────────────────────────┘
```

---

> **文档版本**: 1.0  
> **最后更新**: 2026-06-10  
> **对应代码版本**: upstream/main @ c82c4ad
