/**
 * @file    safety.h
 * @brief   安全检测模块 —— 边界越界 & 无人机间距碰撞预警
 * @author  [你的名字]
 * @date    2026-06-08
 *
 * 每帧自动执行的安全检测（持续运行）：
 * - 边界检测：判断无人机坐标是否超出表演区范围
 * - 间距检测：两两计算欧氏距离，判断是否小于安全阈值
 *
 * 检测结果通过 SafetyResult 结构体返回，供渲染层可视化告警。
 */

#ifndef SAFETY_H
#define SAFETY_H

#include "common.h"

/* ==================== 安全区域管理 ==================== */

/**
 * @brief 创建安全检测区域配置
 * @param x_min        表演区左边界
 * @param y_min        表演区上边界
 * @param x_max        表演区右边界
 * @param y_max        表演区下边界
 * @param min_distance 最小安全间距
 * @return 堆上的 SafetyZone 指针
 */
SafetyZone* safety_zone_create(int x_min, int y_min, int x_max, int y_max,
                               int min_distance);

/**
 * @brief 销毁安全区域配置
 * @param zone 要释放的 SafetyZone
 */
void safety_zone_destroy(SafetyZone* zone);

/**
 * @brief 检查坐标是否在安全区域内
 * @param zone 安全区域
 * @param x    X坐标
 * @param y    Y坐标
 * @return 1=在区域内, 0=越界
 */
int safety_point_in_zone(const SafetyZone* zone, float x, float y);

/* ==================== 安全检测（持续调用） ==================== */

/**
 * @brief 执行边界检测 —— 每帧调用
 *
 * 遍历所有活跃无人机，判断是否超出表演区域。
 *
 * @param fleet   无人机编队
 * @param count   无人机数量
 * @param zone    安全区域配置
 * @param result  输出检测结果（边界违规列表）
 * @return 越界的无人机数量（0 = 全部安全）
 */
int safety_check_boundary(Drone* fleet[], int count,
                          const SafetyZone* zone, SafetyResult* result);

/**
 * @brief 执行间距检测 —— 每帧调用
 *
 * 两两计算欧氏距离，找出所有距离小于 min_distance 的无人机对。
 * 时间复杂度：O(n²)，对于 n ≤ 100 在可接受范围内。
 *
 * @param fleet   无人机编队
 * @param count   无人机数量
 * @param zone    安全区域（提供最小间距阈值）
 * @param result  输出检测结果（间距违规对列表）
 * @return 间距违规的无人机对数量（0 = 全部安全）
 */
int safety_check_distance(Drone* fleet[], int count,
                          const SafetyZone* zone, SafetyResult* result);

/**
 * @brief 一键综合安全检测 —— 每帧调用
 *
 * 同时执行边界检测 + 间距检测，汇总在一次调用中返回。
 * 推荐在主循环中使用此函数，减少函数调用开销。
 *
 * @param fleet   无人机编队
 * @param count   无人机数量
 * @param zone    安全区域配置
 * @param result  输出综合检测结果
 * @return 0=全部安全, >0=有问题（越界数+间距违规对数的总和）
 */
int safety_check_all(Drone* fleet[], int count,
                     const SafetyZone* zone, SafetyResult* result);

/**
 * @brief 初始化 SafetyResult 为空（无违规）
 * @param result 要初始化的结果结构体
 */
void safety_result_clear(SafetyResult* result);

/**
 * @brief 碰撞自动避让 —— 每帧调用
 *
 * 对距离过近（< min_distance/2）的无人机施加排斥力，
 * 自动将它们推开，防止视觉上的重叠。
 * 排斥力随距离减小而增大，超过阈值后不施加力。
 *
 * @param fleet   无人机编队
 * @param count   无人机数量
 * @param zone    安全区域（含最小间距）
 * @param strength 排斥力度（0.0~1.0，建议0.3）
 */
void safety_avoid_collisions(Drone* fleet[], int count,
                             const SafetyZone* zone, float strength);

#endif // SAFETY_H
