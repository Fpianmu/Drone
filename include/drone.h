/*
 * drone.h - 无人机实体模块
 *
 * 管理单架无人机的创建、销毁、位置和灯光。
 * 所有函数通过传入 Drone* 指针操作，不使用全局变量。
 */

#ifndef DRONE_H
#define DRONE_H

#include "common.h"

/* ---- 生命周期 ---- */

// 创建一架新的无人机。id=编号, (x,y)=初始坐标, height=飞行高度。
// 新无人机默认白色常亮，标记为活跃。
Drone* drone_create(int id, float x, float y, float height);

// 销毁一架无人机，释放内存。
void drone_destroy(Drone* drone);

// 批量创建 count 架无人机，编号从 start_id 开始。
// 返回一个指针数组（堆上分配），所有新飞机初始位置在舞台中央。
// 如果某架创建失败，会回滚释放之前已创建的。
Drone** drone_create_fleet(int count, int start_id);

// 批量销毁无人机编队。
void drone_destroy_fleet(Drone** fleet, int count);

/* ---- 位置操作 ---- */

// 直接设置无人机位置（瞬移，初始化或吸附到目标时使用）。
void drone_set_position(Drone* drone, float x, float y, float height);

// 微小移动（每帧插值用。dx, dy, dh 是本帧的偏移量）。
void drone_move(Drone* drone, float dx, float dy, float dh);

// 拿到四舍五入后的整数坐标（用来在控制台上定位）。
void drone_get_display_pos(const Drone* drone, int* x, int* y);

/* ---- 灯光操作 ---- */

// 设置灯光颜色。
void drone_set_light_color(Drone* drone, LightColor color);

// 设置灯光模式（常亮/闪烁/关闭）。切换模式会重置闪烁计时器。
void drone_set_light_mode(Drone* drone, LightMode mode);

// 设置闪烁间隔（毫秒）。小于 50ms 会被强制设为 50ms。
void drone_set_blink_interval(Drone* drone, int interval_ms);

// 简单的开关灯。on=1 开灯（恢复常亮），on=0 关灯。
void drone_light_onoff(Drone* drone, int on);

/* ---- 状态查询 ---- */

// 检查无人机是否活跃（1=活跃, 0=休眠）。
int drone_is_active(const Drone* drone);

/*
 * 获取当前帧应该显示的颜色——每帧调用。
 *
 * 根据灯光模式决定返回什么颜色：
 * - LIGHT_OFF：返回黑色（灭灯）
 * - LIGHT_STEADY：返回设置的颜色
 * - LIGHT_BLINK：累加计时器，到间隔后翻转亮灭状态
 *
 * delta_ms 是本帧经过的时间（毫秒），用来驱动闪烁计时器。
 */
ConsoleColor drone_get_current_color(Drone* drone, int delta_ms);

#endif
