/*
 * controller.h —— 主控制器模块
 *
 * 这是整个系统的"指挥中心"，拥有所有子系统（飞机、编队、轨迹、安全、
 * 渲染、输入、文件）的全部状态。主循环在这里运行，每帧依次：
 *   1. 检查用户输入
 *   2. 如果在运行状态，更新轨迹（飞机移动）
 *   3. 始终运行安全检测
 *   4. 渲染画面
 *   5. 控制帧率（Sleep 50ms ≈ 20fps）
 *
 * 所有状态都封装在 Controller 结构体中，不使用全局变量。
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

#define HISTORY_SIZE 5   // 历史编队记录上限

// 一次编队历史（存图案类型、文字、颜色、模式）
typedef struct {
    PatternType pattern;
    char        text[32];
    LightColor  color;
    LightMode   mode;
} FormationHistory;

/*
 * 控制器上下文 —— 整个程序的"大脑"
 *
 * 这个结构体拥有所有状态，各模块通过操作指针而非全局变量来工作。
 * controller_create 初始化所有子系统，controller_destroy 释放全部资源。
 */
typedef struct {
    // 无人机编队
    Drone*      fleet[MAX_DRONE_COUNT];   // 所有无人机指针（池）
    int         drone_count;               // 池中总数

    // 当前编队
    Formation*  current_formation;         // 当前正在使用的编队
    int         pattern_index;             // 当前图案在图案列表中的索引

    // 每架飞机的轨迹
    Trajectory* trajectories[MAX_DRONE_COUNT];

    // 安全
    SafetyZone*   safety_zone;
    SafetyResult  safety_result;
    int           warning_visible;         // 警告闪烁计数器

    // 灯光效果状态（给 light_* 动态效果用）
    int         wave_elapsed_ms;
    int         alt_phase, alt_timer_ms;
    int         flow_offset, flow_timer_ms;

    // 模拟状态
    SimState    sim_state;
    int         sim_elapsed_ms;
    float       sim_speed;       // 速度倍率（1.0=正常）
    int         is_running;      // 主循环是否继续

    // 用户选择
    PatternType selected_pattern;
    LightColor  selected_color;
    LightMode   selected_light_mode;

    // 历史记录
    FormationHistory history[HISTORY_SIZE];
    int              history_count;

    // 警告日志（面板滚动显示）
    char warn_log[WARN_LOG_SIZE][MAX_WARNING_LEN];
    int  warn_log_count;     // 当前日志条数（最多 WARN_LOG_SIZE）
} Controller;

Controller* controller_create(void);
void         controller_destroy(Controller* ctrl);

// 进入主循环（初始化控制台 → 欢迎界面 → 帧循环 → 清理）
void controller_run(Controller* ctrl);

// 内部使用
void ctrl_handle_command(Controller* ctrl, UICmd cmd);
void ctrl_update_frame(Controller* ctrl, int delta_ms);
void ctrl_render_frame(Controller* ctrl, int delta_ms);
void ctrl_switch_pattern(Controller* ctrl, int direction);
void ctrl_init_default_formation(Controller* ctrl);

#endif
