/**
 * @file    light.cpp
 * @brief   灯光控制模块 —— 编队级灯光效果实现
 * @author  [你的名字]
 * @date    2026-06-08
 */

#include "../include/light.h"

/* ==================== 全队统一操作 ==================== */

void light_fleet_set_color(Drone* fleet[], int count, LightColor color)
{
    if (fleet == NULL || count <= 0) return;

    for (int i = 0; i < count; i++) {
        if (fleet[i] != NULL) {
            drone_set_light_color(fleet[i], color);
        }
    }
}

void light_fleet_set_mode(Drone* fleet[], int count, LightMode mode)
{
    if (fleet == NULL || count <= 0) return;

    for (int i = 0; i < count; i++) {
        if (fleet[i] != NULL) {
            drone_set_light_mode(fleet[i], mode);
        }
    }
}

void light_fleet_set_blink(Drone* fleet[], int count, int interval_ms)
{
    if (fleet == NULL || count <= 0) return;

    for (int i = 0; i < count; i++) {
        if (fleet[i] != NULL) {
            drone_set_blink_interval(fleet[i], interval_ms);
        }
    }
}

/* ==================== 动态效果 ==================== */

void light_wave_effect(Drone* fleet[], int count, LightColor color,
                       int delay_ms, int* elapsed_ms, int reset)
{
    if (fleet == NULL || count <= 0 || elapsed_ms == NULL) return;

    // 重置：从第一架重新开始
    if (reset) {
        *elapsed_ms = 0;
        // 全部熄灭
        light_fleet_set_mode(fleet, count, LIGHT_OFF);
        return;
    }

    // 计算当前该亮到第几架
    int lit_count = *elapsed_ms / delay_ms;

    for (int i = 0; i < count; i++) {
        if (fleet[i] == NULL) continue;

        if (i < lit_count) {
            // 已点亮的无人机：保持常亮
            drone_set_light_color(fleet[i], color);
            drone_set_light_mode(fleet[i], LIGHT_STEADY);
        } else {
            // 尚未点亮的无人机：关闭
            drone_set_light_mode(fleet[i], LIGHT_OFF);
        }
    }
}

void light_alternate(Drone* fleet[], int count,
                     LightColor color_a, LightColor color_b,
                     int* phase, int interval_ms, int* timer_ms)
{
    if (fleet == NULL || count <= 0 || phase == NULL || timer_ms == NULL) return;

    // 累加计时器，检查是否需要切换相位
    *timer_ms += FRAME_INTERVAL_MS;
    if (*timer_ms >= interval_ms) {
        *phase = !(*phase);     // 相位翻转
        *timer_ms = 0;
    }

    for (int i = 0; i < count; i++) {
        if (fleet[i] == NULL) continue;

        if (*phase == 0) {
            // 相位0：奇数号亮 color_a，偶数号亮 color_b
            drone_set_light_color(fleet[i],
                (fleet[i]->id % 2 == 1) ? color_a : color_b);
        } else {
            // 相位1：奇数号亮 color_b，偶数号亮 color_a
            drone_set_light_color(fleet[i],
                (fleet[i]->id % 2 == 1) ? color_b : color_a);
        }
        drone_set_light_mode(fleet[i], LIGHT_STEADY);
    }
}

void light_flow(Drone* fleet[], int count, LightColor color,
                int window_size, int* offset, int speed_ms, int* timer_ms)
{
    if (fleet == NULL || count <= 0 || offset == NULL || timer_ms == NULL) return;
    if (window_size >= count) window_size = count - 1;
    if (window_size <= 0) window_size = 1;

    // 累加计时器，达到速度间隔则窗口滑动一格
    *timer_ms += FRAME_INTERVAL_MS;
    if (*timer_ms >= speed_ms) {
        *offset = (*offset + 1) % count;
        *timer_ms = 0;
    }

    // 根据当前窗口设定每架无人机的亮/灭
    for (int i = 0; i < count; i++) {
        if (fleet[i] == NULL) continue;

        // 环形判断：i 是否在 [offset, offset+window_size) 之间
        int in_window = 0;
        for (int w = 0; w < window_size; w++) {
            if (i == ((*offset + w) % count)) {
                in_window = 1;
                break;
            }
        }

        if (in_window) {
            drone_set_light_color(fleet[i], color);
            drone_set_light_mode(fleet[i], LIGHT_STEADY);
        } else {
            drone_set_light_mode(fleet[i], LIGHT_OFF);
        }
    }
}

/* ==================== 颜色工具 ==================== */

ConsoleColor light_color_lerp(LightColor color_from, LightColor color_to, float t)
{
    // 限制 t 在 [0, 1] 范围
    t = CLAMP(t, 0.0f, 1.0f);

    // t < 0.5 → 使用起始颜色；t >= 0.5 → 使用目标颜色
    // 控制台16色无法做平滑RGB渐变，故用阈值切换
    return (t < 0.5f) ? color_from : color_to;
}
