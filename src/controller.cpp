/*
 * controller.cpp —— 主控制器模块实现
 *
 * 这是所有子系统的粘合剂。主循环每帧依次执行：
 *   ① ui_poll_input() 检查按键 → 处理命令
 *   ② 如果在 RUNNING 状态 → 轨迹更新（飞机移动）
 *   ③ 安全检测（始终运行）
 *   ④ 渲染（清帧缓冲 → 画场景 → 画飞机 → 画面板 → 写控制台）
 *   ⑤ Sleep(50ms) ≈ 20fps
 */

#include "../include/controller.h"

/* ==================== 常量 ==================== */

#define DEFAULT_SPEED       10.0f
#define DEFAULT_DRONE_COUNT 300
#define MIN_SPEED            2.0f
#define MAX_SPEED            40.0f

/* 前置声明 */
static void history_add(Controller* ctrl);

/* ==================== 可选图案列表（供快速切换用） ==================== */

static const PatternType g_pattern_list[] = {
    PAT_CIRCLE,   PAT_SQUARE,   PAT_TRIANGLE,  PAT_DIAMOND,
    PAT_STAR,     PAT_PENTAGON, PAT_HEXAGON,   PAT_HEART,
    PAT_SPIRAL,   PAT_LINE,     PAT_ARROW,     PAT_CROSS,
    PAT_ARC,      PAT_GRID,     PAT_RANDOM,
};
#define PATTERN_COUNT (sizeof(g_pattern_list) / sizeof(g_pattern_list[0]))

/* ==================== 生命周期 ==================== */

Controller* controller_create(void)
{
    Controller* ctrl = (Controller*)malloc(sizeof(Controller));
    if (ctrl == NULL) return NULL;

    // 初始化为零
    memset(ctrl, 0, sizeof(Controller));

    // 设置默认值
    ctrl->drone_count      = DEFAULT_DRONE_COUNT;
    ctrl->sim_speed         = 1.0f;
    ctrl->is_running        = 1;
    ctrl->sim_state         = STATE_IDLE;
    ctrl->sim_elapsed_ms    = 0;
    ctrl->pattern_index     = 0;
    ctrl->selected_pattern  = PAT_CIRCLE;
    ctrl->selected_color    = COLOR_WHITE;
    ctrl->selected_light_mode = LIGHT_STEADY;

    ctrl->current_formation = NULL;

    // 创建无人机编队
    Drone** fleet_ptr = drone_create_fleet(ctrl->drone_count, 1);
    if (fleet_ptr == NULL) {
        free(ctrl);
        return NULL;
    }
    for (int i = 0; i < ctrl->drone_count; i++) {
        ctrl->fleet[i] = fleet_ptr[i];
    }
    free(fleet_ptr);  // 释放指针数组，保留无人机指针

    // 创建安全区域（边界 = 表演区，坐标范围对应控制台字符格）
    ctrl->safety_zone = safety_zone_create(
        STAGE_MARGIN, STAGE_MARGIN,
        STAGE_COLS - STAGE_MARGIN, STAGE_ROWS - STAGE_MARGIN,
        SAFETY_MIN_DISTANCE
    );

    // 生成默认编队
    ctrl_init_default_formation(ctrl);

    return ctrl;
}

void controller_destroy(Controller* ctrl)
{
    if (ctrl == NULL) return;

    // 释放编队
    formation_destroy(ctrl->current_formation);

    // 释放轨迹
    for (int i = 0; i < ctrl->drone_count; i++) {
        traj_destroy(ctrl->trajectories[i]);
    }

    // 释放安全区域
    safety_zone_destroy(ctrl->safety_zone);

    // 释放无人机
    drone_destroy_fleet(ctrl->fleet, ctrl->drone_count);

    free(ctrl);
}

/* ==================== 主循环 ==================== */

void controller_run(Controller* ctrl)
{
    if (ctrl == NULL) return;

    // 初始化控制台窗口
    graphics_init();

    // 显示欢迎界面
    graphics_show_welcome();

    // 设置初始位置：活跃无人机随机散布（保证最小间距，防止重叠）
    int placed = 0;
    Point2f placed_pos[MAX_DRONE_COUNT];
    for (int i = 0; i < ctrl->drone_count; i++) {
        if (!ctrl->fleet[i]->is_active) {
            drone_set_position(ctrl->fleet[i], -10.0f, -10.0f, 0.0f);
            continue;
        }
        // 尝试在舞台上找一个不与其他飞机重叠的位置（最多试50次）
        int tries = 0;
        float rx = 0, ry = 0;
        int ok = 0;
        while (tries < 50) {
            rx = (float)(rand() % (STAGE_COLS - 6) + 3);
            ry = (float)(rand() % (STAGE_ROWS - 6) + 3);
            ok = 1;
            for (int j = 0; j < placed; j++) {
                if (DISTANCE(rx, ry, placed_pos[j].x, placed_pos[j].y) < 2.0f) {
                    ok = 0; break;  // 太近了，换一个位置
                }
            }
            if (ok) break;
            tries++;
        }
        placed_pos[placed].x = rx;
        placed_pos[placed].y = ry;
        placed++;
        drone_set_position(ctrl->fleet[i], rx, ry, 50.0f);
    }

    // 初始化轨迹：从当前位置 → 圆形编队（仅活跃无人机）
    ctrl_init_default_formation(ctrl);

    // 确保编队内没有完全重合的无人机（微调偏移）
    for (int i = 0; i < ctrl->current_formation->drone_count; i++) {
        for (int j = i + 1; j < ctrl->current_formation->drone_count; j++) {
            float dx = ctrl->current_formation->targets[i].x
                     - ctrl->current_formation->targets[j].x;
            float dy = ctrl->current_formation->targets[i].y
                     - ctrl->current_formation->targets[j].y;
            if (dx * dx + dy * dy < 0.01f) {  // 距离<0.1=重合
                ctrl->current_formation->targets[j].x += 0.5f;
                ctrl->current_formation->targets[j].y += 0.5f;
            }
        }
    }

    traj_from_formation(ctrl->fleet,
                        ctrl->current_formation->drone_count,
                        ctrl->current_formation, ctrl->trajectories,
                        DEFAULT_SPEED, ctrl->selected_color,
                        ctrl->selected_light_mode);

    // ==================== 主循环 ====================
    while (ctrl->is_running) {

        // ── 第1步：处理用户输入 ──
        UICmd cmd = ui_poll_input();
        if (cmd != UI_CMD_NONE) {
            ctrl_handle_command(ctrl, cmd);
        }

        // ── 第2步：模拟更新（仅在 RUNNING 状态执行轨迹和计时） ──
        if (ctrl->sim_state == STATE_RUNNING) {
            int delta = FRAME_INTERVAL_MS;
            ctrl_update_frame(ctrl, delta);
            ctrl->sim_elapsed_ms += delta;
        }

        // ── 安全检测：始终执行（任何状态下都要监控） ──
        safety_check_all(ctrl->fleet, ctrl->drone_count,
                         ctrl->safety_zone, &ctrl->safety_result);

        // 生成警告日志（面板滚动显示）
        ctrl->warn_log_count = 0;
        for (int i = 0; i < ctrl->safety_result.boundary_violations
             && ctrl->warn_log_count < WARN_LOG_SIZE; i++) {
            int id = ctrl->safety_result.boundary_ids[i];
            snprintf(ctrl->warn_log[ctrl->warn_log_count], MAX_WARNING_LEN,
                     "越界: D#%d 超出表演区", id);
            ctrl->warn_log_count++;
        }
        for (int i = 0; i < ctrl->safety_result.distance_violations
             && ctrl->warn_log_count < WARN_LOG_SIZE; i++) {
            int a = ctrl->safety_result.pair_a[i];
            int b = ctrl->safety_result.pair_b[i];
            snprintf(ctrl->warn_log[ctrl->warn_log_count], MAX_WARNING_LEN,
                     "碰撞: D#%d-D#%d 间距过近", a, b);
            ctrl->warn_log_count++;
        }

        // ── 第3步：渲染 ──
        ctrl_render_frame(ctrl, FRAME_INTERVAL_MS);

        // ── 第4步：帧率控制 ──
        Sleep(FRAME_INTERVAL_MS);
    }

    // 关闭控制台窗口
    graphics_close();
}

/* ==================== 命令处理 ==================== */

void ctrl_handle_command(Controller* ctrl, UICmd cmd)
{
    if (ctrl == NULL) return;

    switch (cmd) {
    case UI_CMD_START:
        // 开始模拟
        if (ctrl->sim_state == STATE_IDLE || ctrl->sim_state == STATE_PAUSED) {
            ctrl->sim_state = STATE_RUNNING;
        }
        break;

    case UI_CMD_PAUSE:
        // 暂停 / 继续
        if (ctrl->sim_state == STATE_RUNNING) {
            ctrl->sim_state = STATE_PAUSED;
        } else if (ctrl->sim_state == STATE_PAUSED) {
            ctrl->sim_state = STATE_RUNNING;
        }
        break;

    case UI_CMD_STOP:
        // 停止模拟
        ctrl->sim_state = STATE_IDLE;
        ctrl->sim_elapsed_ms = 0;
        break;

    case UI_CMD_SPEED_UP:
        // 加速
        ctrl->sim_speed += 0.25f;
        if (ctrl->sim_speed > MAX_SPEED / DEFAULT_SPEED) {
            ctrl->sim_speed = MAX_SPEED / DEFAULT_SPEED;
        }
        break;

    case UI_CMD_SPEED_DOWN:
        // 减速
        ctrl->sim_speed -= 0.25f;
        if (ctrl->sim_speed < MIN_SPEED / DEFAULT_SPEED) {
            ctrl->sim_speed = MIN_SPEED / DEFAULT_SPEED;
        }
        break;

    case UI_CMD_NEXT_PATTERN:
        ctrl_switch_pattern(ctrl, 1);
        break;

    case UI_CMD_PREV_PATTERN:
        ctrl_switch_pattern(ctrl, -1);
        break;

    case UI_CMD_CHANGE_COLOR:
        // 循环切换颜色
        ctrl->selected_color = (LightColor)((int)ctrl->selected_color + 1);
        if (ctrl->selected_color > COLOR_ORANGE) {
            ctrl->selected_color = COLOR_RED;
        }
        light_fleet_set_color(ctrl->fleet, ctrl->drone_count,
                              ctrl->selected_color);
        break;

    case UI_CMD_TOGGLE_BLINK:
        // 切换闪烁模式
        if (ctrl->selected_light_mode == LIGHT_STEADY) {
            ctrl->selected_light_mode = LIGHT_BLINK;
            light_fleet_set_mode(ctrl->fleet, ctrl->drone_count, LIGHT_BLINK);
            light_fleet_set_blink(ctrl->fleet, ctrl->drone_count, 500);
        } else {
            ctrl->selected_light_mode = LIGHT_STEADY;
            light_fleet_set_mode(ctrl->fleet, ctrl->drone_count, LIGHT_STEADY);
        }
        break;

    case UI_CMD_SAVE:
        file_save_trajectory(ctrl->fleet, ctrl->drone_count, "drone_show.dat");
        break;

    case UI_CMD_LOAD:
        // TODO: 队友实现文件加载功能
        break;

    case UI_CMD_REPLAY:
        ctrl->sim_state = STATE_REPLAY;
        ctrl->sim_elapsed_ms = 0;
        // TODO: 队友实现回放模式
        break;

    case UI_CMD_TEXT_INPUT:
        {
            // 读取用户输入的文字（≤5字）
            char text[32];
            printf("\n请输入文字(≤5字): ");
            fflush(stdout);
            fgets(text, sizeof(text), stdin);
            text[strcspn(text, "\r\n")] = '\0';
            if (strlen(text) == 0) break;

            // 获取文字推荐的参数（用宽字符数而非字节数）
            int wclen = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
            int char_count = (wclen > 1) ? (wclen - 1) : (int)strlen(text);
            int   opt_count;
            float opt_scale;
            pattern_recommend(PAT_TEXT, char_count, &opt_count, &opt_scale);

            // 销毁旧编队，创建文字编队
            Point2f center = { STAGE_COLS / 2.0f, STAGE_ROWS / 2.0f };
            Formation* new_fm = formation_create(
                text, PAT_TEXT, center, opt_scale, 0.0f,
                opt_count, text
            );

            // 如果创建失败（例如全中文不支持），保留旧编队不变
            if (new_fm == NULL) {
                printf("\n[提示] 仅支持英文和数字，中文暂不支持。按任意键继续...");
                _getch();
                break;
            }

            formation_destroy(ctrl->current_formation);
            ctrl->current_formation = new_fm;
            ctrl->selected_pattern = PAT_TEXT;

            // 记录到历史
            history_add(ctrl);

            // 激活对应数量的无人机
            int use_count = ctrl->current_formation->drone_count;
            for (int i = 0; i < ctrl->drone_count; i++) {
                ctrl->fleet[i]->is_active = (i < use_count) ? 1 : 0;
            }

            // 生成轨迹
            traj_from_formation(ctrl->fleet, use_count,
                                ctrl->current_formation, ctrl->trajectories,
                                DEFAULT_SPEED, ctrl->selected_color,
                                ctrl->selected_light_mode);
        }
        break;

    case UI_CMD_HISTORY:
        {
            // 循环切换到最近的历史编队
            // history[0]是最新的, history[1]是上一次, ...
            static int hist_idx = -1;  // -1=当前编队
            if (ctrl->history_count == 0) break;

            hist_idx = (hist_idx + 1) % (ctrl->history_count + 1);

            // hist_idx==0 表示当前编队（不切换）
            if (hist_idx == 0) {
                // 回到当前编队，不需要切换
                break;
            }

            FormationHistory* h = &ctrl->history[hist_idx - 1];

            // 重建该历史编队
            formation_destroy(ctrl->current_formation);
            int   opt_count;
            float opt_scale;
            pattern_recommend(h->pattern,
                (int)strlen(h->text), &opt_count, &opt_scale);

            Point2f center = { STAGE_COLS / 2.0f, STAGE_ROWS / 2.0f };
            ctrl->current_formation = formation_create(
                "历史编队", h->pattern, center, opt_scale, 0.0f,
                opt_count, h->text
            );
            if (ctrl->current_formation == NULL) break;

            ctrl->selected_pattern = h->pattern;
            ctrl->selected_color   = h->color;
            ctrl->selected_light_mode = h->mode;

            int use_count = ctrl->current_formation->drone_count;
            for (int i = 0; i < ctrl->drone_count; i++) {
                ctrl->fleet[i]->is_active = (i < use_count) ? 1 : 0;
            }

            light_fleet_set_color(ctrl->fleet, ctrl->drone_count, h->color);
            light_fleet_set_mode(ctrl->fleet, ctrl->drone_count, h->mode);

            traj_from_formation(ctrl->fleet, use_count,
                                ctrl->current_formation, ctrl->trajectories,
                                DEFAULT_SPEED, h->color, h->mode);
        }
        break;

    case UI_CMD_EXIT:
        ctrl->is_running = 0;
        break;

    default:
        break;
    }
}

/* ==================== 帧更新 ==================== */

void ctrl_update_frame(Controller* ctrl, int delta_ms)
{
    if (ctrl == NULL) return;

    // 根据速度倍率调整实际时间增量
    int effective_delta = (int)(delta_ms * ctrl->sim_speed);

    // 1) 轨迹更新：所有无人机沿轨迹移动一步
    traj_update_fleet(ctrl->fleet, ctrl->trajectories,
                      ctrl->drone_count, DEFAULT_SPEED, effective_delta);

    // 2) 不主动推开——由安全检测+面板日志负责告警
}

/* ==================== 帧渲染 ==================== */

void ctrl_render_frame(Controller* ctrl, int delta_ms)
{
    if (ctrl == NULL) return;

    // 清屏
    graphics_clear();

    // 顶部标题栏
    graphics_draw_title_bar(ctrl->sim_state, ctrl->sim_elapsed_ms);

    // 绘制表演区域
    graphics_draw_stage(ctrl->safety_zone);

    // 绘制所有无人机（含灯光效果）
    graphics_draw_all_drones(ctrl->fleet, ctrl->drone_count, delta_ms);

    // 绘制安全告警
    int has_warning = 0;
    if (ctrl->safety_result.boundary_violations > 0
     || ctrl->safety_result.distance_violations > 0) {
        graphics_draw_warnings(&ctrl->safety_result);
        has_warning = 1;
    }

    // 绘制右侧信息面板
    int active_count = (ctrl->current_formation != NULL)
        ? ctrl->current_formation->drone_count : 0;
    graphics_draw_panel(ctrl->fleet, active_count,
                        ctrl->sim_state, ctrl->current_formation,
                        ctrl->sim_elapsed_ms,
                        ctrl->sim_speed,
                        ctrl->selected_color,
                        ctrl->selected_light_mode,
                        has_warning);

    // 面板警告日志（右侧面板下方6行滚动）
    graphics_draw_warn_panel(ctrl->warn_log, ctrl->warn_log_count);

    // 底部状态栏
    graphics_draw_bottom_bar(ctrl->drone_count, active_count,
                             "按 H 查看历史 | 按 T 输入文字");

    // 刷新到屏幕
    graphics_flush();
}

/* ==================== 图案切换 ==================== */

/**
 * @brief 记录当前编队到历史
 */
static void history_add(Controller* ctrl)
{
    if (ctrl == NULL || ctrl->current_formation == NULL) return;

    // 移动旧记录
    for (int i = HISTORY_SIZE - 1; i > 0; i--) {
        ctrl->history[i] = ctrl->history[i - 1];
    }

    // 保存当前编队
    ctrl->history[0].pattern = ctrl->current_formation->pattern;
    strncpy(ctrl->history[0].text, ctrl->current_formation->display_text,
            sizeof(ctrl->history[0].text) - 1);
    ctrl->history[0].text[sizeof(ctrl->history[0].text) - 1] = '\0';
    ctrl->history[0].color = ctrl->selected_color;
    ctrl->history[0].mode  = ctrl->selected_light_mode;

    if (ctrl->history_count < HISTORY_SIZE) ctrl->history_count++;
}

void ctrl_switch_pattern(Controller* ctrl, int direction)
{
    if (ctrl == NULL) return;

    // 记录当前编队到历史
    history_add(ctrl);

    // 更新图案索引（循环）
    ctrl->pattern_index += direction;
    if (ctrl->pattern_index < 0) {
        ctrl->pattern_index = PATTERN_COUNT - 1;
    }
    if (ctrl->pattern_index >= (int)PATTERN_COUNT) {
        ctrl->pattern_index = 0;
    }

    ctrl->selected_pattern = g_pattern_list[ctrl->pattern_index];

    // 获取该图案的推荐参数
    int    opt_count;
    float  opt_scale;
    const char* txt_param = NULL;
    if (ctrl->selected_pattern == PAT_TEXT) {
        txt_param = "HI";  // 文字编队的默认文字
    }
    pattern_recommend(ctrl->selected_pattern,
        txt_param ? (int)strlen(txt_param) : 3, &opt_count, &opt_scale);

    // 销毁旧编队，创建新编队（使用推荐参数）
    formation_destroy(ctrl->current_formation);

    Point2f center = { STAGE_COLS / 2.0f, STAGE_ROWS / 2.0f };
    ctrl->current_formation = formation_create(
        "当前编队", ctrl->selected_pattern, center, opt_scale, 0.0f,
        opt_count, txt_param
    );

    // 更新活跃无人机数量：formation 说用多少架，就激活多少架
    int use_count = ctrl->current_formation->drone_count;
    for (int i = 0; i < ctrl->drone_count; i++) {
        ctrl->fleet[i]->is_active = (i < use_count) ? 1 : 0;
    }

    // 生成轨迹（仅为活跃无人机生成）
    traj_from_formation(ctrl->fleet, use_count,
                        ctrl->current_formation, ctrl->trajectories,
                        DEFAULT_SPEED, ctrl->selected_color,
                        ctrl->selected_light_mode);
}

/* ==================== 默认编队 ==================== */

void ctrl_init_default_formation(Controller* ctrl)
{
    if (ctrl == NULL) return;

    // 获取圆形的推荐参数
    int   opt_count;
    float opt_scale;
    pattern_recommend(PAT_CIRCLE, 0, &opt_count, &opt_scale);

    Point2f center = { STAGE_COLS / 2.0f, STAGE_ROWS / 2.0f };
    ctrl->current_formation = formation_create(
        "圆形编队", PAT_CIRCLE, center, opt_scale, 0.0f,
        opt_count, NULL
    );
    ctrl->selected_pattern = PAT_CIRCLE;
    ctrl->pattern_index    = 0;

    // 记录初始编队到历史
    history_add(ctrl);

    // 激活对应数量的无人机，其余休眠
    int use_count = ctrl->current_formation->drone_count;
    for (int i = 0; i < ctrl->drone_count; i++) {
        ctrl->fleet[i]->is_active = (i < use_count) ? 1 : 0;
    }
}
