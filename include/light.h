/**
 * @file    light.h
 * @brief   灯光控制模块 —— 编队级灯光效果控制
 * @author  [你的名字]
 * @date    2026-06-08
 *
 * 本模块提供编队级的灯光效果控制功能，包括：
 * - 统一设定全队灯光颜色
 * - 波浪式灯光效果（逐架延迟亮灯）
 * - 交替闪烁模式
 * - 流水灯循环效果
 * - 渐变色插值
 *
 * 模块内部不持有状态，所有操作通过传入参数控制。
 */

#ifndef LIGHT_H
#define LIGHT_H

#include "common.h"
#include "drone.h"

/* ==================== 全队统一操作 ==================== */

/**
 * @brief 统一设定无人机编队的灯光颜色
 * @param fleet   无人机指针数组
 * @param count   无人机数量
 * @param color   统一设置的颜色
 */
void light_fleet_set_color(Drone* fleet[], int count, LightColor color);

/**
 * @brief 统一设定无人机编队的灯光模式
 * @param fleet   无人机指针数组
 * @param count   无人机数量
 * @param mode    统一设置的灯光模式
 */
void light_fleet_set_mode(Drone* fleet[], int count, LightMode mode);

/**
 * @brief 统一设定无人机编队的闪烁间隔
 * @param fleet       无人机指针数组
 * @param count       无人机数量
 * @param interval_ms 闪烁间隔（毫秒）
 */
void light_fleet_set_blink(Drone* fleet[], int count, int interval_ms);

/* ==================== 动态效果（持续调用） ==================== */

/**
 * @brief 波浪灯光效果
 *
 * 无人机按索引依次亮起同一种颜色，相邻无人机之间的延迟为 delay_ms。
 * 类似多米诺骨牌逐个点亮的效果。
 *
 * @param fleet      无人机指针数组
 * @param count      无人机数量
 * @param color      灯光颜色
 * @param delay_ms   相邻无人机之间的点亮延迟（毫秒）
 * @param elapsed_ms 已运行的总时间（毫秒），由调用者在每次调用时累加
 * @param reset      1=重置效果，从第一架重新开始
 */
void light_wave_effect(Drone* fleet[], int count, LightColor color,
                       int delay_ms, int* elapsed_ms, int reset);

/**
 * @brief 交替闪烁效果
 *
 * 奇数编号与偶数编号的无人机交替亮灭，产生"闪烁交替"的视觉效果。
 *
 * @param fleet      无人机指针数组
 * @param count      无人机数量
 * @param color_a    奇数编号无人机的颜色
 * @param color_b    偶数编号无人机的颜色
 * @param phase      当前相位：0=A亮B灭, 1=A灭B亮
 * @param interval_ms 相位切换间隔
 * @param timer_ms   计时器指针，达到间隔则切换相位
 */
void light_alternate(Drone* fleet[], int count,
                     LightColor color_a, LightColor color_b,
                     int* phase, int interval_ms, int* timer_ms);

/**
 * @brief 流水灯循环效果
 *
 * 只有一串连续的 k 架无人机亮灯，这串"亮灯窗口"在编队中循环滑动。
 *
 * @param fleet      无人机指针数组
 * @param count      无人机数量
 * @param color      亮灯颜色
 * @param window_size 同时亮灯的无人机数量
 * @param offset     当前亮灯窗口的起始索引（由调用者递增）
 * @param speed_ms   窗口滑动速度（每移动一个位置需要的毫秒数）
 * @param timer_ms   计时器指针
 */
void light_flow(Drone* fleet[], int count, LightColor color,
                int window_size, int* offset, int speed_ms, int* timer_ms);

/* ==================== 颜色工具 ==================== */

/**
 * @brief 颜色渐变插值
 *
 * 在两个颜色之间按比例计算中间色，用于平缓的颜色过渡效果。
 *
 * @param color_from 起始颜色
 * @param color_to   目标颜色
 * @param t          插值参数 [0.0, 1.0]：0=from颜色, 1=to颜色
 * @return 插值后的控制台颜色属性（ConsoleColor）
 */
ConsoleColor light_color_lerp(LightColor color_from, LightColor color_to, float t);

/**
 * @brief 根据颜色枚举获取对应的中文字符串
 * @param color 颜色枚举
 * @return 中文字符串
 */
const char* color_to_name(LightColor color);

#endif // LIGHT_H
