/*
 * light.h —— 灯光控制模块
 *
 * 提供编队级别的灯光控制效果。单架飞机的灯光操作在 drone.h 中。
 * 本模块的"全队统一操作"直接遍历每架飞机调用 drone 模块的接口。
 *
 * 动态效果包括：
 *   - 波浪点亮：飞机按编号依次亮起，像多米诺骨牌
 *   - 交替闪烁：奇数号和偶数号交替亮灭
 *   - 流水灯：一截"亮灯窗口"在编队中循环滑动
 *   - 颜色渐变插值：两个颜色之间按比例取中间色
 */

#ifndef LIGHT_H
#define LIGHT_H

#include "common.h"
#include "drone.h"

/* ---- 全队统一操作 ---- */

// 把整个编队的灯光设成同一种颜色。
void light_fleet_set_color(Drone* fleet[], int count, LightColor color);

// 把整个编队的灯光设成同一种模式（常亮/闪烁/关闭）。
void light_fleet_set_mode(Drone* fleet[], int count, LightMode mode);

// 把整个编队的闪烁间隔设成同一个值。
void light_fleet_set_blink(Drone* fleet[], int count, int interval_ms);

/* ---- 动态效果（持续调用，内部有计时逻辑） ---- */

/*
 * 波浪效果 —— 飞机按索引顺序依次亮起
 *
 * delay_ms 是相邻两架之间的点亮延迟。
 * elapsed_ms 指向一个外部计时器（在 controller 里），调用者负责管理。
 * reset=1 时重置效果（全部熄灭，从头开始）。
 */
void light_wave_effect(Drone* fleet[], int count, LightColor color,
                       int delay_ms, int* elapsed_ms, int reset);

/*
 * 交替闪烁 —— 奇数号亮 color_a，偶数号亮 color_b，然后翻转
 *
 * phase 指向当前相位（0=A亮B灭, 1=A灭B亮）。
 * timer_ms 指向外部计时器，达到 interval_ms 后翻转相位。
 */
void light_alternate(Drone* fleet[], int count,
                     LightColor color_a, LightColor color_b,
                     int* phase, int interval_ms, int* timer_ms);

/*
 * 流水灯 —— 一截 window_size 架飞机亮灯，在编队中循环滑动
 *
 * offset 指向当前亮灯窗口的起始索引，由本函数自动递增。
 * speed_ms 是窗口每移动一个位置需要的毫秒数。
 */
void light_flow(Drone* fleet[], int count, LightColor color,
                int window_size, int* offset, int speed_ms, int* timer_ms);

/* ---- 颜色工具 ---- */

// 两个颜色之间的线性插值（t ∈ [0.0, 1.0]，0=from, 1=to）
ConsoleColor light_color_lerp(LightColor color_from, LightColor color_to, float t);

// 颜色枚举 → 中文字符串（"红色" / "蓝色" 等）
const char* color_to_name(LightColor color);

#endif
