/*
 * graphics.h - 控制台渲染模块
 *
 * 所有画面先在内存帧缓冲（CHAR_INFO 二维数组）里画好，
 * 每帧结束时用 WriteConsoleOutputW 一次性写到控制台，消除闪烁。
 *
 * 画面布局：
 *   顶部标题栏
 *   左侧 80x36 表演区（双线框 + 淡色网格）
 *   右侧信息面板
 *   表演区下方安全警告
 *   底部状态栏
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common.h"
#include "drone.h"

/* ---- 窗口管理 ---- */

void graphics_init(void);    // 初始化控制台（设尺寸、标题、隐藏光标）
void graphics_close(void);   // 恢复控制台
void graphics_clear(void);   // 清空帧缓冲（每帧开始时调用）
void graphics_flush(void);   // 把帧缓冲写到控制台（每帧结束时调用）

/* ---- 绘制元素 ---- */

void graphics_draw_stage(const SafetyZone* zone);  // 表演区边框 + 网格
void graphics_draw_drone(const Drone* drone, ConsoleColor color); // 一架飞机（●）
void graphics_draw_all_drones(Drone* fleet[], int count, int delta_ms); // 全部飞机

/* ---- 覆盖层 ---- */

void graphics_draw_warnings(const SafetyResult* result);  // 底部安全告警

// 右侧信息面板（状态、参数、编队、安全、操作键）
void graphics_draw_panel(Drone* fleet[], int count, SimState state,
                         const Formation* formation, int elapsed_ms,
                         float sim_speed, LightColor light_color,
                         LightMode light_mode, int has_warning,
                         LightFX light_fx);

/* ---- 特殊界面 ---- */

void graphics_show_welcome(void);  // 启动欢迎界面（阻塞等待按键）
void graphics_draw_title_bar(SimState state, int elapsed_ms); // 顶部标题栏
void graphics_draw_bottom_bar(int drone_count, int active_count,
                              const char* hint_text); // 底部状态栏
void graphics_draw_warn_panel(char warn_log[][MAX_WARNING_LEN],
                              int warn_count); // 面板警告日志

/* ---- 底层工具 ---- */

void graphics_gotoxy(int col, int row);
void graphics_set_color(ConsoleColor color);
void graphics_put_str(int col, int row, ConsoleColor color, const char* str);

#endif
