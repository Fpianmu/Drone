/*
 * drone.h —— 无人机实体模块
 *
 * 这个模块管理单架无人机的生命周期（创建/销毁）和状态操作。
 * 所有函数都通过传入 Drone* 指针来做操作，不使用全局变量。
 *
 * 主要职责：
 *   - 创建和销毁无人机
 *   - 批量创建/销毁无人机编队
 *   - 读取和改变无人机位置
 *   - 控制灯光（颜色、模式、闪烁间隔）
 *   - 每帧计算当前应显示的颜色（处理闪烁逻辑）
 */

#ifndef DRONE_H
#define DRONE_H

#include "common.h"

/* ---- 生命周期 ---- */

// 创建一架无人机，返回堆上的指针。初始位置 (x, y)，高度 height，默认白灯常亮。
Drone* drone_create(int id, float x, float y, float height);

// 销毁一架无人机，释放内存。
void drone_destroy(Drone* drone);

// 批量创建 count 架无人机，编号从 start_id 开始。
// 返回指针数组（堆分配），失败时自动回滚已创建的。
Drone** drone_create_fleet(int count, int start_id);

// 批量销毁。
void drone_destroy_fleet(Drone** fleet, int count);

/* ---- 位置操作 ---- */

// 直接设置位置（瞬移，通常用于初始化）。
void drone_set_position(Drone* drone, float x, float y, float height);

// 微小移动（用于逐帧插值。dx, dy, dh 是本帧的偏移量）。
void drone_move(Drone* drone, float dx, float dy, float dh);

// 获取四舍五入后的整数显示坐标。
void drone_get_display_pos(const Drone* drone, int* x, int* y);

/* ---- 灯光操作 ---- */

// 设置灯光颜色。
void drone_set_light_color(Drone* drone, LightColor color);

// 设置灯光模式（常亮/闪烁/关闭）。切换模式时会重置闪烁计时器。
void drone_set_light_mode(Drone* drone, LightMode mode);

// 设置闪烁间隔（毫秒），小于50ms会被强制设为50ms。
void drone_set_blink_interval(Drone* drone, int interval_ms);

// 直接开灯或关灯（关灯 = LIGHT_OFF，开灯 = 恢复到 LIGHT_STEADY）。
void drone_light_onoff(Drone* drone, int on);

/* ---- 状态查询 ---- */

// 检查无人机是否活跃（1=活跃, 0=休眠）。
int drone_is_active(const Drone* drone);

/*
 * 获取本帧应显示的控制台颜色 —— 每帧调用。
 *
 * 这个函数会读取无人机的灯光模式：
 *   - 如果是常亮，直接返回颜色
 *   - 如果是闪烁，累加计时器，到间隔后翻转可见性
 *   - 如果是关闭，返回黑色
 *
 * delta_ms 是本帧时间增量，用于驱动闪烁计时器。
 */
ConsoleColor drone_get_current_color(Drone* drone, int delta_ms);

#endif
