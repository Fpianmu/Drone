/**
 * @file    drone.h
 * @brief   无人机实体模块 —— 无人机生命周期管理与状态操作
 * @author  [你的名字]
 * @date    2026-06-08
 *
 * 本模块封装了单架无人机的所有状态（位置、高度、灯光）
 * 以及与无人机相关的批量操作。
 * 所有函数通过传入 Drone* 指针操作，不使用全局变量。
 */

#ifndef DRONE_H
#define DRONE_H

#include "common.h"

/* ==================== 生命周期管理 ==================== */

/**
 * @brief 创建一架无人机
 * @param id    无人机编号
 * @param x     初始 X 坐标
 * @param y     初始 Y 坐标
 * @param height 初始飞行高度（米）
 * @return 堆上分配的 Drone 指针，调用者负责 drone_destroy 释放
 */
Drone* drone_create(int id, float x, float y, float height);

/**
 * @brief 销毁一架无人机，释放内存
 * @param drone 要销毁的无人机指针
 */
void drone_destroy(Drone* drone);

/**
 * @brief 批量创建无人机编队
 * @param count     创建的无人机数量
 * @param start_id  起始编号
 * @return Drone* 数组（堆上分配），数量为 count
 */
Drone** drone_create_fleet(int count, int start_id);

/**
 * @brief 批量销毁无人机编队
 * @param fleet 无人机指针数组
 * @param count 无人机数量
 */
void drone_destroy_fleet(Drone** fleet, int count);

/* ==================== 位置操作 ==================== */

/**
 * @brief 设置无人机位置（瞬移，用于初始化）
 * @param drone 目标无人机
 * @param x     X 坐标
 * @param y     Y 坐标
 * @param height 飞行高度
 */
void drone_set_position(Drone* drone, float x, float y, float height);

/**
 * @brief 线性移动无人机（用于逐帧插值）
 * @param drone   目标无人机
 * @param dx      沿 X 方向移动量
 * @param dy      沿 Y 方向移动量
 * @param dh      高度变化量
 */
void drone_move(Drone* drone, float dx, float dy, float dh);

/**
 * @brief 获取无人机当前 2D 投影坐标
 * @param drone 目标无人机
 * @param x     输出 X 坐标（整数，用于显示）
 * @param y     输出 Y 坐标（整数，用于显示）
 */
void drone_get_display_pos(const Drone* drone, int* x, int* y);

/* ==================== 灯光操作 ==================== */

/**
 * @brief 设置无人机的灯光颜色
 * @param drone 目标无人机
 * @param color 新颜色
 */
void drone_set_light_color(Drone* drone, LightColor color);

/**
 * @brief 设置无人机的灯光模式
 * @param drone 目标无人机
 * @param mode  新模式（常亮/闪烁/关闭）
 */
void drone_set_light_mode(Drone* drone, LightMode mode);

/**
 * @brief 设置闪烁间隔
 * @param drone      目标无人机
 * @param interval_ms 闪烁间隔（毫秒）
 */
void drone_set_blink_interval(Drone* drone, int interval_ms);

/**
 * @brief 开关无人机灯光
 * @param drone目标无人机
 * @param on   1=开灯, 0=关灯
 */
void drone_light_onoff(Drone* drone, int on);

/* ==================== 状态查询 ==================== */

/**
 * @brief 判断无人机是否活跃
 * @param drone 目标无人机
 * @return 1=活跃, 0=停用
 */
int drone_is_active(const Drone* drone);

/**
 * @brief 返回无人机当前应显示的颜色（考虑闪烁状态）
 *
 * 持续运行的函数：每帧调用，根据闪烁计时器自动切换亮/灭。
 *
 * @param drone     目标无人机
 * @param delta_ms  本帧时间增量（毫秒）
 * @return 当前应显示的控制台颜色属性（ConsoleColor），若熄灭返回 CON_BLACK
 */
ConsoleColor drone_get_current_color(Drone* drone, int delta_ms);

#endif // DRONE_H
