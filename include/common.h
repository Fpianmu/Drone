/*
 * common.h - 整个项目共用的数据类型和常量
 *
 * 所有模块都引入这个文件，保证类型定义一致。
 * 如果新增了跨模块使用的结构体或枚举，加到这里。
 *
 * 显示方案：Windows 控制台字符画，120 列 x 40 行
 * 左侧 80x36 是表演区，右侧是信息面板，顶部是标题栏，底部是状态栏
 */

#ifndef COMMON_H
#define COMMON_H

#include <windows.h>   // 控制台颜色、光标、GDI 文字渲染
#include <conio.h>     // _kbhit() / _getch() 键盘输入
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ================================================================
 * 系统常量 - 控制画面布局和资源上限
 * ================================================================ */

#define CONSOLE_WIDTH      120    // 控制台总列数
#define CONSOLE_HEIGHT      40    // 控制台总行数

// 表演区在控制台中的位置和大小
#define STAGE_LEFT           2    // 表演区左上角列号（从0开始）
#define STAGE_TOP            1    // 表演区左上角行号（标题栏下方）
#define STAGE_COLS          80    // 表演区宽（列数）
#define STAGE_ROWS          36    // 表演区高（行数）

// 右侧信息面板
#define PANEL_LEFT          (STAGE_LEFT + STAGE_COLS + 2)  // 面板起始列 = 84
#define PANEL_WIDTH         (CONSOLE_WIDTH - PANEL_LEFT - 2)

// 各种上限
#define MAX_DRONE_COUNT     500   // 最多同时存在的无人机数量
#define MAX_WAYPOINTS       200   // 单条轨迹最多航点数
#define MAX_FORMATIONS       50   // 最多编队数
#define MAX_FILENAME_LEN    128   // 文件名最大长度
#define MAX_WARNING_LEN     256   // 警告信息最大长度
#define WARN_LOG_SIZE         6   // 面板警告日志最多显示行数

// 帧率参数
#define FRAME_INTERVAL_MS    50   // 每帧间隔（毫秒），约等于 20 帧/秒

// 默认参数
#define BLINK_INTERVAL_MS   500   // 灯光闪烁默认间隔（毫秒）
#define SAFETY_MIN_DISTANCE   3   // 安全间距下限（字符格数）
#define STAGE_MARGIN          1   // 表演区边界向内收缩的边距

/* ================================================================
 * 控制台颜色常量
 *
 * 这些常量用于 SetConsoleTextAttribute(h, color) 设置文字颜色。
 * 由前景色的红、绿、蓝分量和亮度位组合而成。
 * 本项目只使用前景色，背景统一为黑色。
 * ================================================================ */

typedef WORD ConsoleColor;  // 就是 Windows 控制台的颜色属性类型

#define CON_BLACK       0                                         // 黑色（灭灯时用）
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
#define CON_ORANGE      (FOREGROUND_RED | FOREGROUND_GREEN)          // 暗黄的近似橙色
#define CON_GRAY        (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)  // 灰色
#define CON_DARK_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN)

/* ================================================================
 * 枚举类型
 * ================================================================ */

// 灯光颜色（8 种可选颜色 + 熄灭）
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

// 灯光基础模式
typedef enum {
    LIGHT_STEADY,      // 常亮：一直亮着
    LIGHT_BLINK,       // 闪烁：一亮一灭来回切换
    LIGHT_OFF          // 关闭：完全熄灭
} LightMode;

// 灯光特效模式（按 E 键循环切换）
typedef enum {
    FX_NONE = 0,       // 无特效（保持基础常亮或闪烁）
    FX_WAVE,           // 波浪点亮：无人机像多米诺骨牌一样依次亮起
    FX_FLOW,           // 流水灯：一段亮灯窗口在编队中循环滑过
    FX_ALTERNATE,      // 交替闪烁：奇数号和偶数号的灯交替亮灭
    FX_COLOR_FLOW,     // 彩虹流水：流水灯 + 颜色自动渐变
    FX_COUNT           // 特效总数（用于循环判断）
} LightFX;

// 图案类型（全部支持的编队图案）
typedef enum {
    PAT_NONE,          // 未选择
    PAT_CIRCLE,        // 圆形（等角度分布在圆周上）
    PAT_SQUARE,        // 正方形（四边等距排列）
    PAT_TRIANGLE,      // 正三角形
    PAT_DIAMOND,       // 菱形
    PAT_STAR,          // 五角星（10 个顶点）
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
    PAT_TEXT,          // 文字编队（GDI 渲染后采样）
    PAT_IMAGE,         // BMP 图片编队
} PatternType;

// 模拟运行状态
typedef enum {
    STATE_IDLE,        // 待命中：画面显示但无人机不动
    STATE_RUNNING,     // 运行中：无人机在飞行
    STATE_PAUSED,      // 暂停：画面冻结
    STATE_REPLAY       // 回放模式（预留）
} SimState;

/* ================================================================
 * 结构体
 * ================================================================ */

// 浮点坐标（用于精确的位置计算和插值）
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
    int         blink_timer;     // 闪烁计时器（累计毫秒，到间隔就翻转）
    int         is_visible;      // 当前帧是否可见（1=亮, 0=灭）
} LightState;

/*
 * 单架无人机
 *
 * 每架无人机有编号、三维位置（x,y 平面坐标 + 高度）、
 * 灯光状态和一个"是否活跃"的标记。
 * 活跃 = 参与当前编队，休眠 = 不渲染也不检测。
 * 所有状态在这里，通过指针传递，不依赖全局变量。
 */
typedef struct {
    int         id;              // 编号（唯一标识，从1开始）
    Point2f     position;        // 当前平面坐标（浮点，用于平滑移动）
    float       height;          // 飞行高度（米）
    LightState  light;           // 灯光状态
    int         is_active;       // 是否参与当前编队（1=活跃, 0=休眠）
} Drone;

/*
 * 编队
 *
 * 描述了一组无人机应该排成什么图案。
 * 包含图案类型、几何参数、每架飞机的目标坐标。
 * drone_count 记录这个编队用了多少架飞机（<= 总池大小）。
 */
typedef struct {
    int         formation_id;                    // 编队编号
    char        name[32];                        // 编队名称（如"Circle"）
    PatternType pattern;                         // 图案类型
    Point2f     center;                          // 图案中心坐标
    float       scale;                           // 缩放大小（圆=半径，正方=边长...）
    float       rotation_deg;                    // 旋转角度（度）
    Point2f     targets[MAX_DRONE_COUNT];        // 每架飞机的目标位置
    int         drone_count;                     // 这个编队实际用了多少架
    char        display_text[32];                // 文字编队时显示的文字内容
    float       recommended_scale;               // 推荐缩放值（生成时填入）
} Formation;

// 一个航点：轨迹上的一个关键位置
typedef struct {
    Point2f     position;         // 坐标
    LightColor  light_color;      // 到达后切换的颜色
    LightMode   light_mode;       // 到达后切换的模式
    int         hold_ms;          // 到达后停留时间（毫秒），0 = 不停留
} WayPoint;

/*
 * 轨迹
 *
 * 一串航点组成飞行路线。current_index 指向当前要去的航点。
 * 每帧沿当前航点方向前进一小步，到达后自动切到下一个。
 */
typedef struct {
    WayPoint    waypoints[MAX_WAYPOINTS];  // 航点序列
    int         waypoint_count;             // 航点总数
    int         current_index;              // 当前要去第几个
    float       total_progress;             // 整体进度 0.0 ~ 1.0
    int         departure_delay;            // 随机出发延迟（毫秒），0 表示立刻出发
} Trajectory;

// 安全区域配置
typedef struct {
    int         x_min, y_min;    // 表演区左上角
    int         x_max, y_max;    // 表演区右下角
    int         min_distance;    // 最小安全间距（小于这个值视为危险接近）
} SafetyZone;

/*
 * 安全检测结果
 *
 * 每帧检测后填充。包含：
 * - 哪些无人机越界了（ID 列表）
 * - 哪些无人机对距离太近（ID 对列表）
 */
typedef struct {
    int         boundary_violations;                // 越界的无人机数量
    int         boundary_ids[MAX_DRONE_COUNT];       // 越界无人机的 ID 列表
    int         distance_violations;                 // 距离太近的无人机对数量
    int         pair_a[MAX_DRONE_COUNT];             // 太近的无人机 A 的 ID
    int         pair_b[MAX_DRONE_COUNT];             // 太近的无人机 B 的 ID
} SafetyResult;

// 灯光秀脚本中的一步（预留，目前未用到）
typedef struct {
    int         step_id;
    PatternType pattern;
    LightColor  light_color;
    LightMode   light_mode;
    int         duration_ms;
    int         transition_ms;
} ShowStep;

// 完整的灯光秀脚本（预留，目前未用到）
typedef struct {
    ShowStep    steps[MAX_WAYPOINTS];
    int         step_count;
    int         current_step;
    int         step_timer;
} ShowScript;

/* ================================================================
 * 工具宏
 * ================================================================ */

// 安全释放内存并置空
#define SAFE_FREE(ptr) do { if (ptr) { free(ptr); (ptr) = NULL; } } while(0)

// 把值限制在 [lo, hi] 范围内
#define CLAMP(val, lo, hi) (((val) < (lo)) ? (lo) : (((val) > (hi)) ? (hi) : (val)))

// 两点之间的欧氏距离
#define DISTANCE(x1, y1, x2, y2) \
    (float)(sqrt(((x1) - (x2)) * ((x1) - (x2)) + ((y1) - (y2)) * ((y1) - (y2))))

// 角度和弧度互转
#define DEG2RAD(deg) ((float)(deg) * 3.14159265f / 180.0f)
#define RAD2DEG(rad) ((float)(rad) * 180.0f / 3.14159265f)

#endif
