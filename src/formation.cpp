/**
 * @file    formation.cpp
 * @brief   编队与图案生成模块实现
 * @author  [你的名字]
 * @date    2026-06-08
 *
 * 每种图案类型的生成算法：
 *   圆形 —— 等角度分布在圆周上
 *   正方形 —— 等间距分布在外框上
 *   三角形 —— 均匀分布在三条边上
 *   菱形 —— 四条边等间距
 *   五角星 —— 10个顶点（外5 + 内5），按需分配
 *   正五边形/六边形 —— 等间距分布在边上
 *   心形 —— 参数方程 (x = 16sin³t, y = 13cost - 5cos2t - 2cos3t - cos4t)
 *   螺旋线 —— 阿基米德螺旋 r = a * θ
 *   直线 —— 等间距横排
 *   箭头 —— 箭身直线 + 箭尾三角形
 *   十字 —— 横竖两条线
 *   弧形 —— 等角度分布在圆弧上
 *   网格 —— row * col 矩形网格
 *   随机 —— 随机坐标
 */

#include "../include/formation.h"

/* ==================== 几何工具函数 ==================== */

/**
 * @brief 绕中心点旋转坐标（二维旋转矩阵）
 *
 *   x' = cx + (x-cx)*cos(θ) - (y-cy)*sin(θ)
 *   y' = cy + (x-cx)*sin(θ) + (y-cy)*cos(θ)
 */
void rotate_point(float cx, float cy, float x, float y, float deg,
                  float* out_x, float* out_y)
{
    float rad  = DEG2RAD(deg);
    float cosA = (float)cos(rad);
    float sinA = (float)sin(rad);

    float dx = x - cx;
    float dy = y - cy;

    *out_x = cx + dx * cosA - dy * sinA;
    *out_y = cy + dx * sinA + dy * cosA;
}

/* ==================== 图案生成器实现 ==================== */

/**
 * @brief 生成圆形编队
 *
 * 无人机等角度分布在圆周上。
 * 圆的参数方程：x = cx + R*cos(θ), y = cy + R*sin(θ)
 *
 * @param center  圆心
 * @param radius  半径
 * @param count   无人机数量
 * @param out     输出位置数组
 * @return 生成的位置数量
 */
int gen_circle(Point2f center, float radius, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    float angle_step = 360.0f / count;  // 每个无人机之间的角度间隔

    for (int i = 0; i < count; i++) {
        float angle = DEG2RAD(angle_step * i);
        out[i].x = center.x + radius * (float)cos(angle);
        out[i].y = center.y + radius * (float)sin(angle);
    }

    return count;
}

/**
 * @brief 生成正方形编队
 *
 * 无人机等间距分布在正方形四条边上。
 * 周长 = 4 * side, 步长 = 周长 / count
 *
 * @param center  正方形中心
 * @param side    边长
 * @param count   无人机数量
 * @param out     输出位置数组
 * @return 生成的位置数量
 */
int gen_square(Point2f center, float side, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    float half   = side / 2.0f;         // 半边长
    float perimeter = 4.0f * side;      // 总周长
    float step  = perimeter / count;    // 每架无人机间隔

    for (int i = 0; i < count; i++) {
        float dist = step * i;  // 从起点出发的距离

        // 按距离确定在哪条边上（上 → 右 → 下 → 左）
        if (dist < side) {
            // 上边：从左到右
            out[i].x = center.x - half + dist;
            out[i].y = center.y - half;
        } else if (dist < 2.0f * side) {
            // 右边：从上到下
            out[i].x = center.x + half;
            out[i].y = center.y - half + (dist - side);
        } else if (dist < 3.0f * side) {
            // 下边：从右到左
            out[i].x = center.x + half - (dist - 2.0f * side);
            out[i].y = center.y + half;
        } else {
            // 左边：从下到上
            out[i].x = center.x - half;
            out[i].y = center.y + half - (dist - 3.0f * side);
        }
    }

    return count;
}

/**
 * @brief 生成正三角形编队
 *
 * 无人机均匀分布在三条边上。
 * 底边水平，顶点在上。
 *
 * @param center  三角形重心
 * @param size    边长
 * @param count   无人机数量
 * @param out     输出位置数组
 * @return 生成的位置数量
 */
int gen_triangle(Point2f center, float size, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    // 正三角形三个顶点（重心在 center）
    float h = size * 0.8660254f;  // size * sqrt(3)/2
    // 顶点在上
    float x0 = center.x;           float y0 = center.y - h * 2.0f / 3.0f;
    float x1 = center.x - size / 2.0f; float y1 = center.y + h / 3.0f;
    float x2 = center.x + size / 2.0f; float y2 = center.y + h / 3.0f;

    // 三条边等分
    int per_side = count / 3;
    int rem = count % 3;  // 余数分配

    int sides[3] = { per_side, per_side, per_side };
    if (rem > 0) sides[0]++;
    if (rem > 1) sides[1]++;

    int idx = 0;

    // 边 0→1
    for (int i = 0; i < sides[0] && idx < count; i++) {
        float t = (sides[0] == 1) ? 0.0f : (float)i / (sides[0] - 1);
        out[idx].x = x0 + (x1 - x0) * t;
        out[idx].y = y0 + (y1 - y0) * t;
        idx++;
    }

    // 边 1→2
    for (int i = 0; i < sides[1] && idx < count; i++) {
        float t = (sides[1] == 1) ? 0.0f : (float)i / (sides[1] - 1);
        out[idx].x = x1 + (x2 - x1) * t;
        out[idx].y = y1 + (y2 - y1) * t;
        idx++;
    }

    // 边 2→0
    for (int i = 0; i < sides[2] && idx < count; i++) {
        float t = (sides[2] == 1) ? 0.0f : (float)i / (sides[2] - 1);
        out[idx].x = x2 + (x0 - x2) * t;
        out[idx].y = y2 + (y0 - y2) * t;
        idx++;
    }

    return count;
}

/**
 * @brief 生成菱形编队
 *
 * 菱形 = 上下左右四个顶点，四条边等分。
 * 对角线一横一竖。
 *
 * @param center  菱形中心
 * @param size    对角线长度
 * @param count   无人机数量
 * @param out     输出位置数组
 * @return 生成的位置数量
 */
int gen_diamond(Point2f center, float size, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    float half_x = size / 2.0f;       // 水平半轴
    float half_y = size / 2.0f * 0.7f; // 垂直半轴（稍扁）

    // 四个顶点
    float pts[4][2] = {
        { center.x,           center.y - half_y },  // 上
        { center.x + half_x,  center.y          },  // 右
        { center.x,           center.y + half_y },  // 下
        { center.x - half_x,  center.y          },  // 左
    };

    // 四条边等分
    int per_side = count / 4;
    int rem = count % 4;
    int sides[4] = { per_side, per_side, per_side, per_side };
    for (int s = 0; s < rem; s++) sides[s]++;

    int idx = 0;

    for (int s = 0; s < 4; s++) {
        int next_s = (s + 1) % 4;
        float x0 = pts[s][0];     float y0 = pts[s][1];
        float x1 = pts[next_s][0]; float y1 = pts[next_s][1];

        for (int i = 0; i < sides[s] && idx < count; i++) {
            float t = (sides[s] == 1) ? 0.0f : (float)i / (sides[s] - 1);
            out[idx].x = x0 + (x1 - x0) * t;
            out[idx].y = y0 + (y1 - y0) * t;
            idx++;
        }
    }

    return count;
}

/**
 * @brief 生成五角星编队
 *
 * 五角星有 10 个顶点（外圈5个 + 内圈5个交替排列）。
 * 外圈半径 R，内圈半径 r ≈ 0.382R。
 * 根据 count 数量分配无人机到这10个顶点及连接边上。
 *
 * @param center  星形中心
 * @param size    外接圆半径
 * @param count   无人机数量
 * @param out     输出位置数组
 * @return 生成的位置数量
 */
int gen_star(Point2f center, float size, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    float R = size;                        // 外圈半径
    float r = R * 0.382f;                  // 内圈半径（黄金比例倒数）
    float start_angle = -90.0f;            // 从正上方开始

    // 计算10个顶点坐标（外顶点 0,2,4,6,8；内顶点 1,3,5,7,9）
    float vert_x[10], vert_y[10];
    for (int i = 0; i < 10; i++) {
        float angle = DEG2RAD(start_angle + i * 36.0f);
        float rad = (i % 2 == 0) ? R : r;
        vert_x[i] = center.x + rad * (float)cos(angle);
        vert_y[i] = center.y + rad * (float)sin(angle);
    }

    // 无人机均匀分配到10条边上
    int per_edge = count / 10;
    int rem = count % 10;
    int edges[10];
    for (int e = 0; e < 10; e++) {
        edges[e] = per_edge + (e < rem ? 1 : 0);
    }

    int idx = 0;
    for (int e = 0; e < 10; e++) {
        int next_e = (e + 1) % 10;
        float x0 = vert_x[e];       float y0 = vert_y[e];
        float x1 = vert_x[next_e];  float y1 = vert_y[next_e];

        for (int i = 0; i < edges[e] && idx < count; i++) {
            float t = (edges[e] == 1) ? 0.0f : (float)i / (edges[e] - 1);
            out[idx].x = x0 + (x1 - x0) * t;
            out[idx].y = y0 + (y1 - y0) * t;
            idx++;
        }
    }

    return count;
}

/**
 * @brief 生成正五边形编队
 * @param center  中心
 * @param size    外接圆半径
 * @param count   无人机数量
 * @param out     输出位置数组
 * @return 生成的位置数量
 */
int gen_pentagon(Point2f center, float size, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    float R = size;
    float start_angle = -90.0f;

    // 5个顶点
    float vx[5], vy[5];
    for (int i = 0; i < 5; i++) {
        float angle = DEG2RAD(start_angle + i * 72.0f);
        vx[i] = center.x + R * (float)cos(angle);
        vy[i] = center.y + R * (float)sin(angle);
    }

    // 均匀分配到5条边
    int per_edge = count / 5;
    int rem = count % 5;
    int edges[5];
    for (int e = 0; e < 5; e++) edges[e] = per_edge + (e < rem ? 1 : 0);

    int idx = 0;
    for (int e = 0; e < 5; e++) {
        int ne = (e + 1) % 5;
        for (int i = 0; i < edges[e] && idx < count; i++) {
            float t = (edges[e] == 1) ? 0.0f : (float)i / (edges[e] - 1);
            out[idx].x = vx[e] + (vx[ne] - vx[e]) * t;
            out[idx].y = vy[e] + (vy[ne] - vy[e]) * t;
            idx++;
        }
    }

    return count;
}

/**
 * @brief 生成正六边形编队
 * @param center  中心
 * @param size    外接圆半径
 * @param count   无人机数量
 * @param out     输出位置数组
 * @return 生成的位置数量
 */
int gen_hexagon(Point2f center, float size, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    float R = size;
    float start_angle = -90.0f;

    float vx[6], vy[6];
    for (int i = 0; i < 6; i++) {
        float angle = DEG2RAD(start_angle + i * 60.0f);
        vx[i] = center.x + R * (float)cos(angle);
        vy[i] = center.y + R * (float)sin(angle);
    }

    int per_edge = count / 6;
    int rem = count % 6;
    int edges[6];
    for (int e = 0; e < 6; e++) edges[e] = per_edge + (e < rem ? 1 : 0);

    int idx = 0;
    for (int e = 0; e < 6; e++) {
        int ne = (e + 1) % 6;
        for (int i = 0; i < edges[e] && idx < count; i++) {
            float t = (edges[e] == 1) ? 0.0f : (float)i / (edges[e] - 1);
            out[idx].x = vx[e] + (vx[ne] - vx[e]) * t;
            out[idx].y = vy[e] + (vy[ne] - vy[e]) * t;
            idx++;
        }
    }

    return count;
}

/**
 * @brief 生成心形编队
 *
 * 使用心形参数方程：
 *   x = 16 * sin³(t)
 *   y = 13*cos(t) - 5*cos(2t) - 2*cos(3t) - cos(4t)
 * 其中 t ∈ [0, 2π]
 *
 * @param center  心形中心
 * @param size    大小缩放
 * @param count   无人机数量
 * @param out     输出位置数组
 * @return 生成的位置数量
 */
int gen_heart(Point2f center, float size, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    float scale = size / 16.0f;  // 归一化因子

    for (int i = 0; i < count; i++) {
        float t = (float)i / count * 2.0f * 3.14159265f;

        float sin_t = (float)sin(t);
        float cos_t = (float)cos(t);

        // 心形参数方程
        float raw_x = 16.0f * sin_t * sin_t * sin_t;
        float raw_y = -(13.0f * cos_t
                      - 5.0f  * (float)cos(2.0 * t)
                      - 2.0f  * (float)cos(3.0 * t)
                      - 1.0f  * (float)cos(4.0 * t));

        out[i].x = center.x + raw_x * scale;
        out[i].y = center.y + raw_y * scale;
    }

    return count;
}

/**
 * @brief 生成螺旋编队（阿基米德螺旋）
 *
 * 极坐标方程：r = a + b * θ
 * 转回笛卡尔坐标：x = r*cos(θ), y = r*sin(θ)
 *
 * @param center  螺旋中心
 * @param size    总展开范围
 * @param count   无人机数量
 * @param out     输出位置数组
 * @return 生成的数量
 */
int gen_spiral(Point2f center, float size, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    int   turns  = 3;                              // 螺旋圈数
    float max_th = turns * 2.0f * 3.14159265f;    // 最大角度
    float a      = 0.0f;                          // 起始半径
    float b      = size / max_th;                  // 半径增长率

    for (int i = 0; i < count; i++) {
        float theta = ((float)i / (count - 1)) * max_th;
        float r     = a + b * theta;

        out[i].x = center.x + r * (float)cos(theta);
        out[i].y = center.y + r * (float)sin(theta);
    }

    return count;
}

/**
 * @brief 生成直线编队（水平排列）
 * @param center  中心
 * @param length  直线长度
 * @param count   无人机数量
 * @param out     输出位置数组
 * @return 生成的数量
 */
int gen_line(Point2f center, float length, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    float half = length / 2.0f;
    float step = (count == 1) ? 0.0f : length / (count - 1);

    for (int i = 0; i < count; i++) {
        out[i].x = center.x - half + step * i;
        out[i].y = center.y;
    }

    return count;
}

/**
 * @brief 生成箭头编队
 *
 * 箭身 + 三角形箭头。结合直线和三角形。
 * 箭头尖端指向右侧。
 *
 * @param center  中心
 * @param size    总长度
 * @param count   数量
 * @param out     输出
 * @return 数量
 */
int gen_arrow(Point2f center, float size, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    float half = size / 2.0f;
    // 箭头三角形顶点在右侧
    float tip_x = center.x + half;

    // 30% 放箭头（三角形），70% 放箭身
    int arrow_count  = (int)(count * 0.3f);
    int shaft_count  = count - arrow_count;
    if (arrow_count < 2) arrow_count = 2;
    if (shaft_count < 1) shaft_count = 1;

    // 生成箭身（水平线，左侧部分）
    float shaft_len = half * 1.2f;
    float shaft_start = center.x - shaft_len / 2.0f;
    float shaft_end   = tip_x - size * 0.3f;  // 给箭头留空间

    int idx = 0;
    for (int i = 0; i < shaft_count && idx < count; i++) {
        float t = (shaft_count == 1) ? 0.5f : (float)i / (shaft_count - 1);
        out[idx].x = shaft_start + (shaft_end - shaft_start) * t;
        out[idx].y = center.y;
        idx++;
    }

    // 生成箭头三角形
    float arr_half = size * 0.3f;
    for (int i = 0; i < arrow_count && idx < count; i++) {
        float t = (arrow_count == 1) ? 0.0f : (float)i / (arrow_count - 1);

        // 三角形填充：从尖端开始，沿两条边分布
        if (t < 0.5f) {
            // 上半边：尖端 → 左下
            float tt = t * 2.0f;
            out[idx].x = tip_x - arr_half * tt;
            out[idx].y = center.y - arr_half * tt;
        } else {
            // 下半边：尖端 → 右下
            float tt = (t - 0.5f) * 2.0f;
            out[idx].x = tip_x - arr_half * tt;
            out[idx].y = center.y + arr_half * tt;
        }
        idx++;
    }

    return count;
}

/**
 * @brief 生成十字编队
 *
 * 竖直和水平两条线在中心交叉。
 *
 * @param center  中心
 * @param size    臂长
 * @param count   数量
 * @param out     输出
 * @return 数量
 */
int gen_cross(Point2f center, float size, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    float half = size / 2.0f;

    // 竖直臂和水平臂各占一半
    int v_count = count / 2;
    int h_count = count - v_count;

    int idx = 0;

    // 竖直臂
    for (int i = 0; i < v_count; i++) {
        float t = (v_count == 1) ? 0.5f : (float)i / (v_count - 1);
        out[idx].x = center.x;
        out[idx].y = center.y - half + size * t;
        idx++;
    }

    // 水平臂
    for (int i = 0; i < h_count; i++) {
        float t = (h_count == 1) ? 0.5f : (float)i / (h_count - 1);
        out[idx].x = center.x - half + size * t;
        out[idx].y = center.y;
        idx++;
    }

    return count;
}

/**
 * @brief 生成弧形编队
 * @param center     中心
 * @param radius     半径
 * @param count      数量
 * @param out        输出
 * @return 数量
 */
int gen_arc(Point2f center, float radius, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    float start_angle = -150.0f;  // 起始角度
    float sweep       = 300.0f;   // 扫过角度（非完整圆）

    for (int i = 0; i < count; i++) {
        float t = (count == 1) ? 0.5f : (float)i / (count - 1);
        float angle = DEG2RAD(start_angle + sweep * t);

        out[i].x = center.x + radius * (float)cos(angle);
        out[i].y = center.y + radius * (float)sin(angle);
    }

    return count;
}

/**
 * @brief 生成网格编队
 *
 * 自动计算最接近的 rows × cols 布局。
 *
 * @param center  网格中心
 * @param size    网格边长
 * @param count   数量
 * @param out     输出
 * @return 数量
 */
int gen_grid(Point2f center, float size, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    // 计算最接近正方形的行列数
    int cols = (int)(sqrt((float)count) + 0.5f);
    if (cols < 1) cols = 1;
    int rows = (count + cols - 1) / cols;

    float spacing_x = size / (cols + 1);
    float spacing_y = size / (rows + 1);
    float start_x = center.x - size / 2.0f + spacing_x;
    float start_y = center.y - size / 2.0f + spacing_y;

    int idx = 0;
    for (int r = 0; r < rows && idx < count; r++) {
        for (int c = 0; c < cols && idx < count; c++) {
            out[idx].x = start_x + c * spacing_x;
            out[idx].y = start_y + r * spacing_y;
            idx++;
        }
    }

    return count;
}

/**
 * @brief 生成随机散布编队
 *
 * 无人机在指定范围内随机分布，通常用作过渡状态。
 *
 * @param center  散布中心
 * @param size    散布范围
 * @param count   数量
 * @param out     输出
 * @return 数量
 */
int gen_random(Point2f center, float size, int count, Point2f out[])
{
    if (count <= 0 || out == NULL) return 0;

    float half = size / 2.0f;

    for (int i = 0; i < count; i++) {
        // 使用简单的伪随机（标准库 rand）
        out[i].x = center.x - half + (float)(rand() % (int)(size));
        out[i].y = center.y - half + (float)(rand() % (int)(size));
    }

    return count;
}

/* ==================== 文字点阵生成器 ==================== */

/**
 * @brief 5×7 点阵字库
 *
 * 每个字符用 7 个字节表示（每行一个字节，bit4..0 对应 5 列像素）。
 * 索引：font_table[ascii - 32]，可打印字符范围 32(空格)~90(Z)。
 */
static const unsigned char g_font_5x7[][7] = {
    // 32 SPACE
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    // 33 !
    {0x04,0x04,0x04,0x04,0x00,0x04,0x00},
    // 34 "
    {0x0A,0x0A,0x0A,0x00,0x00,0x00,0x00},
    // 35 #
    {0x0A,0x0A,0x1F,0x0A,0x1F,0x0A,0x0A},
    // 36 $
    {0x04,0x0F,0x14,0x0E,0x05,0x1E,0x04},
    // 37 %
    {0x18,0x19,0x02,0x04,0x08,0x13,0x03},
    // 38 &
    {0x0C,0x12,0x14,0x08,0x15,0x12,0x0D},
    // 39 '
    {0x04,0x04,0x00,0x00,0x00,0x00,0x00},
    // 40 (
    {0x02,0x04,0x08,0x08,0x08,0x04,0x02},
    // 41 )
    {0x08,0x04,0x02,0x02,0x02,0x04,0x08},
    // 42 *
    {0x00,0x04,0x15,0x0E,0x15,0x04,0x00},
    // 43 +
    {0x00,0x04,0x04,0x1F,0x04,0x04,0x00},
    // 44 ,
    {0x00,0x00,0x00,0x00,0x04,0x04,0x08},
    // 45 -
    {0x00,0x00,0x00,0x1F,0x00,0x00,0x00},
    // 46 .
    {0x00,0x00,0x00,0x00,0x00,0x04,0x00},
    // 47 /
    {0x01,0x02,0x02,0x04,0x08,0x08,0x10},
    // 48 0
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},
    // 49 1
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},
    // 50 2
    {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F},
    // 51 3
    {0x0E,0x11,0x01,0x06,0x01,0x11,0x0E},
    // 52 4
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},
    // 53 5
    {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
    // 54 6
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},
    // 55 7
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08},
    // 56 8
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
    // 57 9
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C},
    // 58 :
    {0x00,0x04,0x00,0x00,0x04,0x00,0x00},
    // 59 ;
    {0x00,0x04,0x00,0x00,0x04,0x04,0x08},
    // 60 <
    {0x02,0x04,0x08,0x10,0x08,0x04,0x02},
    // 61 =
    {0x00,0x00,0x1F,0x00,0x1F,0x00,0x00},
    // 62 >
    {0x08,0x04,0x02,0x01,0x02,0x04,0x08},
    // 63 ?
    {0x0E,0x11,0x01,0x02,0x04,0x00,0x04},
    // 64 @
    {0x0E,0x11,0x17,0x15,0x17,0x10,0x0F},
    // 65 A
    {0x04,0x0A,0x11,0x11,0x1F,0x11,0x11},
    // 66 B
    {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},
    // 67 C
    {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    // 68 D
    {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E},
    // 69 E
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
    // 70 F
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
    // 71 G
    {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F},
    // 72 H
    {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},
    // 73 I
    {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
    // 74 J
    {0x07,0x02,0x02,0x02,0x02,0x12,0x0C},
    // 75 K
    {0x11,0x12,0x14,0x18,0x14,0x12,0x11},
    // 76 L
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    // 77 M
    {0x11,0x1B,0x15,0x15,0x11,0x11,0x11},
    // 78 N
    {0x11,0x11,0x19,0x15,0x13,0x11,0x11},
    // 79 O
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    // 80 P
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},
    // 81 Q
    {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},
    // 82 R
    {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    // 83 S
    {0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E},
    // 84 T
    {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},
    // 85 U
    {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},
    // 86 V
    {0x11,0x11,0x11,0x11,0x0A,0x0A,0x04},
    // 87 W
    {0x11,0x11,0x11,0x15,0x15,0x15,0x0A},
    // 88 X
    {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
    // 89 Y
    {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},
    // 90 Z
    {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},
};
#define FONT_TABLE_START 32   /* ASCII 空格 */
#define FONT_TABLE_END   90   /* ASCII Z */

/**
 * @brief 生成文字编队（高密度像素簇版）
 *
 * 每个"亮"像素点不是放一架无人机，而是放一个 density×density 的
 * 像素簇（默认 3×3 = 9 架无人机），让文字看起来像密集的连线。
 * 相邻像素的簇会自然重叠融合。
 *
 * @param center    文字区域中心
 * @param char_size  像素间距（字符格）
 * @param text       要显示的文字
 * @param count      可用无人机总数
 * @param out        输出位置数组
 * @return 实际使用的无人机数量
 */
int gen_text(Point2f center, float char_size, const char* text,
             int count, Point2f out[])
{
    if (text == NULL || count <= 0 || out == NULL) return 0;

    int text_len = (int)strlen(text);
    if (text_len == 0) return 0;

    // ── 先统计总像素数，决定簇密度 ──
    // 密度自适应：可用无人机越多，簇越密
    int total_pixels = 0;
    for (int ci = 0; ci < text_len; ci++) {
        unsigned char ascii = (unsigned char)text[ci];
        if (ascii >= 'a' && ascii <= 'z') ascii -= 32;
        if (ascii < FONT_TABLE_START || ascii > FONT_TABLE_END) continue;
        const unsigned char* g = g_font_5x7[ascii - FONT_TABLE_START];
        for (int r = 0; r < 7; r++) {
            unsigned char b = g[r];
            for (int c = 0; c < 5; c++) {
                if (b & (1 << (4 - c))) total_pixels++;
            }
        }
    }
    if (total_pixels == 0) return 0;  // 没有可渲染字符（如全中文）

    // 根据可用无人机数量选择密度
    // density=3 → 9架/像素 (超密)  density=2 → 4架/像素 (适中)
    // density=1 → 1架/像素 (骨架)
    int density;
    if (count >= total_pixels * 12)      density = 3;
    else if (count >= total_pixels * 4)  density = 2;
    else                                 density = 1;

    // ── 布局参数 ──
    float char_width = 6.0f * char_size;
    float total_w = char_width * text_len - char_size;
    float start_x = center.x - total_w / 2.0f;
    float start_y = center.y - 3.0f * char_size;
    float spacing = char_size / (float)density;

    int idx = 0;

    for (int ci = 0; ci < text_len && idx < count; ci++) {
        unsigned char ascii = (unsigned char)text[ci];
        if (ascii >= 'a' && ascii <= 'z') ascii -= 32;
        if (ascii < FONT_TABLE_START || ascii > FONT_TABLE_END) continue;

        const unsigned char* glyph = g_font_5x7[ascii - FONT_TABLE_START];

        // 遍历 7×5 点阵
        for (int row = 0; row < 7 && idx < count; row++) {
            unsigned char bits = glyph[row];
            for (int col = 0; col < 5 && idx < count; col++) {
                if (!(bits & (1 << (4 - col)))) continue;

                // 像素中心坐标
                float cx = start_x + ci * char_width + col * char_size;
                float cy = start_y + row * char_size;

                // 在该像素周围放置 density×density 簇
                for (int dr = 0; dr < density && idx < count; dr++) {
                    for (int dc = 0; dc < density && idx < count; dc++) {
                        out[idx].x = cx + (dc - (density - 1) / 2.0f) * spacing;
                        out[idx].y = cy + (dr - (density - 1) / 2.0f) * spacing;
                        idx++;
                    }
                }
            }
        }
    }

    return idx;
}

/* ==================== 统一调度接口 ==================== */

int pattern_generate(PatternType type, Point2f center,
                     float scale, float rotation_deg,
                     int drone_count, Point2f out_positions[],
                     const char* text)
{
    if (drone_count <= 0 || drone_count > MAX_DRONE_COUNT
        || out_positions == NULL) {
        return 0;
    }

    int generated = 0;

    // 根据类型调用对应生成器
    switch (type) {
    case PAT_CIRCLE:    generated = gen_circle(center, scale, drone_count, out_positions);   break;
    case PAT_SQUARE:    generated = gen_square(center, scale, drone_count, out_positions);   break;
    case PAT_TRIANGLE:  generated = gen_triangle(center, scale, drone_count, out_positions); break;
    case PAT_DIAMOND:   generated = gen_diamond(center, scale, drone_count, out_positions);  break;
    case PAT_STAR:      generated = gen_star(center, scale, drone_count, out_positions);     break;
    case PAT_PENTAGON:  generated = gen_pentagon(center, scale, drone_count, out_positions); break;
    case PAT_HEXAGON:   generated = gen_hexagon(center, scale, drone_count, out_positions);  break;
    case PAT_HEART:     generated = gen_heart(center, scale, drone_count, out_positions);    break;
    case PAT_SPIRAL:    generated = gen_spiral(center, scale, drone_count, out_positions);   break;
    case PAT_LINE:      generated = gen_line(center, scale, drone_count, out_positions);     break;
    case PAT_ARROW:     generated = gen_arrow(center, scale, drone_count, out_positions);    break;
    case PAT_CROSS:     generated = gen_cross(center, scale, drone_count, out_positions);    break;
    case PAT_ARC:       generated = gen_arc(center, scale, drone_count, out_positions);      break;
    case PAT_GRID:      generated = gen_grid(center, scale, drone_count, out_positions);     break;
    case PAT_RANDOM:    generated = gen_random(center, scale, drone_count, out_positions);   break;
    case PAT_TEXT:      generated = gen_text(center, scale / 3.0f,
                                         text ? text : "HI",
                                         drone_count, out_positions); break;
    default:
        // 未知图案类型 → 返回0
        generated = 0;
        break;
    }

    // 如果指定了旋转角度（非零），对所有输出坐标进行旋转
    if (rotation_deg != 0.0f && generated > 0) {
        for (int i = 0; i < generated; i++) {
            float rx, ry;
            rotate_point(center.x, center.y,
                         out_positions[i].x, out_positions[i].y,
                         rotation_deg, &rx, &ry);
            out_positions[i].x = rx;
            out_positions[i].y = ry;
        }
    }

    return generated;
}

/* ==================== 编队便捷函数 ==================== */

/**
 * @brief 获取图案的推荐无人机数量和缩放值
 *
 * 不同图案的最优呈现需要不同数量：
 *   - 圆形/方形等边框图案：稍少无人机避免拥挤
 *   - 星形/心形：稍多无人机使轮廓清晰
 *   - 文字：根据文本长度动态计算
 */
void pattern_recommend(PatternType type, int text_len,
                       int* out_count, float* out_scale)
{
    switch (type) {
    case PAT_CIRCLE:    *out_count = 30;  *out_scale = 18.0f; break;
    case PAT_SQUARE:    *out_count = 28;  *out_scale = 22.0f; break;
    case PAT_TRIANGLE:  *out_count = 27;  *out_scale = 22.0f; break;
    case PAT_DIAMOND:   *out_count = 24;  *out_scale = 20.0f; break;
    case PAT_STAR:      *out_count = 40;  *out_scale = 18.0f; break;
    case PAT_PENTAGON:  *out_count = 25;  *out_scale = 18.0f; break;
    case PAT_HEXAGON:   *out_count = 30;  *out_scale = 18.0f; break;
    case PAT_HEART:     *out_count = 35;  *out_scale = 18.0f; break;
    case PAT_SPIRAL:    *out_count = 40;  *out_scale = 16.0f; break;
    case PAT_LINE:      *out_count = 20;  *out_scale = 35.0f; break;
    case PAT_ARROW:     *out_count = 25;  *out_scale = 22.0f; break;
    case PAT_CROSS:     *out_count = 20;  *out_scale = 22.0f; break;
    case PAT_ARC:       *out_count = 25;  *out_scale = 18.0f; break;
    case PAT_GRID:      *out_count = 25;  *out_scale = 22.0f; break;
    case PAT_RANDOM:    *out_count = 30;  *out_scale = 25.0f; break;
    case PAT_TEXT:
        // 统一简约风格：每像素1架无人机（骨架点阵），不管几个字都一样
        if (text_len <= 0) text_len = 3;
        *out_count = text_len * 20;   // 每字≈20像素，密度1
        if (*out_count > 100) *out_count = 100;
        if (*out_count < 20)  *out_count = 20;
        *out_scale = 6.0f;   // char_size=2 整数，字形均匀
        break;
    default:
        *out_count = 20;  *out_scale = 15.0f; break;
    }
}

Formation* formation_create(const char* name, PatternType type,
                            Point2f center, float scale, float rotation_deg,
                            int drone_count, const char* display_text)
{
    if (drone_count <= 0 || drone_count > MAX_DRONE_COUNT) return NULL;

    // 分配编队结构体
    Formation* f = (Formation*)malloc(sizeof(Formation));
    if (f == NULL) return NULL;

    // 复制名称
    strncpy(f->name, name, sizeof(f->name) - 1);
    f->name[sizeof(f->name) - 1] = '\0';

    // 复制文字（用于 PAT_TEXT）
    f->display_text[0] = '\0';
    if (display_text != NULL) {
        strncpy(f->display_text, display_text, sizeof(f->display_text) - 1);
        f->display_text[sizeof(f->display_text) - 1] = '\0';
    }

    // 填充参数
    f->formation_id = 0;
    f->pattern      = type;
    f->center       = center;
    f->scale        = scale;
    f->rotation_deg = rotation_deg;
    // 调用生成器计算目标位置
    int generated = pattern_generate(type, center, scale, rotation_deg,
                                     drone_count, f->targets,
                                     f->display_text);

    // 实际使用的无人机数量（可能少于传入的 drone_count）
    f->drone_count = generated;

    if (generated == 0) {
        free(f);
        return NULL;
    }

    return f;
}

void formation_destroy(Formation* formation)
{
    if (formation != NULL) {
        free(formation);
    }
}

void formation_update(Formation* formation, PatternType type,
                      Point2f center, float scale, float rotation_deg)
{
    if (formation == NULL) return;

    formation->pattern      = type;
    formation->center       = center;
    formation->scale        = scale;
    formation->rotation_deg = rotation_deg;

    // 重新生成目标位置
    pattern_generate(type, center, scale, rotation_deg,
                     formation->drone_count, formation->targets,
                     formation->display_text);
}

int formation_get_target(const Formation* formation, int index,
                         float* out_x, float* out_y)
{
    if (formation == NULL || out_x == NULL || out_y == NULL) return 0;
    if (index < 0 || index >= formation->drone_count) return 0;

    *out_x = formation->targets[index].x;
    *out_y = formation->targets[index].y;
    return 1;
}
