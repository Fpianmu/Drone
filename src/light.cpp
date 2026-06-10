/*
 * light.cpp —— 灯光控制模块实现
 *
 * 所有函数都通过遍历编队并调用 drone 模块的接口来实现。
 * 动态效果的计时器由调用者（controller）提供，不在本模块内部持有状态。
 */

#include "../include/light.h"

/* ---- 全队统一操作 ---- */

void light_fleet_set_color(Drone* fleet[], int count, LightColor color)
{
    if (fleet == NULL || count <= 0) return;
    for (int i = 0; i < count; i++) {
        if (fleet[i] != NULL) drone_set_light_color(fleet[i], color);
    }
}

void light_fleet_set_mode(Drone* fleet[], int count, LightMode mode)
{
    if (fleet == NULL || count <= 0) return;
    for (int i = 0; i < count; i++) {
        if (fleet[i] != NULL) drone_set_light_mode(fleet[i], mode);
    }
}

void light_fleet_set_blink(Drone* fleet[], int count, int interval_ms)
{
    if (fleet == NULL || count <= 0) return;
    for (int i = 0; i < count; i++) {
        if (fleet[i] != NULL) drone_set_blink_interval(fleet[i], interval_ms);
    }
}

/* ---- 动态效果 ---- */

/*
 * 波浪点亮：第 i 架飞机在 i * delay_ms 时刻亮起。
 * elapsed_ms 累计运行时间，lit_count = elapsed_ms / delay_ms 决定亮了到第几架。
 */
void light_wave_effect(Drone* fleet[], int count, LightColor color,
                       int delay_ms, int* elapsed_ms, int reset)
{
    if (fleet == NULL || count <= 0 || elapsed_ms == NULL) return;

    if (reset) {
        *elapsed_ms = 0;
        light_fleet_set_mode(fleet, count, LIGHT_OFF);
        return;
    }

    int lit_count = *elapsed_ms / delay_ms;
    for (int i = 0; i < count; i++) {
        if (fleet[i] == NULL) continue;
        if (i < lit_count) {
            drone_set_light_color(fleet[i], color);
            drone_set_light_mode(fleet[i], LIGHT_STEADY);
        } else {
            drone_set_light_mode(fleet[i], LIGHT_OFF);
        }
    }
}

/*
 * 交替闪烁：phase=0 时奇数亮 A 偶数亮 B，phase=1 时翻转。
 * 计时器每到 interval_ms 翻转一次相位。
 */
void light_alternate(Drone* fleet[], int count,
                     LightColor color_a, LightColor color_b,
                     int* phase, int interval_ms, int* timer_ms)
{
    if (fleet == NULL || count <= 0 || phase == NULL || timer_ms == NULL) return;

    *timer_ms += FRAME_INTERVAL_MS;
    if (*timer_ms >= interval_ms) {
        *phase = !(*phase);
        *timer_ms = 0;
    }

    for (int i = 0; i < count; i++) {
        if (fleet[i] == NULL) continue;
        drone_set_light_color(fleet[i],
            (fleet[i]->id % 2 == (*phase == 0 ? 1 : 0)) ? color_a : color_b);
    }
}

/*
 * 流水灯：连续 window_size 架飞机亮灯，窗口在编队中循环。
 * offset 指向窗口起始索引，每 speed_ms 毫秒向前滑动一格。
 *
 * 环形判断：对每架飞机 i，检查 i 是否落在 [offset, offset+window_size) 内
 *（对 count 取模处理环形）。
 */
void light_flow(Drone* fleet[], int count, LightColor color,
                int window_size, int* offset, int speed_ms, int* timer_ms)
{
    if (fleet == NULL || count <= 0 || offset == NULL || timer_ms == NULL) return;
    if (window_size >= count) window_size = count - 1;
    if (window_size <= 0)      window_size = 1;

    *timer_ms += FRAME_INTERVAL_MS;
    if (*timer_ms >= speed_ms) {
        *offset = (*offset + 1) % count;
        *timer_ms = 0;
    }

    for (int i = 0; i < count; i++) {
        if (fleet[i] == NULL) continue;

        // 环形判断 i 是否在窗口内
        int in_win = 0;
        for (int w = 0; w < window_size; w++) {
            if (i == ((*offset + w) % count)) { in_win = 1; break; }
        }

        if (in_win) {
            drone_set_light_color(fleet[i], color);
            drone_set_light_mode(fleet[i], LIGHT_STEADY);
        } else {
            drone_set_light_mode(fleet[i], LIGHT_OFF);
        }
    }
}

/* ---- 颜色工具 ---- */

/*
 * 颜色渐变插值
 *
 * 控制台只有 16 色，做不出真正的 RGB 平滑渐变，
 * 所以用阈值切换：t < 0.5 用起始色，t >= 0.5 用目标色。
 */
ConsoleColor light_color_lerp(LightColor color_from, LightColor color_to, float t)
{
    t = CLAMP(t, 0.0f, 1.0f);
    return (t < 0.5f) ? color_from : color_to;
}
