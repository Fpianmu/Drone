/**
 * @file    common.h
 * @brief   无人机编队灯光秀 —— 公共类型定义与全局常量
 * @author  [你的名字]
 * @date    2026-06-08
 *
 * 本文件包含了项目中所有模块共用的数据结构、枚举常量和宏定义。
 * 所有模块均 include 此文件，确保类型定义的一致性。
 *
 * 渲染方案：Windows 控制台字符画（无需 EasyX）
 *   使用 Windows Console API 实现彩色字符显示与光标定位。
 */

#ifndef COMMON_H
#define COMMON_H

#include <windows.h>    // Windows API（控制台颜色、光标定位）
#include <conio.h>      // _kbhit() / _getch()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ==================== 系统常量 ==================== */

// 控制台字符尺寸（列 × 行）
#define CONSOLE_WIDTH      120    // 控制台总列数
#define CONSOLE_HEIGHT      45    // 控制台总行数

// 表演区与控制台坐标映射
#define STAGE_LEFT           2    // 表演区左边距（列）
#define STAGE_TOP            2    // 表演区上边距（行）
#define STAGE_COLS          80    // 表演区宽度（列）
#define STAGE_ROWS          40    // 表演区高度（行）

// 右侧信息面板
#define PANEL_LEFT          (STAGE_LEFT + STAGE_COLS + 2)  // 面板起始列
#define PANEL_WIDTH         (CONSOLE_WIDTH - PANEL_LEFT - 2)

// 无人机数量上限
#define MAX_DRONE_COUNT     300  // 最大无人机数量（文字编队需要较多）
#define MAX_WAYPOINTS       200  // 单次轨迹最大关键点数量
#define MAX_FORMATIONS       50  // 最多支持的编队数量
#define MAX_FILENAME_LEN    128  // 文件路径最大长度
#define MAX_WARNING_LEN     256  // 警告信息最大长度

// 帧率控制
#define FRAME_INTERVAL_MS    50  // 每帧间隔 50ms（≈20fps，控制台刷新够用）

// 灯光默认值
#define BLINK_INTERVAL_MS   500  // 默认闪烁间隔（毫秒）

// 安全检测默认值
#define SAFETY_MIN_DISTANCE   3  // 最小安全间距（控制台字符格）
#define STAGE_MARGIN          1  // 表演区内边距

/* ==================== 控制台颜色常量 ==================== */

/**
 * @brief 控制台颜色类型（Windows Console API 颜色属性 WORD）
 *
 * 使用 SetConsoleTextAttribute(hConsole, color) 设置文本颜色。
 * 支持 16 种基本颜色（前景/背景组合）。
 *
 * 背景色 = 颜色值左移 4 位，但通常只设前景色。
 */
typedef WORD ConsoleColor;

// 基本前景色（可组合 FOREGROUND_INTENSITY 增亮）
#define CON_BLACK       0
#define CON_DARK_RED    FOREGROUND_RED
#define CON_DARK_GREEN  FOREGROUND_GREEN
#define CON_DARK_BLUE   FOREGROUND_BLUE
#define CON_RED         (FOREGROUND_RED   | FOREGROUND_INTENSITY)
#define CON_GREEN       (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define CON_BLUE        (FOREGROUND_BLUE  | FOREGROUND_INTENSITY)
#define CON_WHITE       (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define CON_YELLOW      (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define CON_CYAN        (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define CON_PURPLE      (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define CON_ORANGE      (FOREGROUND_RED | FOREGROUND_GREEN)

// 特殊色：灰色、暗黄
#define CON_GRAY        (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define CON_DARK_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN)

/* ==================== 枚举类型 ==================== */

/**
 * @brief 灯光颜色枚举
 *        与控制台颜色的映射由 drone.cpp 中的颜色表完成
 */
typedef enum {
    COLOR_OFF   =  0,   // 熄灭（黑色）
    COLOR_RED   =  1,   // 红色
    COLOR_GREEN =  2,   // 绿色
    COLOR_BLUE  =  3,   // 蓝色
    COLOR_WHITE =  4,   // 白色
    COLOR_YELLOW=  5,   // 黄色
    COLOR_CYAN  =  6,   // 青色
    COLOR_PURPLE=  7,   // 紫色
    COLOR_ORANGE=  8    // 橙色
} LightColor;

/**
 * @brief 无人机灯光模式
 */
typedef enum {
    LIGHT_STEADY,       // 常亮
    LIGHT_BLINK,        // 闪烁
    LIGHT_OFF           // 关闭
} LightMode;

/**
 * @brief 图案类型枚举（生成器参数）
 */
typedef enum {
    PAT_NONE,           // 无图案
    PAT_CIRCLE,         // 圆形
    PAT_SQUARE,         // 正方形
    PAT_TRIANGLE,       // 正三角形
    PAT_DIAMOND,        // 菱形
    PAT_STAR,           // 五角星
    PAT_PENTAGON,       // 正五边形
    PAT_HEXAGON,        // 正六边形
    PAT_HEART,          // 心形
    PAT_SPIRAL,         // 螺旋线
    PAT_LINE,           // 直线排布
    PAT_ARROW,          // 箭头形
    PAT_CROSS,          // 十字形
    PAT_ARC,            // 弧形
    PAT_GRID,           // 网格排布
    PAT_RANDOM,         // 随机散布
    PAT_TEXT,           // 文字点阵（英文/数字/中文）
} PatternType;

/**
 * @brief 模拟运行状态
 */
typedef enum {
    STATE_IDLE,         // 空闲（未开始）
    STATE_RUNNING,      // 模拟运行中
    STATE_PAUSED,       // 已暂停
    STATE_REPLAY        // 回放模式
} SimState;

/* ==================== 结构体 ==================== */

/**
 * @brief 二维坐标（浮点，用于精确插值计算）
 */
typedef struct {
    float x;
    float y;
} Point2f;

/**
 * @brief 二维坐标（整数，用于显示）
 */
typedef struct {
    int x;
    int y;
} Point2i;

/**
 * @brief 无人机灯光状态
 */
typedef struct {
    LightColor  color;          // 当前灯光颜色
    LightMode   mode;           // 灯光模式（常亮/闪烁/关闭）
    int         blink_interval; // 闪烁间隔（毫秒）
    int         blink_timer;    // 闪烁计时器（累计毫秒）
    int         is_visible;     // 当前是否可见（1=亮, 0=灭，供闪烁切换）
} LightState;

/**
 * @brief 单架无人机实体
 *
 * 所有状态封装在结构体内，不使用全局变量。
 */
typedef struct {
    int         id;             // 无人机编号（唯一标识）
    Point2f     position;       // 当前位置（浮点坐标）
    float       height;         // 当前飞行高度（米）
    LightState  light;          // 灯光状态
    int         is_active;      // 是否活跃（1=参与编队, 0=已停用）
} Drone;

/**
 * @brief 编队定义
 *
 * 一个编队 = 一种图案 + 所有无人机在该图案中的目标位置。
 */
typedef struct {
    int         formation_id;   // 编队编号
    char        name[32];       // 编队名称（如"圆形编队"）
    PatternType pattern;        // 使用的图案类型
    Point2f     center;         // 图案中心坐标
    float       scale;          // 缩放因子
    float       rotation_deg;   // 旋转角度（度）
    Point2f     targets[MAX_DRONE_COUNT]; // 每架无人机的目标位置
    int         drone_count;    // 该编队实际使用的无人机数量（由生成器决定）
    char        display_text[32]; // 文字编队的显示文本（PAT_TEXT 时有效）
    float       recommended_scale; // 该图案的推荐缩放值
} Formation;

/**
 * @brief 关键航点（用于轨迹定义）
 */
typedef struct {
    Point2f     position;       // 航点坐标
    LightColor  light_color;    // 到达该航点后的灯光颜色
    LightMode   light_mode;     // 到达该航点后的灯光模式
    int         hold_ms;        // 在该航点停留时间（毫秒），0=不停留
} WayPoint;

/**
 * @brief 轨迹序列（一组航点构成完整飞行轨迹）
 */
typedef struct {
    WayPoint    waypoints[MAX_WAYPOINTS]; // 航点数组
    int         waypoint_count;           // 航点数量
    int         current_index;            // 当前目标航点索引
    float       total_progress;           // 整体进度 [0.0, 1.0]
} Trajectory;

/**
 * @brief 安全检测区域配置
 */
typedef struct {
    int         x_min;          // 表演区左边界
    int         y_min;          // 表演区上边界
    int         x_max;          // 表演区右边界
    int         y_max;          // 表演区下边界
    int         min_distance;   // 无人机最小安全间距
} SafetyZone;

/**
 * @brief 安全检测结果
 */
typedef struct {
    int         boundary_violations;                // 越界的无人机数量
    int         boundary_ids[MAX_DRONE_COUNT];      // 越界无人机ID列表
    int         distance_violations;                 // 间距违规的无人机对数量
    int         pair_a[MAX_DRONE_COUNT];             // 间距违规对——无人机A
    int         pair_b[MAX_DRONE_COUNT];             // 间距违规对——无人机B
} SafetyResult;

/**
 * @brief 灯光秀脚本条目（编排一场完整的灯光秀）
 */
typedef struct {
    int         step_id;        // 步骤编号
    PatternType pattern;        // 该步骤的图案
    LightColor  light_color;    // 该步骤的灯光颜色
    LightMode   light_mode;     // 该步骤的灯光模式
    int         duration_ms;    // 该步骤持续时间（毫秒）
    int         transition_ms;  // 过渡动画时间（毫秒）
} ShowStep;

/**
 * @brief 完整灯光秀脚本
 */
typedef struct {
    ShowStep    steps[MAX_WAYPOINTS];  // 步骤数组
    int         step_count;            // 总步骤数
    int         current_step;          // 当前步骤索引
    int         step_timer;            // 当前步骤已过毫秒数
} ShowScript;

/* ==================== 工具宏 ==================== */

#define SAFE_FREE(ptr) do { if (ptr) { free(ptr); (ptr) = NULL; } } while(0)
#define CLAMP(val, lo, hi) (((val) < (lo)) ? (lo) : (((val) > (hi)) ? (hi) : (val)))
#define DISTANCE(x1, y1, x2, y2) \
    (float)(sqrt(((x1) - (x2)) * ((x1) - (x2)) + ((y1) - (y2)) * ((y1) - (y2))))
#define DEG2RAD(deg) ((float)(deg) * 3.14159265f / 180.0f)
#define RAD2DEG(rad) ((float)(rad) * 180.0f / 3.14159265f)

#endif // COMMON_H
