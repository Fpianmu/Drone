/**
 * @file    formation.h
 * @brief   编队与图案生成模块 —— 图案生成器架构
 * @author  [你的名字]
 * @date    2026-06-08
 *
 * 本模块实现图案生成器架构：每种图案由一个独立的生成器函数实现，
 * 统一通过 pattern_generate() 调度接口调用。
 *
 * 支持的图案类型（PatternType 枚举）：
 *   几何图形：圆形、正方形、三角形、菱形、五角星、五边形、
 *            六边形、心形、螺旋线、直线、箭头、十字、弧形、网格
 *   特殊：   随机散布
 *
 * 扩展方式：新增图案只需添加一个生成器函数 + 在调度表中注册即可。
 */

#ifndef FORMATION_H
#define FORMATION_H

#include "common.h"

/* ==================== 生成器调度接口 ==================== */

/**
 * @brief 通用图案生成器
 *
 * 根据 PatternConfig 中的图案类型，调用对应的生成器函数，
 * 计算每架无人机在编队中的目标位置。
 *
 * @param cfg          图案配置（类型、中心、缩放、旋转）
 * @param drone_count  需要排布的无人机数量
 * @param out_positions 输出目标位置数组（长度至少为 drone_count）
 * @return 实际生成的坐标数量（= drone_count）
 */
int pattern_generate(PatternType type, Point2f center,
                     float scale, float rotation_deg,
                     int drone_count, Point2f out_positions[],
                     const char* text);

/**
 * @brief 创建编队（便捷函数）
 *
 * 分配并填充一个 Formation 结构体，自动计算每架无人机在图案中的目标位置。
 *
 * @param name         编队名称
 * @param type         图案类型
 * @param center       中心坐标
 * @param scale        缩放
 * @param rotation_deg 旋转角度
 * @param drone_count  参与的无人机数量
 * @return 堆上的 Formation 指针，调用者负责 formation_destroy 释放
 */
Formation* formation_create(const char* name, PatternType type,
                            Point2f center, float scale, float rotation_deg,
                            int drone_count, const char* display_text);

/**
 * @brief 销毁编队
 * @param formation 要释放的 Formation 指针
 */
void formation_destroy(Formation* formation);

/**
 * @brief 更新编队的图案参数（重新计算所有目标位置）
 * @param formation    目标编队
 * @param type         新图案类型
 * @param center       新中心坐标
 * @param scale        新缩放
 * @param rotation_deg 新旋转角度
 */
void formation_update(Formation* formation, PatternType type,
                      Point2f center, float scale, float rotation_deg);

/**
 * @brief 获取编队中指定无人机的目标位置
 * @param formation 编队
 * @param index    无人机在编队中的索引 [0, drone_count)
 * @param out_x     输出 X 坐标
 * @param out_y     输出 Y 坐标
 * @return 1=成功, 0=失败（索引越界）
 */
int formation_get_target(const Formation* formation, int index,
                         float* out_x, float* out_y);

/* ==================== 单个图案生成器（可直接调用） ==================== */

int gen_circle(Point2f center, float radius, int count, Point2f out[]);
int gen_square(Point2f center, float size, int count, Point2f out[]);
int gen_triangle(Point2f center, float size, int count, Point2f out[]);
int gen_diamond(Point2f center, float size, int count, Point2f out[]);
int gen_star(Point2f center, float size, int count, Point2f out[]);
int gen_pentagon(Point2f center, float size, int count, Point2f out[]);
int gen_hexagon(Point2f center, float size, int count, Point2f out[]);
int gen_heart(Point2f center, float size, int count, Point2f out[]);
int gen_spiral(Point2f center, float size, int count, Point2f out[]);
int gen_line(Point2f center, float size, int count, Point2f out[]);
int gen_arrow(Point2f center, float size, int count, Point2f out[]);
int gen_cross(Point2f center, float size, int count, Point2f out[]);
int gen_arc(Point2f center, float size, int count, Point2f out[]);
int gen_grid(Point2f center, float size, int count, Point2f out[]);
int gen_random(Point2f center, float size, int count, Point2f out[]);

/**
 * @brief 生成文字点阵编队
 *
 * 将输入字符串（英文/数字）转为 5×7 点阵，
 * 无人机排布在点阵的"亮"位置上。
 * 字符之间间隔 1 格，整行文字居中。
 *
 * @param center    文字区域中心
 * @param char_size  单字大小（字符格宽度）
 * @param text       要显示的文字（ASCII，≤5字）
 * @param count      可用无人机数量
 * @param out        输出位置数组
 * @return 实际使用的无人机数量（≤count）
 */
int gen_text(Point2f center, float char_size, const char* text,
             int count, Point2f out[]);

/**
 * @brief 获取图案建议参数（推荐的无人机数量 + 缩放值）
 * @param type       图案类型
 * @param text_len   文字长度（仅 PAT_TEXT 时有效，其他图案忽略）
 * @param out_count  输出：建议无人机数量
 * @param out_scale  输出：建议缩放值
 */
void pattern_recommend(PatternType type, int text_len,
                       int* out_count, float* out_scale);

/**
 * @brief 旋转一个坐标点（绕中心旋转指定角度）
 * @param cx     旋转中心 X
 * @param cy     旋转中心 Y
 * @param x      待旋转点 X
 * @param y      待旋转点 Y
 * @param deg    旋转角度（度）
 * @param out_x  输出 X
 * @param out_y  输出 Y
 */
void rotate_point(float cx, float cy, float x, float y, float deg,
                  float* out_x, float* out_y);

#endif // FORMATION_H
