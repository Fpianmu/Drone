/*
 * safety.h —— 安全检测模块
 *
 * 每帧自动运行的安全检测，包含两部分：
 *   1. 边界检测：检查无人机是否飞出表演区（越界）
 *   2. 间距检测：两两计算距离，找出太近的无人机对（碰撞风险）
 *
 * 检测结果存在 SafetyResult 里，供 graphics 层显示告警。
 */

#ifndef SAFETY_H
#define SAFETY_H

#include "common.h"

/* ---- 安全区域 ---- */

SafetyZone* safety_zone_create(int x_min, int y_min, int x_max, int y_max,
                               int min_distance);
void        safety_zone_destroy(SafetyZone* zone);
int         safety_point_in_zone(const SafetyZone* zone, float x, float y);

/* ---- 安全检测（每帧调用）---- */

// 边界检测 —— 返回越界的无人机数量
int safety_check_boundary(Drone* fleet[], int count,
                          const SafetyZone* zone, SafetyResult* result);

// 间距检测 —— 返回距离过近的无人机对数量
int safety_check_distance(Drone* fleet[], int count,
                          const SafetyZone* zone, SafetyResult* result);

// 综合检测 = 边界 + 间距
int safety_check_all(Drone* fleet[], int count,
                     const SafetyZone* zone, SafetyResult* result);

void safety_result_clear(SafetyResult* result);

/*
 * 碰撞自动避让 —— 对距离太近的无人机施加排斥力，自动推开
 *
 * 只在距离 < 安全间距的一半时触发，力度由 strength 控制（0~1）。
 * 注意：文字编队中无人机本身就靠得很近，避让可能破坏字形。
 * 目前该功能未在主循环中默认启用，可按需调用。
 */
void safety_avoid_collisions(Drone* fleet[], int count,
                             const SafetyZone* zone, float strength);

#endif
