/*
 * safety.h - 安全检测模块
 *
 * 每帧自动运行的安全检测，包括两部分：
 *   1. 边界检测：查无人机有没有飞出表演区（越界）
 *   2. 间距检测：算两两之间的距离，找出太近的无人机对
 *
 * 检测结果存在 SafetyResult 里，给 graphics 模块画警告用。
 */

#ifndef SAFETY_H
#define SAFETY_H

#include "common.h"

/* ---- 安全区域管理 ---- */

SafetyZone* safety_zone_create(int x_min, int y_min, int x_max, int y_max,
                               int min_distance);
void        safety_zone_destroy(SafetyZone* zone);
int         safety_point_in_zone(const SafetyZone* zone, float x, float y);

/* ---- 检测函数（每帧调用） ---- */

// 边界检测：遍历活跃无人机，检查位置是否在安全区内。返回越界数量。
int safety_check_boundary(Drone* fleet[], int count,
                          const SafetyZone* zone, SafetyResult* result);

// 间距检测：两两计算欧氏距离，找出距离 < 最小间距的飞机对。返回违规对数。
int safety_check_distance(Drone* fleet[], int count,
                          const SafetyZone* zone, SafetyResult* result);

// 综合检测 = 边界检测 + 间距检测。
int safety_check_all(Drone* fleet[], int count,
                     const SafetyZone* zone, SafetyResult* result);

void safety_result_clear(SafetyResult* result);

/*
 * 碰撞自动避让。
 *
 * 仅当两架飞机距离 < 0.5 格时推开一点点（力度由 strength 控制，
 * 0 到 1 之间）。力度越小影响越小。
 * 注意：文字编队里无人机本身就靠得很近，避让可能会把字形推散。
 */
void safety_avoid_collisions(Drone* fleet[], int count,
                             const SafetyZone* zone, float strength);

#endif
