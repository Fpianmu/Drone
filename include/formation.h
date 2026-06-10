/*
 * formation.h —— 编队与图案生成模块
 *
 * 这个模块负责"无人机应该排成什么图案"。
 * 每种图案类型对应一个生成器函数，统一通过 pattern_generate() 调度。
 *
 * 支持的图案：
 *   圆形、正方形、三角形、菱形、五角星、五边形、六边形、
 *   心形、螺旋线、直线、箭头、十字、弧形、网格、随机散布。
 *   PAT_TEXT 用 GDI 将文字渲染为位图后降采样生成编队。
 *
 * 扩展方式：新增一种图案只需写一个生成器，然后在 pattern_generate
 * 和 pattern_recommend 的 switch 中加一行。
 */

#ifndef FORMATION_H
#define FORMATION_H

#include "common.h"

/* ---- 核心调度接口 ---- */

/*
 * 通用图案生成器 —— 根据类型调用对应的生成器。
 *
 * center 是图案中心在舞台坐标系中的坐标（0~80, 0~40）。
 * scale 含义因图案而异（圆=半径，正方=边长……）。
 * rotation_deg 顺时针旋转角度。
 * text 仅在 PAT_TEXT 时使用（其他图案传 NULL）。
 * 返回实际填充的位置数量（通常等于 drone_count）。
 */
int pattern_generate(PatternType type, Point2f center,
                     float scale, float rotation_deg,
                     int drone_count, Point2f out_positions[],
                     const char* text);

/*
 * 创建编队 —— 调用 pattern_generate 自动计算所有目标位置。
 * display_text 仅在 PAT_TEXT 时传入要显示的文字（其他传 NULL）。
 */
Formation* formation_create(const char* name, PatternType type,
                            Point2f center, float scale, float rotation_deg,
                            int drone_count, const char* display_text);

void formation_destroy(Formation* formation);

// 更新编队参数（重新生成目标位置）。
void formation_update(Formation* formation, PatternType type,
                      Point2f center, float scale, float rotation_deg);

// 获取编队中第 index 架飞机的目标坐标（1=成功 0=越界）。
int formation_get_target(const Formation* formation, int index,
                         float* out_x, float* out_y);

/*
 * 获取图案建议参数 —— 不同图案的最优无人机数和缩放值不一样。
 * 例如圆形用 30 架、半径 18 格好看；五角星用 40 架、半径 18 格好看。
 * text_len 仅在 PAT_TEXT 时使用（其他图案忽略）。
 */
void pattern_recommend(PatternType type, int text_len,
                       int* out_count, float* out_scale);

/* ---- 单个图案生成器（可独立调用） ---- */

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
 * 文字编队生成器（GDI 渲染 + 降采样）
 *
 * 将 UTF-8 文字用 Windows GDI 渲染到内存位图，降采样为二值点阵，
 * 亮的像素放置无人机。支持中英文等所有 Windows 字体字符。
 */
int gen_text(Point2f center, float char_size, const char* text,
             int count, Point2f out[]);

/*
 * BMP 图片编队：读取 BMP 文件，转灰度 → 降采样 → 无人机排列
 * 不依赖任何第三方库，纯 Windows API + 位图文件头解析
 */
int gen_image(Point2f center, float char_size, const char* filename,
              int count, Point2f out[]);

// 绕 center 旋转一个点（旋转矩阵）。
void rotate_point(float cx, float cy, float x, float y, float deg,
                  float* out_x, float* out_y);

#endif
