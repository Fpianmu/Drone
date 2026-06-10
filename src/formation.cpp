/*
 * formation.cpp —— 编队与图案生成模块实现
 *
 * 包含 15 种几何图案的生成算法，以及用 GDI 渲染文字的点阵编队生成器。
 *
 * 图案生成算法一览：
 *   圆形     —— 等角度分布在圆周上（参数方程）
 *   正方形   —— 四边等距排列
 *   三角形   —— 三边等距排列
 *   菱形     —— 四边等距排列
 *   五角星   —— 10 个顶点（外圈 5 个 + 内圈 5 个），黄金比例 0.382
 *   五边/六边  —— 等距分布在边上
 *   心形     —— 心形参数方程
 *   螺旋线   —— 阿基米德螺旋 r = aθ
 *   直线     —— 水平等距
 *   箭头     —— 箭身 + 三角形箭头
 *   十字     —— 横竖两条
 *   弧形     —— 等角度分布在弧上
 *   网格     —— rows × cols 矩形
 *   随机     —— 伪随机散布
 *   文字     —— Windows GDI 渲染 → 降采样为二值点阵
 */

#include "../include/formation.h"

/*
 * 绕中心点旋转坐标（二维旋转矩阵）
 *
 * 公式：x' = cx + (x-cx)·cosθ - (y-cy)·sinθ
 *       y' = cy + (x-cx)·sinθ + (y-cy)·cosθ
 * 所有图案生成完后都会经过这个函数做旋转。
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

/*
 * 图案生成器 —— 每种图案一个函数，输入几何参数 + 飞机数量，输出坐标数组
 *
 * 所有生成器遵循相同的调用约定：
 *   center: 图案中心
 *   size:   含义因图案而异（圆=半径，正方=边长……）
 *   count:  飞机数量
 *   out:    输出坐标数组，调用者保证长度 ≥ count
 *   返回:   实际填充的坐标数（通常 = count）
 */

/*
 * 圆形：等角度分布在圆周上
 * 圆的参数方程 x = cx + R·cosθ, y = cy + R·sinθ
 * 角度步长 = 360° / count，保证均匀分布
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

/*
 * 正方形：四边等距分布
 * 周长 = 4 × side，步长 = 周长 ÷ count
 * 按距离确定在四条边中的哪一条（上 → 右 → 下 → 左）
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

/*
 * 正三角形：三边等距分布，顶点在上
 * 重心在 center，高 = side × √3/2
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

/*
 * 菱形：四条边等距分布，水平对角线 = size，垂直 = size×0.7
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

/*
 * 五角星：10 顶点（外 5 + 内 5 交替），内圈半径 ≈ 外圈 × 0.382（黄金比例）
 * 无人机分配到 10 条边上
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

/*
 * 正五边形：5 顶点，72° 间隔
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

/*
 * 正六边形：6 顶点，60° 间隔
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

/*
 * 心形：参数方程
 * x = 16sin³t, y = 13cost - 5cos2t - 2cos3t - cos4t
 * t ∈ [0, 2π]，等距采样 count 个点
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

/*
 * 阿基米德螺旋：极坐标 r = a + bθ, 笛卡尔 x = r·cosθ, y = r·sinθ
 * 3 圈展开，半径从 0 线性增长到 size
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

/*
 * 直线：水平等距排列，长度 = size
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

/*
 * 箭头：箭身（直线，70% 飞机） + 三角箭头（30% 飞机），尖端朝右
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

/*
 * 十字：竖线 + 横线在中心交叉，各占一半飞机
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

/*
 * 弧形：半个圆多一点（-150° 到 +150°），等角度分布
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

/*
 * 网格：自动计算最接近正方形的 rows × cols 布局
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

/*
 * 随机散布：在 size×size 范围内伪随机分布
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

/* ==================== 文字编队生成器（GDI 渲染） ==================== */

/*
 * 文字编队生成器 —— 逐字渲染为固定点阵（和旧版英文思路一致）
 *
 * 中文字符渲染到 16x16 px 小位图，ASCII 渲染到 9x16 px，
 * 逐像素映射为无人机坐标。不用降采样、自适应网格。
 * char_size 决定像素点在舞台上的间距（格子大小）。
 *
 * 每个"亮"像素 → 一架无人机，紧凑排列。
 */
int gen_text(Point2f center, float char_size, const char* text,
             int count, Point2f out[])
{
    if (text == NULL || count <= 0 || out == NULL) return 0;
    if (strlen(text) == 0) return 0;

    // UTF-8 转宽字符
    int wlen = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    if (wlen <= 1) return 0;
    WCHAR* wtext = (WCHAR*)malloc(wlen * sizeof(WCHAR));
    if (wtext == NULL) return 0;
    MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext, wlen);
    int wchars = wlen - 1;

    // 创建 GDI 环境
    HDC hScrDC = GetDC(NULL);
    HDC hDC    = CreateCompatibleDC(hScrDC);

    // 像素格：中文 16x16，英文 9x16
    int cellPX  = 16;
    int wideW   = 16;
    int narrowW = 9;

    // 无抗锯齿字体 → 清晰的二值像素
    HFONT hFont = CreateFontW(cellPX, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        NONANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"SimHei");
    if (hFont == NULL)
        hFont = CreateFontW(cellPX, 0, 0, 0, FW_NORMAL,
            FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            NONANTIALIASED_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
    SelectObject(hDC, hFont);
    SetBkColor(hDC, RGB(0,0,0));
    SetTextColor(hDC, RGB(255,255,255));
    SetBkMode(hDC, OPAQUE);

    // 小位图（只画一个字）
    HBITMAP hBmp = CreateCompatibleBitmap(hScrDC, 20, 20);
    SelectObject(hDC, hBmp);

    // 计算总宽度（字符格数），用于居中
    int total_cols = 0;
    for (int i = 0; i < wchars; i++)
        total_cols += (wtext[i] < 0x80) ? narrowW + 1 : wideW + 1;

    float start_x = center.x - (total_cols - 1) * char_size / 2.0f;
    float start_y = center.y - (cellPX - 1) * char_size / 2.0f;
    float cur_x   = start_x;
    int   idx     = 0;

    // 逐字渲染到位图，逐像素读取
    for (int ci = 0; ci < wchars && idx < count; ci++) {
        int cw = (wtext[ci] < 0x80) ? narrowW : wideW;

        // 清空位图
        RECT rc = {0, 0, 20, 20};
        FillRect(hDC, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

        // 渲染单个字
        TextOutW(hDC, 0, 0, &wtext[ci], 1);

        // 逐像素：白像素 → 放置无人机
        for (int py = 0; py < cellPX && idx < count; py++) {
            for (int px = 0; px < cw && idx < count; px++) {
                COLORREF c = GetPixel(hDC, px, py);
                if (GetRValue(c) > 80) {
                    out[idx].x = cur_x + px * char_size;
                    out[idx].y = start_y + py * char_size;
                    idx++;
                }
            }
        }

        cur_x += (cw + 1) * char_size;  // +1 字符间距
    }

    // 清理
    DeleteObject(hBmp);
    DeleteObject(hFont);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScrDC);
    free(wtext);

    return idx;
}

/* ==================== BMP 图片编队 ==================== */

/*
 * 读取 24/32 位 BMP 文件，转灰度 → 自适应阈值二值化 → 降采样
 * 亮像素放置无人机。纯手工解析 BMP 文件头，不调任何图像库。
 */
int gen_image(Point2f center, float char_size, const char* filename,
              int count, Point2f out[])
{
    if (filename == NULL || count <= 0 || out == NULL) return 0;

    // 打开文件
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) return 0;

    // 读取 BITMAPFILEHEADER（14 字节）
    unsigned char bfType[2] = {0,0};
    unsigned int  bfSize = 0, bfOffBits = 0;
    if (fread(bfType, 2, 1, fp) != 1) { fclose(fp); return 0; }
    if (bfType[0] != 'B' || bfType[1] != 'M') { fclose(fp); return 0; }
    fread(&bfSize,    4, 1, fp);
    fseek(fp, 4, SEEK_CUR);
    fread(&bfOffBits, 4, 1, fp);

    // 读取 BITMAPINFOHEADER
    unsigned int biWidth = 0, biHeight = 0;
    unsigned short biBitCount = 0;
    fseek(fp, 4, SEEK_CUR);
    fread(&biWidth,  4, 1, fp);
    fread(&biHeight, 4, 1, fp);
    fseek(fp, 2, SEEK_CUR);
    fread(&biBitCount, 2, 1, fp);
    if (biBitCount != 24 && biBitCount != 32) { fclose(fp); return 0; }

    // 跳到像素数据
    fseek(fp, bfOffBits, SEEK_SET);

    int rowSize = ((biWidth * (biBitCount / 8) + 3) / 4) * 4;  // 对齐到4字节
    int bpp     = biBitCount / 8;   // 每像素字节数

    // 输出网格直接用舞台尺寸（40行 × 比例），保证填满表演区
    float aspect = (float)biWidth / (float)biHeight;
    int outH = STAGE_ROWS - 4;    // 36 行，充分利用舞台高度
    int outW = (int)(outH * aspect);
    if (outW < 1) outW = 1;
    if (outW > STAGE_COLS - 4) { outW = STAGE_COLS - 4; outH = (int)(outW / aspect); }

    float cellW = (float)biWidth  / outW;
    float cellH = (float)biHeight / outH;

    // 读入像素数据并生成灰度阵列
    int* cellSum = (int*)calloc(outW * outH, sizeof(int));
    int* cellCnt = (int*)calloc(outW * outH, sizeof(int));
    unsigned char* rowBuf = (unsigned char*)malloc(rowSize);
    if (cellSum == NULL || cellCnt == NULL || rowBuf == NULL) {
        free(cellSum); free(cellCnt); free(rowBuf); fclose(fp); return 0;
    }

    for (unsigned int y = 0; y < biHeight; y++) {
        fread(rowBuf, rowSize, 1, fp);
        int gy = (int)(y * cellH);
        if (gy >= outH) gy = outH - 1;
        for (unsigned int x = 0; x < biWidth; x++) {
            int gx = (int)(x * cellW);
            if (gx >= outW) gx = outW - 1;
            unsigned char* p = &rowBuf[x * bpp];
            int gray = (p[2] + p[1] + p[0]) / 3;  // BGR → 灰度
            cellSum[gy * outW + gx] += gray;
            cellCnt[gy * outW + gx]++;
        }
    }
    free(rowBuf);
    fclose(fp);

    // 阈值 128：暗像素（<128）放无人机，亮像素（≥128）忽略
    // PCtoLCD2002 默认逻辑：白底黑画 → 无人机勾勒暗色轮廓

    // 生成无人机位置（填满舞台）
    float cell_size = (float)(STAGE_COLS - 4) / outW;
    float cell_h    = (float)(STAGE_ROWS - 4) / outH;
    if (cell_h < cell_size) cell_size = cell_h;
    if (cell_size < 1.0f) cell_size = 1.0f;

    float total_w = outW * cell_size;
    float total_h = outH * cell_size;
    float start_x = center.x - total_w / 2.0f;
    float start_y = center.y - total_h / 2.0f;

    int idx = 0;
    for (int gy = 0; gy < outH && idx < count; gy++) {
        for (int gx = 0; gx < outW && idx < count; gx++) {
            int ci = gy * outW + gx;
            // 暗像素 → 放无人机（和 PCtoLCD2002 一致：白底黑画取暗点）
            if (cellCnt[ci] > 0 && (cellSum[ci] / cellCnt[ci]) < 128) {
                out[idx].x = start_x + gx * cell_size + cell_size / 2.0f;
                out[idx].y = start_y + gy * cell_size + cell_size / 2.0f;
                idx++;
            }
        }
    }

    // 调试：把结果写到文件
    {
        FILE* dbg = fopen("bmp_debug.txt", "w");
        if (dbg) {
            fprintf(dbg, "文件: %s\n尺寸: %ux%u, %u位\n",
                    filename, biWidth, biHeight, biBitCount);
            fprintf(dbg, "输出网格: %dx%d, 阈值<128, 生成%d架无人机\n",
                    outW, outH, idx);
            fclose(dbg);
        }
    }

    free(cellSum);
    free(cellCnt);
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
    case PAT_IMAGE:     generated = gen_image(center, scale / 3.0f,
                                         text ? text : "sample.bmp",
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

/*
 * 获取图案的推荐无人机数量和缩放值
 * 不同图案最优呈现需要的无人机数不同（边框图案少、星形心形多、文字动态计算）
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
        // GDI 渲染：char_size=输出像素间距（1~2格较合适）
        if (text_len <= 0) text_len = 3;
        *out_count = text_len * 120;   // 每字约120架（大字体+密集采样）
        if (*out_count > 280) *out_count = 280;
        if (*out_count < 80)  *out_count = 80;
        *out_scale = 3.0f;   // 仅作后备，实际格子数由 count 决定
        break;
    case PAT_IMAGE:
        *out_count = 200;  *out_scale = 3.0f;  break;
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
