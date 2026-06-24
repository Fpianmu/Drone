/*
 * trajectory.h - 轨迹与插值模块
 *
 * 轨迹 = 一串"航点"。无人机从当前位置出发，逐个飞过每个航点。
 * 每帧沿当前航点方向走一小步（线性插值），到达后自动切到下一个。
 *
 * 两种使用场景：
 *   1. 切图案时：给每架飞机建一条"当前位置 -> 编队目标位置"的轨迹
 *   2. 每帧更新时：每架飞机沿自己的轨迹移动一步
 */

#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include "common.h"
#include "drone.h"
#include "formation.h"

/* ---- 生命周期 ---- */

Trajectory* traj_create(void);              // 创建一条空轨迹
void        traj_destroy(Trajectory* traj); // 销毁轨迹
void        traj_clear(Trajectory* traj);   // 清空所有航点

// 向轨迹末尾追加一个航点。
// hold_ms：到达后停多久（0 = 立刻切到下一航点）。
int traj_add_waypoint(Trajectory* traj, Point2f position,
                      LightColor color, LightMode mode, int hold_ms);

/* ---- 每帧调用 ---- */

/*
 * 轨迹主更新函数。
 *
 * 1. 算出到目标航点的距离
 * 2. 距离 < 0.5 格 -> 判定到达，吸附到精确位置，切下一航点
 * 3. 步长 >= 剩余距离 -> 直接吸附到目标
 * 4. 否则 -> 沿方向向量走一步（速度 x 时间 / 1000）
 *
 * 返回 1 = 还在走，0 = 走完了。
 */
int traj_update(Trajectory* traj, Drone* drone, float speed, int delta_ms);

// 直接跳到第 index 个航点（瞬移）。
void traj_jump_to(Trajectory* traj, Drone* drone, int index);

// 查询当前进度（当前是第几个航点/总共多少个/整体进度）。
void traj_get_status(const Trajectory* traj, int* out_index,
                     int* out_total, float* out_progress);

/* ---- 批量操作 ---- */

/*
 * 从编队给每架飞机生成轨迹。
 * 每架飞机一条：从当前位置 -> 编队中的目标位置（单航点）。
 */
int traj_from_formation(Drone* fleet[], int count, const Formation* formation,
                        Trajectory* trajectories[], float speed,
                        LightColor color, LightMode mode);

// 批量更新所有飞机的轨迹。返回还在动的飞机数量。
int traj_update_fleet(Drone* fleet[], Trajectory* trajectories[],
                      int count, float speed, int delta_ms);

#endif
