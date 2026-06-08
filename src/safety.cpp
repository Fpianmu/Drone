/**
 * @file    safety.cpp
 * @brief   安全检测模块实现
 * @author  [你的名字]
 * @date    2026-06-08
 *
 * 检测算法：
 *   边界检测 —— 单向比较，O(n)
 *   间距检测 —— 两两比较，O(n²)，跳过自身和无序重复对
 */

#include "../include/safety.h"

/* ==================== 安全区域管理 ==================== */

SafetyZone* safety_zone_create(int x_min, int y_min, int x_max, int y_max,
                               int min_distance)
{
    SafetyZone* zone = (SafetyZone*)malloc(sizeof(SafetyZone));
    if (zone == NULL) return NULL;

    zone->x_min       = x_min;
    zone->y_min       = y_min;
    zone->x_max       = x_max;
    zone->y_max       = y_max;
    zone->min_distance = min_distance;

    return zone;
}

void safety_zone_destroy(SafetyZone* zone)
{
    if (zone != NULL) {
        free(zone);
    }
}

int safety_point_in_zone(const SafetyZone* zone, float x, float y)
{
    if (zone == NULL) return 1;  // 无边界 = 始终安全

    return (x >= zone->x_min && x <= zone->x_max
         && y >= zone->y_min && y <= zone->y_max) ? 1 : 0;
}

/* ==================== 边界检测 ==================== */

int safety_check_boundary(Drone* fleet[], int count,
                          const SafetyZone* zone, SafetyResult* result)
{
    if (fleet == NULL || zone == NULL || result == NULL) return 0;

    result->boundary_violations = 0;

    for (int i = 0; i < count; i++) {
        if (fleet[i] == NULL || !fleet[i]->is_active) continue;

        float x = fleet[i]->position.x;
        float y = fleet[i]->position.y;

        // 判断是否越界
        if (x < zone->x_min || x > zone->x_max
         || y < zone->y_min || y > zone->y_max) {

            // 记录越界无人机ID
            int idx = result->boundary_violations;
            if (idx < MAX_DRONE_COUNT) {
                result->boundary_ids[idx] = fleet[i]->id;
                result->boundary_violations++;
            }
        }
    }

    return result->boundary_violations;
}

/* ==================== 间距检测 ==================== */

int safety_check_distance(Drone* fleet[], int count,
                          const SafetyZone* zone, SafetyResult* result)
{
    if (fleet == NULL || zone == NULL || result == NULL) return 0;

    result->distance_violations = 0;
    int min_dist = zone->min_distance;

    // 两两比较，j 从 i+1 开始避免重复
    for (int i = 0; i < count; i++) {
        if (fleet[i] == NULL || !fleet[i]->is_active) continue;

        for (int j = i + 1; j < count; j++) {
            if (fleet[j] == NULL || !fleet[j]->is_active) continue;

            float dx = fleet[i]->position.x - fleet[j]->position.x;
            float dy = fleet[i]->position.y - fleet[j]->position.y;
            float dist = (float)sqrt(dx * dx + dy * dy);

            if (dist < min_dist) {
                // 记录违规对
                int idx = result->distance_violations;
                if (idx < MAX_DRONE_COUNT) {
                    result->pair_a[idx] = fleet[i]->id;
                    result->pair_b[idx] = fleet[j]->id;
                    result->distance_violations++;
                }
            }
        }
    }

    return result->distance_violations;
}

/* ==================== 综合检测 ==================== */

int safety_check_all(Drone* fleet[], int count,
                     const SafetyZone* zone, SafetyResult* result)
{
    if (fleet == NULL || zone == NULL || result == NULL) return 0;

    // 初始化结果
    safety_result_clear(result);

    // 执行两项检测
    int boundary_count  = safety_check_boundary(fleet, count, zone, result);
    int distance_count  = safety_check_distance(fleet, count, zone, result);

    // 返回总违规数
    return boundary_count + distance_count;
}

void safety_result_clear(SafetyResult* result)
{
    if (result == NULL) return;

    result->boundary_violations = 0;
    result->distance_violations = 0;

    for (int i = 0; i < MAX_DRONE_COUNT; i++) {
        result->boundary_ids[i] = 0;
        result->pair_a[i]       = 0;
        result->pair_b[i]       = 0;
    }
}

/* ==================== 碰撞自动避让 ==================== */

/**
 * @brief 碰撞自动避让
 *
 * 对距离过近的无人机对施加排斥力，自动推开。
 * 仅在距离 < 安全间距的一半时触发，轻微推开。
 */
void safety_avoid_collisions(Drone* fleet[], int count,
                             const SafetyZone* zone, float strength)
{
    if (fleet == NULL || zone == NULL) return;

    float avoid_dist = (float)zone->min_distance * 0.5f;
    if (avoid_dist < 1.0f) avoid_dist = 1.0f;  // 至少1格

    for (int i = 0; i < count; i++) {
        if (fleet[i] == NULL || !fleet[i]->is_active) continue;

        for (int j = i + 1; j < count; j++) {
            if (fleet[j] == NULL || !fleet[j]->is_active) continue;

            float dx = fleet[i]->position.x - fleet[j]->position.x;
            float dy = fleet[i]->position.y - fleet[j]->position.y;
            float dist = (float)sqrt(dx * dx + dy * dy);

            if (dist < avoid_dist && dist > 0.001f) {
                // 排斥力：距离越近力越大
                float force = (avoid_dist - dist) / avoid_dist * strength;
                float nx = dx / dist;
                float ny = dy / dist;

                // 两架无人机各向相反方向推开
                fleet[i]->position.x += nx * force;
                fleet[i]->position.y += ny * force;
                fleet[j]->position.x -= nx * force;
                fleet[j]->position.y -= ny * force;
            }
        }
    }
}
