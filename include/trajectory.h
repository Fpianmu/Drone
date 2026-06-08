/**
 * @file    trajectory.h
 * @brief   轨迹与插值模块 —— 关键航点管理与逐帧线性插值
 * @author  [你的名字]
 * @date    2026-06-08
 *
 * 本模块负责：
 * - 定义航点序列（WayPoint 数组）
 * - 逐帧线性插值计算当前位置（持续调用）
 * - 航点到达判定与自动切换
 * - 灯光同步切换（到达航点自动变更灯光状态）
 */

#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include "common.h"
#include "drone.h"
#include "formation.h"

/* ==================== 轨迹创建 ==================== */

/**
 * @brief 创建一个空轨迹对象
 * @return 堆上的 Trajectory 指针
 */
Trajectory* traj_create(void);

/**
 * @brief 销毁轨迹对象
 * @param traj 要释放的轨迹
 */
void traj_destroy(Trajectory* traj);

/**
 * @brief 向轨迹末尾添加一个航点
 * @param traj     目标轨迹
 * @param position  航点坐标
 * @param color    到达此航点后的灯光颜色
 * @param mode     到达此航点后的灯光模式
 * @param hold_ms  到达后停留时间（ms），0=不停留立即切换下一航点
 * @return 1=成功, 0=失败（航点已满）
 */
int traj_add_waypoint(Trajectory* traj, Point2f position,
                      LightColor color, LightMode mode, int hold_ms);

/**
 * @brief 清空轨迹，重置所有航点和进度
 * @param traj 目标轨迹
 */
void traj_clear(Trajectory* traj);

/* ==================== 逐帧更新（持续调用） ==================== */

/**
 * @brief 轨迹逐帧更新 —— 核心持续函数
 *
 * 每个模拟帧调用一次。根据设定的移动速度，沿直线插值当前航点→下一航点。
 * 到达航点后自动停留 hold_ms，然后切换到下一航点。
 *
 * @param traj         轨迹对象
 * @param drone        要移动的无人机
 * @param speed        移动速度（字符格/秒）
 * @param delta_ms     本帧时间增量（毫秒）
 * @return 1=仍在运动, 0=轨迹已走完（所有航点完成）
 */
int traj_update(Trajectory* traj, Drone* drone, float speed, int delta_ms);

/**
 * @brief 跳跃到指定航点（用于快速跳转或初始化）
 * @param traj    轨迹
 * @param drone   无人机
 * @param index   目标航点索引
 */
void traj_jump_to(Trajectory* traj, Drone* drone, int index);

/**
 * @brief 获取当前航点信息（供 UI 层查询显示）
 * @param traj        轨迹
 * @param out_index   输出：当前航点索引
 * @param out_total   输出：总航点数
 * @param out_progress 输出：整体进度 [0.0, 1.0]
 */
void traj_get_status(const Trajectory* traj, int* out_index,
                     int* out_total, float* out_progress);

/* ==================== 批量操作 ==================== */

/**
 * @brief 从编队生成轨迹
 *
 * 为 fleet 中的每架无人机计算从"当前位置"到"编队目标位置"的直线航点。
 * 通常用于队形切换：用户选择新图案 → 生成新编队 → 生成所有无人机的迁移轨迹。
 *
 * @param fleet      无人机编队
 * @param count      无人机数量
 * @param formation  目标编队
 * @param trajectories 输出轨迹数组（长度 count），每个指向新创建的 Trajectory
 * @param speed      移动速度
 * @param color      到达后的灯光颜色
 * @param mode       到达后的灯光模式
 * @return 1=成功, 0=失败
 */
int traj_from_formation(Drone* fleet[], int count, const Formation* formation,
                        Trajectory* trajectories[], float speed,
                        LightColor color, LightMode mode);

/**
 * @brief 批量更新轨迹
 *
 * 对编队中的所有无人机同时执行一帧轨迹更新。
 *
 * @param fleet        无人机编队
 * @param trajectories 对应的轨迹数组
 * @param count        数量
 * @param speed        速度
 * @param delta_ms     帧间隔
 * @return 仍在运动的无人机数量（全完成为0）
 */
int traj_update_fleet(Drone* fleet[], Trajectory* trajectories[],
                      int count, float speed, int delta_ms);

#endif // TRAJECTORY_H
