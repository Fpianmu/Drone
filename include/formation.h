/*
 * formation.h - 编队与图案生成模块
 *
 * 这个模块负责"无人机应该排成什么图案"。
 * 每种图案是一个独立的生成器函数，统一通过 pattern_generate() 调度。
 *
 * 当前支持的图案：
 *   几何：圆形、正方形、三角形、菱形、五角星、五边形、六边形、
 *        心形、螺旋线、直线、箭头、十字、弧形、网格、随机散布
 *   文字：GDI 渲染后降采样为点阵（支持中英文）
 *   图片：BMP 加载后降采样为像素阵列
 *
 * 想要新增一种图案的话，写一个生成器函数，然后在 pattern_generate
 * 和 pattern_recommend 的 switch 里各加一行就行。
 */

#ifndef FORMATION_H
#define FORMATION_H

#include "common.h"

/* ---- 核心调度 ---- */

/*
 * 通用图案生成器。
 *
 * center 是图案中心坐标。
 * scale 含义因图案而异（圆=半径，正方=边长……）。
 * rotation_deg 是旋转角度。
 * text 只在 PAT_TEXT 时用（其他图案传 NULL）。
 * 返回实际生成的位置数量。
 */
int pattern_generate(PatternType type, Point2f center,
                     float scale, float rotation_deg,
                     int drone_count, Point2f out_positions[],
                     const char* text);

/*
 * 创建编队对象（会自动调用 pattern_generate 来填充目标坐标）。
 * display_text 只在 PAT_TEXT 时传入要显示的文字。
 */
Formation* formation_create(const char* name, PatternType type,
                            Point2f center, float scale, float rotation_deg,
                            int drone_count, const char* display_text);

void formation_destroy(Formation* formation);

// 更新编队参数（会重新生成所有目标位置）。
void formation_update(Formation* formation, PatternType type,
                      Point2f center, float scale, float rotation_deg);

// 拿到编队中第 index 架飞机的目标坐标。成功返回 1，越界返回 0。
int formation_get_target(const Formation* formation, int index,
                         float* out_x, float* out_y);

/*
 * 获取推荐参数。
 *
 * 不同图案需要不同的无人机数量和缩放才好看，
 * 这个函数返回每种图案的建议值。
 */
void pattern_recommend(PatternType type, int text_len,
                       int* out_count, float* out_scale);

/* ---- 每种图案的生成器（也可以单独调用） ---- */

int gen_circle   (Point2f center, float size, int count, Point2f out[]);
int gen_square   (Point2f center, float size, int count, Point2f out[]);
int gen_triangle (Point2f center, float size, int count, Point2f out[]);
int gen_diamond  (Point2f center, float size, int count, Point2f out[]);
int gen_star     (Point2f center, float size, int count, Point2f out[]);
int gen_pentagon (Point2f center, float size, int count, Point2f out[]);
int gen_hexagon  (Point2f center, float size, int count, Point2f out[]);
int gen_heart    (Point2f center, float size, int count, Point2f out[]);
int gen_spiral   (Point2f center, float size, int count, Point2f out[]);
int gen_line     (Point2f center, float size, int count, Point2f out[]);
int gen_arrow    (Point2f center, float size, int count, Point2f out[]);
int gen_cross    (Point2f center, float size, int count, Point2f out[]);
int gen_arc      (Point2f center, float size, int count, Point2f out[]);
int gen_grid     (Point2f center, float size, int count, Point2f out[]);
int gen_random   (Point2f center, float size, int count, Point2f out[]);

/*
 * 文字编队生成器。
 *
 * 每个中文字渲染到 12x12 像素的小位图，英文 6x12，
 * 然后逐个像素读出来：白像素 -> 放一架无人机。
 * 思路和旧版英文 5x7 点阵一样，只是用 GDI 代替手工字库。
 */
int gen_text(Point2f center, float char_size, const char* text,
             int count, Point2f out[]);

/*
 * BMP 图片编队生成器。
 *
 * 用 GDI 加载 BMP，StretchBlt 缩到目标网格，
 * 逐像素采样：暗色 -> 放无人机（白底黑画取暗点）。
 */
int gen_image(Point2f center, float char_size, const char* filename,
              int count, Point2f out[]);

// 绕一个中心点旋转坐标（二维旋转矩阵）。
void rotate_point(float cx, float cy, float x, float y, float deg,
                  float* out_x, float* out_y);

#endif
