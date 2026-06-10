/*
 * graphics.h —— 控制台渲染模块
 *
 * 所有绘制操作都先在内存帧缓冲（CHAR_INFO 二维数组）中完成，
 * 最后一帧调用 graphics_flush 用 WriteConsoleOutputW 一次性写入控制台。
 * 这样可以彻底消除闪烁。
 *
 * 画面布局：
 *   左侧 80×40 表演区（有框线和网格点）
 *   右侧信息面板（状态、编队、操作键提示）
 *   底部 2 行安全警告
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common.h"
#include "drone.h"

/* ---- 窗口管理 ---- */

void graphics_init(void);    // 初始化控制台（尺寸、标题、隐藏光标）
void graphics_close(void);   // 恢复控制台并清理
void graphics_clear(void);   // 清空帧缓冲（每帧开始时调用）
void graphics_flush(void);   // 将帧缓冲写入控制台（每帧结束时调用）

/* ---- 绘制元素 ---- */

void graphics_draw_stage(const SafetyZone* zone);          // 表演区边框 + 网格
void graphics_draw_drone(const Drone* drone, ConsoleColor color); // 单架飞机（●）
void graphics_draw_all_drones(Drone* fleet[], int count, int delta_ms); // 所有飞机

/* ---- 覆盖层 ---- */

void graphics_draw_warnings(const SafetyResult* result);   // 安全告警（红/黄文字）
void graphics_draw_panel(Drone* fleet[], int count, SimState state,
                         const Formation* formation, int elapsed_ms); // 信息面板

/* ---- 特殊界面 ---- */

void graphics_show_welcome(void);   // 启动欢迎界面（阻塞等待按键）

/* ---- 底层工具 ---- */

void graphics_gotoxy(int col, int row);     // 定位
void graphics_set_color(ConsoleColor color); // 设颜色
void graphics_put_str(int col, int row, ConsoleColor color, const char* str); // 写字符串

#endif
