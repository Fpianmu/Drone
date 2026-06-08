/**
 * @file    controller.h
 * @brief   主控制器模块 —— 模拟主循环与模块调度
 * @author  [与队友协商]
 * @date    2026-06-08
 *
 * 本模块是系统的"指挥中心"，负责：
 * - 初始化所有子系统
 * - 运行主模拟循环（每帧调用各模块）
 * - 处理用户输入 → 转化为系统操作
 * - 管理模拟状态机（IDLE → RUNNING → PAUSED → STOPPED）
 *
 * 主循环流程（每帧）：
 *   1. 检查用户输入（UI 模块）→ 处理状态切换
 *   2. 如果 RUNNING：
 *      a. 轨迹更新（Trajectory 模块）
 *      b. 安全检测（Safety 模块）
 *      c. 灯光刷新（通过 drone_get_current_color）
 *      d. 渲染（Graphics 模块）
 *   3. 帧率控制（Sleep 33ms）
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "common.h"
#include "drone.h"
#include "formation.h"
#include "trajectory.h"
#include "safety.h"
#include "light.h"
#include "graphics.h"
#include "ui.h"
#include "file_io.h"

/* ==================== 控制器状态 ==================== */

/**
 * @brief 编队历史记录
 */
typedef struct {
    PatternType pattern;
    char        text[32];
    LightColor  color;
    LightMode   mode;
} FormationHistory;

#define HISTORY_SIZE 5

/**
 * @brief 控制器上下文（管理所有状态，不使用全局变量）
 */
typedef struct {
    // 无人机编队
    Drone*      fleet[MAX_DRONE_COUNT];   // 无人机指针数组
    int         drone_count;              // 实际无人机数量

    // 当前编队
    Formation*  current_formation;         // 当前编队配置
    int         pattern_index;            // 当前图案索引（供菜单切换）

    // 轨迹数组（每架无人机一条轨迹）
    Trajectory* trajectories[MAX_DRONE_COUNT];

    // 安全区域
    SafetyZone* safety_zone;

    // 安全检测结果
    SafetyResult safety_result;
    int          warning_visible;         // 警告闪烁计数器

    // 灯光效果状态
    int         wave_elapsed_ms;
    int         alt_phase;
    int         alt_timer_ms;
    int         flow_offset;
    int         flow_timer_ms;

    // 模拟状态
    SimState    sim_state;
    int         sim_elapsed_ms;
    float       sim_speed;
    int         is_running;

    // 用户选中的特性
    PatternType selected_pattern;
    LightColor  selected_color;
    LightMode   selected_light_mode;

    // 编队历史记录（最近5次）
    FormationHistory history[HISTORY_SIZE];
    int              history_count;
} Controller;

/* ==================== 控制器接口 ==================== */

/**
 * @brief 创建并初始化控制器
 * @return 堆上的 Controller 指针
 */
Controller* controller_create(void);

/**
 * @brief 销毁控制器，释放所有资源
 * @param ctrl 控制器
 */
void controller_destroy(Controller* ctrl);

/**
 * @brief 运行主循环
 *
 * 初始化控制台窗口 → 显示欢迎界面 → 进入主循环 → 退出时清理。
 * 这是程序的顶层入口函数（由 main.cpp 调用）。
 *
 * @param ctrl 已初始化的控制器
 */
void controller_run(Controller* ctrl);

/* ==================== 内部函数（供 controller.cpp 内部使用） ==================== */

/** @brief 处理用户操作并修改控制器状态 */
void ctrl_handle_command(Controller* ctrl, UICmd cmd);

/** @brief 执行一帧模拟（更新轨迹 + 安全检测 + 灯光） */
void ctrl_update_frame(Controller* ctrl, int delta_ms);

/** @brief 执行一帧渲染（绘制所有元素） */
void ctrl_render_frame(Controller* ctrl, int delta_ms);

/** @brief 切换编队图案 */
void ctrl_switch_pattern(Controller* ctrl, int direction);

/** @brief 生成初始编队（默认圆形） */
void ctrl_init_default_formation(Controller* ctrl);

#endif // CONTROLLER_H
