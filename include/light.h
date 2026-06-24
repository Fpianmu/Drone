/*
 * light.h - 灯光控制模块
 *
 * 提供编队级别的灯光效果。单架飞机的灯光操作在 drone.h 中。
 *
 * 动态效果包括：
 *   - 波浪点亮：飞机按编号依次亮起，像多米诺骨牌
 *   - 交替闪烁：奇数号和偶数号交替亮灭
 *   - 流水灯：一段亮灯窗口在编队中循环滑动
 *   - 颜色渐变插值：两个颜色之间的过渡
 */

#ifndef LIGHT_H
#define LIGHT_H

#include "common.h"
#include "drone.h"

/* ---- 全队统一操作（都是遍历编队然后调用 drone 的函数） ---- */

// 把整个编队的灯光设成同一种颜色。
void light_fleet_set_color(Drone* fleet[], int count, LightColor color);

// 把整个编队的灯光设成同一种模式（常亮/闪烁/关闭）。
void light_fleet_set_mode(Drone* fleet[], int count, LightMode mode);

// 把整个编队的闪烁间隔设成同一个值。
void light_fleet_set_blink(Drone* fleet[], int count, int interval_ms);

/* ---- 动态效果（持续调用，内部有计时逻辑） ---- */

/*
 * 波浪效果
 *
 * delay_ms 是相邻两架之间的点亮延迟。
 * elapsed_ms 是外部计时器（在 controller 里），由调用者管理。
 * reset=1 时重置：全部熄灭，从头开始。
 */
void light_wave_effect(Drone* fleet[], int count, LightColor color,
                       int delay_ms, int* elapsed_ms, int reset);

/*
 * 交替闪烁
 *
 * phase 表示当前相位（0 = A亮B灭, 1 = A灭B亮）。
 * 每到 interval_ms 就翻转相位。
 */
void light_alternate(Drone* fleet[], int count,
                     LightColor color_a, LightColor color_b,
                     int* phase, int interval_ms, int* timer_ms);

/*
 * 流水灯
 *
 * window_size 架连续飞机亮灯，这截"亮灯窗口"在编队中循环滑动。
 * offset 是当前窗口的起始位置。
 * speed_ms 是窗口每滑一格需要的毫秒数。
 */
void light_flow(Drone* fleet[], int count, LightColor color,
                int window_size, int* offset, int speed_ms, int* timer_ms);

/* ---- 颜色工具 ---- */

// 两个颜色之间的插值（因为控制台只有16色，所以 t<0.5 用 from 色，t>=0.5 用 to 色）。
ConsoleColor light_color_lerp(LightColor color_from, LightColor color_to, float t);

// 颜色枚举转中文名字（"红色" / "蓝色" 等），给 UI 显示用。
const char* color_to_name(LightColor color);

#endif
