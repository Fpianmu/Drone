/*
 * common.h — 整个项目共用的数据类型、常量和宏定义
 *
 * 所有模块都引用这个文件，确保类型定义全项目统一。
 * 如果新增了跨模块使用的结构体或枚举，加在这里。
 *
 * 渲染方案：Windows 控制台字符画，不用任何第三方图形库。
 * 控制台窗口 120 列 × 45 行，其中左侧 80 列是表演区，右侧是信息面板。
 */

#ifndef COMMON_H
#define COMMON_H

#include <windows.h>   // Windows API：控制台颜色、光标、GDI 文字渲染
#include <conio.h>     // _kbhit() / _getch() 键盘输入
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ================================================================
 * 系统常量 —— 控制画面布局和资源上限
 * ================================================================ */

// 控制台总尺寸（字符格为单位）
#define CONSOLE_WIDTH      120    // 总列数
#define CONSOLE_HEIGHT      45    // 总行数

// 表演区在控制台中的位置和大小
#define STAGE_LEFT           2    // 表演区左上角列号
#define STAGE_TOP            2    // 表演区左上角行号
#define STAGE_COLS          80    // 表演区宽（列）
#define STAGE_ROWS          40    // 表演区高（行）

// 右侧信息面板
#define PANEL_LEFT          (STAGE_LEFT + STAGE_COLS + 2)  // 面板起始列
#define PANEL_WIDTH         (CONSOLE_WIDTH - PANEL_LEFT - 2)

// 资源上限
#define MAX_DRONE_COUNT     300   // 最多支持的无人机总数
#define MAX_WAYPOINTS       200   // 单次轨迹最多航点数
#define MAX_FORMATIONS       50   // 最多编队数
#define MAX_FILENAME_LEN    128   // 文件名最大长度
#define MAX_WARNING_LEN     256   // 警告信息最大长度
#define WARN_LOG_SIZE         6   // 面板警告日志最大行数

// 帧率（毫秒/帧），≈20fps
#define FRAME_INTERVAL_MS    50

// 默认参数
#define BLINK_INTERVAL_MS   500   // 灯光闪烁默认间隔
#define SAFETY_MIN_DISTANCE   3   // 安全间距下限（字符格）
#define STAGE_MARGIN          1   // 表演区边界内缩

/* ================================================================
 * 控制台颜色常量 —— Windows Console API 的颜色属性
 *
 * 用 SetConsoleTextAttribute(h, color) 设置文字颜色。
 * 由前景色的红/绿/蓝 + 亮度位组合而成。
 * 背景色可通过左移 4 位组合，本项目只用前景色。
 * ================================================================ */

// ConsoleColor 就是 Windows 控制台的颜色属性类型（WORD = unsigned short）
typedef WORD ConsoleColor;

// 基本前景色定义（每个颜色由红/绿/蓝分量和亮度位组合）
#define CON_BLACK       0                                         // 黑色（灭灯）
#define CON_DARK_RED    FOREGROUND_RED                            // 暗红
#define CON_DARK_GREEN  FOREGROUND_GREEN                          // 暗绿
#define CON_DARK_BLUE   FOREGROUND_BLUE                           // 暗蓝
#define CON_RED         (FOREGROUND_RED   | FOREGROUND_INTENSITY) // 亮红
#define CON_GREEN       (FOREGROUND_GREEN | FOREGROUND_INTENSITY) // 亮绿
#define CON_BLUE        (FOREGROUND_BLUE  | FOREGROUND_INTENSITY) // 亮蓝
#define CON_WHITE       (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define CON_YELLOW      (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)   // 红+绿=黄
#define CON_CYAN        (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)   // 绿+蓝=青
#define CON_PURPLE      (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY)     // 红+蓝=紫
#define CON_ORANGE      (FOREGROUND_RED | FOREGROUND_GREEN)          // 暗黄的近似橙

// 灰色（无亮度位的白色）和暗黄
#define CON_GRAY        (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define CON_DARK_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN)

/* ================================================================
 * 枚举类型 —— 给整数常量起有意义的名字
 * ================================================================ */

// 灯光颜色（用于设置无人机灯光，也对应控制台颜色表中的索引）
typedef enum {
    COLOR_OFF   = 0,   // 熄灭
    COLOR_RED   = 1,   // 红色
    COLOR_GREEN = 2,   // 绿色
    COLOR_BLUE  = 3,   // 蓝色
    COLOR_WHITE = 4,   // 白色
    COLOR_YELLOW= 5,   // 黄色
    COLOR_CYAN  = 6,   // 青色
    COLOR_PURPLE= 7,   // 紫色
    COLOR_ORANGE= 8    // 橙色
} LightColor;

// 灯光模式 —— 决定无人机灯怎么亮
typedef enum {
    LIGHT_STEADY,      // 常亮：始终亮着不灭
    LIGHT_BLINK,       // 闪烁：按设定间隔一亮一灭
    LIGHT_OFF          // 关闭：完全熄灭
} LightMode;

// 图案类型 —— 所有支持的编队图案
typedef enum {
    PAT_NONE,          // 未选择
    PAT_CIRCLE,        // 圆形（等角度分布）
    PAT_SQUARE,        // 正方形（四边等距）
    PAT_TRIANGLE,      // 正三角形（三边等距）
    PAT_DIAMOND,       // 菱形
    PAT_STAR,          // 五角星（10 顶点）
    PAT_PENTAGON,      // 正五边形
    PAT_HEXAGON,       // 正六边形
    PAT_HEART,         // 心形（参数方程）
    PAT_SPIRAL,        // 阿基米德螺旋线
    PAT_LINE,          // 水平直线
    PAT_ARROW,         // 箭头形
    PAT_CROSS,         // 十字形
    PAT_ARC,           // 弧形
    PAT_GRID,          // 矩形网格
    PAT_RANDOM,        // 随机散布
    PAT_TEXT,          // 文字编队（GDI 渲染后降采样）
} PatternType;

// 模拟状态 —— 控制主循环的行为
typedef enum {
    STATE_IDLE,        // 待命中：画面显示但不动
    STATE_RUNNING,     // 运行中：无人机在飞
    STATE_PAUSED,      // 暂停：画面冻结
    STATE_REPLAY       // 回放：从文件重放（预留功能）
} SimState;

/* ================================================================
 * 结构体 —— 系统中流动的"数据包"
 * ================================================================ */

// 浮点坐标（用于精确的位置和插值计算）
typedef struct {
    float x;
    float y;
} Point2f;

// 整数坐标（用于最终显示位置）
typedef struct {
    int x;
    int y;
} Point2i;

// 单架无人机的灯光状态
typedef struct {
    LightColor  color;           // 当前颜色
    LightMode   mode;            // 常亮/闪烁/熄灭
    int         blink_interval;  // 闪烁间隔（毫秒）
    int         blink_timer;     // 闪烁计时器（累计毫秒，到间隔就翻转亮灭）
    int         is_visible;      // 当前帧是否可见（1=亮 0=灭）
} LightState;

/*
 * 单架无人机 —— 飞行表演的基本单位
 *
 * 每架无人机有自己的编号、三维位置（x,y 平面 + 高度）、
 * 灯光状态和活跃标记。所有无人机状态都封装在这个结构体里，
 * 通过指针操作，不依赖全局变量。
 */
typedef struct {
    int         id;              // 唯一编号
    Point2f     position;        // 当前位置（浮点，用于平滑移动）
    float       height;          // 飞行高度（米，目前主要做标注用）
    LightState  light;           // 灯光状态
    int         is_active;       // 是否参与当前编队（0=休眠，不渲染也不检测）
} Drone;

/*
 * 编队 —— 描述了"无人机应该排成什么图案"
 *
 * 包含图案类型、几何参数、以及每架无人机的目标位置。
 * 创建编队时自动调用图案生成器填充 targets 数组。
 * drone_count 记录实际使用的无人机数（可能少于池中总数）。
 */
typedef struct {
    int         formation_id;                    // 编队编号
    char        name[32];                        // 名称（如"圆形编队"）
    PatternType pattern;                         // 图案类型
    Point2f     center;                          // 图案中心坐标
    float       scale;                           // 缩放（不同图案含义不同：半径/边长等）
    float       rotation_deg;                    // 旋转角度（度）
    Point2f     targets[MAX_DRONE_COUNT];        // 每架飞机的目标坐标
    int         drone_count;                     // 实际需要多少架
    char        display_text[32];                // 文字编队的文本（仅 PAT_TEXT 用）
    float       recommended_scale;               // 推荐的缩放值（生成时填充）
} Formation;

// 单个航点 —— 轨迹上的一个关键位置
typedef struct {
    Point2f     position;         // 航点坐标
    LightColor  light_color;      // 到达后切换到的灯光颜色
    LightMode   light_mode;       // 到达后切换到的灯光模式
    int         hold_ms;          // 到达后停留多久（毫秒），0=不停留
} WayPoint;

/*
 * 轨迹 —— 由多个航点组成的飞行路线
 *
 * 当前航点用 current_index 指向。每帧沿直线插值前进，
 * 到达一个航点后自动切到下一个。
 */
typedef struct {
    WayPoint    waypoints[MAX_WAYPOINTS];  // 航点序列
    int         waypoint_count;             // 总航点数
    int         current_index;              // 当前要去第几个
    float       total_progress;             // 整体进度 0.0 ~ 1.0
} Trajectory;

// 表演区域的安全配置
typedef struct {
    int         x_min, y_min;    // 表演区左上角（字符格）
    int         x_max, y_max;    // 表演区右下角
    int         min_distance;    // 最小安全间距（小于此值视为危险接近）
} SafetyZone;

/*
 * 安全检测结果 —— 每帧输出
 *
 * 包含两部分：哪些无人机越界了、哪些飞机对距离太近。
 * 每种违规最多记录 MAX_DRONE_COUNT 条，超出的丢弃。
 */
typedef struct {
    int         boundary_violations;                // 越界的飞机数量
    int         boundary_ids[MAX_DRONE_COUNT];       // 越界飞机的 ID 列表
    int         distance_violations;                 // 间距不足的飞机对数量
    int         pair_a[MAX_DRONE_COUNT];             // 距离太近的飞机 A 的 ID
    int         pair_b[MAX_DRONE_COUNT];             // 距离太近的飞机 B 的 ID
} SafetyResult;

// 灯光秀脚本中的一步
typedef struct {
    int         step_id;         // 步骤编号
    PatternType pattern;         // 该步图案
    LightColor  light_color;     // 该步灯光颜色
    LightMode   light_mode;      // 该步灯光模式
    int         duration_ms;     // 持续时间
    int         transition_ms;   // 过渡时间
} ShowStep;

// 完整灯光秀脚本（预留功能，目前未接入主循环）
typedef struct {
    ShowStep    steps[MAX_WAYPOINTS];
    int         step_count;
    int         current_step;
    int         step_timer;
} ShowScript;

/* ================================================================
 * 工具宏
 * ================================================================ */

#define SAFE_FREE(ptr) do { if (ptr) { free(ptr); (ptr) = NULL; } } while(0)
#define CLAMP(val, lo, hi) (((val) < (lo)) ? (lo) : (((val) > (hi)) ? (hi) : (val)))
#define DISTANCE(x1, y1, x2, y2) \
    (float)(sqrt(((x1) - (x2)) * ((x1) - (x2)) + ((y1) - (y2)) * ((y1) - (y2))))
#define DEG2RAD(deg) ((float)(deg) * 3.14159265f / 180.0f)
#define RAD2DEG(rad) ((float)(rad) * 180.0f / 3.14159265f)

#endif
