/**
 * @file    trajectory.cpp
 * @brief   轨迹与插值模块实现
 * @author  [你的名字]
 * @date    2026-06-08
 *
 * 逐帧线性插值算法：
 *   每帧计算：delta_pixels = speed * delta_ms / 1000.0
 *   沿方向向量归一化后按步长推进
 *   到达目标航点后停留 hold_ms，然后自动切换下一航点
 */

#include "../include/trajectory.h"

/* ==================== 生命周期 ==================== */

Trajectory* traj_create(void)
{
    Trajectory* traj = (Trajectory*)malloc(sizeof(Trajectory));
    if (traj == NULL) return NULL;

    traj->waypoint_count = 0;
    traj->current_index  = 0;
    traj->total_progress = 0.0f;

    return traj;
}

void traj_destroy(Trajectory* traj)
{
    if (traj != NULL) {
        free(traj);
    }
}

int traj_add_waypoint(Trajectory* traj, Point2f position,
                      LightColor color, LightMode mode, int hold_ms)
{
    if (traj == NULL) return 0;
    if (traj->waypoint_count >= MAX_WAYPOINTS) return 0;

    int idx = traj->waypoint_count;

    traj->waypoints[idx].position    = position;
    traj->waypoints[idx].light_color = color;
    traj->waypoints[idx].light_mode  = mode;
    traj->waypoints[idx].hold_ms     = hold_ms;

    traj->waypoint_count++;

    return 1;
}

void traj_clear(Trajectory* traj)
{
    if (traj == NULL) return;

    traj->waypoint_count = 0;
    traj->current_index  = 0;
    traj->total_progress = 0.0f;
}

/* ==================== 逐帧更新 ==================== */

/**
 * @brief 轨迹逐帧更新
 *
 * 逻辑流程：
 *   1. 检查是否已走完所有航点 → 返回 0
 *   2. 计算当前位置到目标航点的直线距离
 *   3. 如果距离很小（< 1px），判定为到达
 *   4. 到达后如果有 hold_ms > 0，先停留
 *   5. 停留结束后或无需停留，切换到下一航点
 *   6. 如果未到达，沿方向移动一步
 */
int traj_update(Trajectory* traj, Drone* drone, float speed, int delta_ms)
{
    if (traj == NULL || drone == NULL) return 0;

    // 如果轨迹已走完
    if (traj->current_index >= traj->waypoint_count) {
        return 0;
    }

    // 当前目标航点
    const WayPoint* target = &traj->waypoints[traj->current_index];

    // 计算到目标的距离
    float dx = target->position.x - drone->position.x;
    float dy = target->position.y - drone->position.y;
    float dist = (float)sqrt(dx * dx + dy * dy);

    // 每帧可移动的距离（字符格）
    float step = speed * delta_ms / 1000.0f;

    if (dist < 0.5f) {
        // 已到达航点 → 同步灯光状态
        drone_set_light_color(drone, target->light_color);
        drone_set_light_mode(drone, target->light_mode);

        // 检查是否需要停留
        if (target->hold_ms > 0) {
            // TODO: 停留计时由外部控制或增加停留计时器
            // 当前简化实现：停留后再切到下一航点
            // （停留可通过在主循环中保持同一航点来实现）
        }

        // 切换到下一个航点
        traj->current_index++;

        // 更新整体进度
        if (traj->waypoint_count > 0) {
            traj->total_progress = (float)traj->current_index
                                   / traj->waypoint_count;
        }

        // 检查是否全部完成
        if (traj->current_index >= traj->waypoint_count) {
            return 0;   // 轨迹结束
        }

        return 1;   // 切换到下一航点继续
    }

    // 未到达 → 沿方向移动一步
    float nx = dx / dist;   // 方向向量归一化 X
    float ny = dy / dist;   // 方向向量归一化 Y

    // 移动（不超过目标距离）
    if (step > dist) step = dist;

    drone_move(drone, nx * step, ny * step, 0.0f);

    return 1;   // 仍在运动
}

void traj_jump_to(Trajectory* traj, Drone* drone, int index)
{
    if (traj == NULL || drone == NULL) return;
    if (index < 0 || index >= traj->waypoint_count) return;

    // 直接设置位置到目标航点
    const WayPoint* wp = &traj->waypoints[index];
    drone_set_position(drone, wp->position.x, wp->position.y, drone->height);
    drone_set_light_color(drone, wp->light_color);
    drone_set_light_mode(drone, wp->light_mode);

    traj->current_index = index;
}

void traj_get_status(const Trajectory* traj, int* out_index,
                     int* out_total, float* out_progress)
{
    if (traj == NULL) {
        if (out_index)    *out_index    = 0;
        if (out_total)    *out_total    = 0;
        if (out_progress) *out_progress = 0.0f;
        return;
    }

    if (out_index)    *out_index    = traj->current_index;
    if (out_total)    *out_total    = traj->waypoint_count;
    if (out_progress) *out_progress = traj->total_progress;
}

/* ==================== 批量操作 ==================== */

int traj_from_formation(Drone* fleet[], int count, const Formation* formation,
                        Trajectory* trajectories[], float speed,
                        LightColor color, LightMode mode)
{
    if (fleet == NULL || formation == NULL || trajectories == NULL) return 0;
    if (count <= 0 || count > MAX_DRONE_COUNT) return 0;

    for (int i = 0; i < count; i++) {
        if (fleet[i] == NULL) continue;

        // 为每架无人机创建独立轨迹
        trajectories[i] = traj_create();
        if (trajectories[i] == NULL) {
            // 回滚：释放已创建的轨迹
            for (int j = 0; j < i; j++) {
                traj_destroy(trajectories[j]);
                trajectories[j] = NULL;
            }
            return 0;
        }

        // 航点 = 编队中该无人机的目标位置（单点直达）
        Point2f target;
        if (formation_get_target(formation, i, &target.x, &target.y)) {
            traj_add_waypoint(trajectories[i], target, color, mode, 0);
        }
    }

    return 1;
}

int traj_update_fleet(Drone* fleet[], Trajectory* trajectories[],
                      int count, float speed, int delta_ms)
{
    if (fleet == NULL || trajectories == NULL) return 0;

    int still_moving = 0;   // 仍在移动的无人机计数

    for (int i = 0; i < count; i++) {
        if (fleet[i] == NULL || trajectories[i] == NULL) continue;

        // 逐帧更新每架无人机的轨迹
        int moving = traj_update(trajectories[i], fleet[i], speed, delta_ms);
        if (moving) {
            still_moving++;
        }
    }

    return still_moving;
}
