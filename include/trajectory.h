/*
 * trajectory.h —— 轨迹与插值模块
 *
 * 轨迹是一串"航点"序列，无人机从当前位置出发，依次飞过每个航点。
 * 每帧沿当前航点到下一航点的方向移动一小步（线性插值），到达后自动切换。
 *
 * 主要用于两种场景：
 *   1. 创建编队时：为每架飞机生成一条从"当前位置"到"目标编队位置"的轨迹
 *   2. 逐帧更新时：每架飞机沿自己轨迹移动一步
 */

#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include "common.h"
#include "drone.h"
#include "formation.h"

/* ---- 生命周期 ---- */

Trajectory* traj_create(void);              // 创建空轨迹
void        traj_destroy(Trajectory* traj); // 销毁
void        traj_clear(Trajectory* traj);   // 清空所有航点

// 向轨迹末尾追加一个航点。hold_ms：到达后停留多久（0=不停留）
int traj_add_waypoint(Trajectory* traj, Point2f position,
                      LightColor color, LightMode mode, int hold_ms);

/* ---- 逐帧更新（每帧调用）---- */

/*
 * 轨迹主更新 —— 向当前目标航点直线移动一步
 *
 * 移动步长 = speed × delta_ms / 1000（字符格）
 * 到达目标航点后自动切换灯光并跳到下一航点。
 * 返回 1 表示仍在移动，0 表示轨迹已走完。
 */
int traj_update(Trajectory* traj, Drone* drone, float speed, int delta_ms);

// 直接跳到指定航点（瞬移）
void traj_jump_to(Trajectory* traj, Drone* drone, int index);

// 查询当前进度（当前航点索引 / 总数 / 整体进度）
void traj_get_status(const Trajectory* traj, int* out_index,
                     int* out_total, float* out_progress);

/* ---- 批量操作 ---- */

/*
 * 从编队生成所有飞机的轨迹 —— 每架飞机建立一个从"当前位置"到"编队目标位置"的轨迹
 */
int traj_from_formation(Drone* fleet[], int count, const Formation* formation,
                        Trajectory* trajectories[], float speed,
                        LightColor color, LightMode mode);

// 批量更新所有飞机的轨迹，返回仍在移动的飞机数量
int traj_update_fleet(Drone* fleet[], Trajectory* trajectories[],
                      int count, float speed, int delta_ms);

#endif
