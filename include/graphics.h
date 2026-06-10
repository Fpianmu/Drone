/**
 * @file    graphics.h
 * @brief   图形渲染模块 —— Windows 控制台字符画渲染
 * @author  [队友名字]
 * @date    2026-06-08
 *
 * 本模块使用 Windows Console API 实现：
 * - 控制台窗口初始化（设置尺寸、标题、隐藏光标）
 * - 绘制无人机（彩色字符 ●）
 * - 绘制表演区域（框线 + 背景）
 * - 绘制安全告警信息
 * - 绘制右侧信息面板
 * - 双缓冲刷新技术（避免闪烁）
 *
 * 坐标映射：
 *   表演区逻辑坐标 (0~STAGE_COLS, 0~STAGE_ROWS)
 *   控制台字符坐标  (STAGE_LEFT + x, STAGE_TOP + y)
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "common.h"
#include "drone.h"

/* ==================== 窗口管理 ==================== */

/**
 * @brief 初始化控制台窗口
 *
 * 设置控制台缓冲区大小、窗口标题、字体、隐藏光标。
 * 程序启动时调用一次。
 */
void graphics_init(void);

/**
 * @brief 恢复控制台设置并关闭
 *        恢复光标显示、还原颜色等。
 */
void graphics_close(void);

/**
 * @brief 清空画面（重置屏幕缓冲区光标到起始位置）
 *        每帧开始时调用。
 */
void graphics_clear(void);

/**
 * @brief 将缓冲区内容写入控制台（定位光标到起始位）
 *        每帧结束时调用。
 */
void graphics_flush(void);

/* ==================== 绘制元素 ==================== */

/**
 * @brief 绘制表演区域边框和背景
 *        - 用 ┌─┐│└┘ 等框线字符画边框
 *        - 内部区域为空白网格
 * @param zone 表演区域边界配置
 */
void graphics_draw_stage(const SafetyZone* zone);

/**
 * @brief 绘制单架无人机（彩色圆点 ●）
 * @param drone  无人机指针
 * @param color  当前应显示的控制台颜色属性
 */
void graphics_draw_drone(const Drone* drone, ConsoleColor color);

/**
 * @brief 批量绘制所有无人机
 *
 * 遍历编队，每帧调用 drone_get_current_color 获取闪烁后的颜色，
 * 然后绘制到控制台。
 *
 * @param fleet     无人机编队
 * @param count     无人机数量
 * @param delta_ms  帧间隔（ms），传给 drone_get_current_color 驱动闪烁
 */
void graphics_draw_all_drones(Drone* fleet[], int count, int delta_ms);

/* ==================== 覆盖层 ==================== */

/**
 * @brief 在底部绘制安全区域告警信息
 * @param result 安全检测结果
 */
void graphics_draw_warnings(const SafetyResult* result);

/**
 * @brief 绘制右侧信息面板
 *        - 模拟状态、运行时间、速度
 *        - 无人机数量、当前编队、灯光颜色/模式
 *        - 按键提示、安全状态
 * @param fleet      无人机编队
 * @param count      无人机数量
 * @param state      当前模拟状态
 * @param formation  当前编队（可为NULL）
 * @param elapsed_ms 已运行时间（ms）
 * @param sim_speed  当前模拟速度倍率
 * @param light_color 当前灯光颜色
 * @param light_mode  当前灯光模式
 * @param has_warning 是否有安全警告
 */
void graphics_draw_panel(Drone* fleet[], int count, SimState state,
                         const Formation* formation, int elapsed_ms,
                         float sim_speed, LightColor light_color,
                         LightMode light_mode, int has_warning);

/** @brief 显示启动欢迎界面（阻塞等待按键） */
void graphics_show_welcome(void);

/** @brief 绘制顶部标题栏 */
void graphics_draw_title_bar(SimState state, int elapsed_ms);

/** @brief 绘制底部状态栏 */
void graphics_draw_bottom_bar(int drone_count, int active_count,
                              const char* hint_text);

/* ==================== 工具函数 ==================== */

/**
 * @brief 设置控制台光标位置
 * @param col 列（X）
 * @param row 行（Y）
 */
void graphics_gotoxy(int col, int row);

/**
 * @brief 设置控制台当前文本颜色
 * @param color 控制台颜色属性
 */
void graphics_set_color(ConsoleColor color);

/**
 * @brief 在指定位置绘制彩色字符串
 * @param col   列
 * @param row   行
 * @param color 颜色
 * @param str   字符串（UTF-8）
 */
void graphics_put_str(int col, int row, ConsoleColor color, const char* str);

#endif // GRAPHICS_H
