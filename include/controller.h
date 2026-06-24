/*
 * controller.h - 主控制器模块
 *
 * 这是整个系统的"大脑"，管理全部状态，运行主循环。
 * 每帧做的事情：
 *   1. 检查用户输入
 *   2. 如果在 RUNNING 状态：更新轨迹 + 灯光特效
 *   3. 安全检测（始终运行）
 *   4. 渲染画面
 *   5. Sleep 50ms（大约 20 帧/秒）
 *
 * 所有状态都在 Controller 结构体里，不使用全局变量。
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

#define HISTORY_SIZE 5   // 历史编队最多保存几个

// 一次编队的历史记录（存图案类型、文字、颜色、模式）
typedef struct {
    PatternType pattern;
    char        text[32];
    LightColor  color;
    LightMode   mode;
} FormationHistory;

/*
 * 控制器上下文 - 整个程序的所有状态都在这里
 *
 * controller_create 会初始化所有子系统，
 * controller_destroy 会释放全部资源。
 */
typedef struct {
    // 无人机编队
    Drone*      fleet[MAX_DRONE_COUNT];   // 所有无人机指针（池）
    int         drone_count;               // 池里一共有多少架

    // 当前编队
    Formation*  current_formation;
    int         pattern_index;             // 在图案列表中的序号

    // 每架飞机的轨迹
    Trajectory* trajectories[MAX_DRONE_COUNT];

    // 安全检测
    SafetyZone*   safety_zone;
    SafetyResult  safety_result;

    // 灯光特效的运行状态（各种计时器和偏移量）
    int         wave_elapsed_ms;
    int         alt_phase, alt_timer_ms;
    int         flow_offset, flow_timer_ms;
    int         color_cycle_timer;
    int         color_cycle_idx;

    // 模拟状态
    SimState    sim_state;
    int         sim_elapsed_ms;
    float       sim_speed;       // 速度倍率（1.0 = 正常）
    int         is_running;      // 主循环是否继续

    // 用户当前选择
    PatternType selected_pattern;
    LightColor  selected_color;
    LightMode   selected_light_mode;
    LightFX     light_fx;

    // 历史记录
    FormationHistory history[HISTORY_SIZE];
    int              history_count;

    // 警告日志（面板里滚动显示的那几行）
    char warn_log[WARN_LOG_SIZE][MAX_WARNING_LEN];
    int  warn_log_count;
} Controller;

Controller* controller_create(void);
void         controller_destroy(Controller* ctrl);
void         controller_run(Controller* ctrl);  // 主循环从这里进

// 内部用的函数
void ctrl_handle_command(Controller* ctrl, UICmd cmd);
void ctrl_update_frame(Controller* ctrl, int delta_ms);
void ctrl_render_frame(Controller* ctrl, int delta_ms);
void ctrl_switch_pattern(Controller* ctrl, int direction);
void ctrl_init_default_formation(Controller* ctrl);

#endif
