/*
 * graphics.cpp —— 控制台渲染模块实现
 *
 * 核心思路：在内存中维护一个 CHAR_INFO[45][120] 帧缓冲，
 * 所有绘制操作只改这个数组，不碰控制台。
 * 每帧结束时用 WriteConsoleOutputW 一次性写入，彻底消除闪烁。
 *
 * 为什么不用 printf：printf 写 stdout，指向原始控制台缓冲区，
 * 和我们的帧缓冲不同步，会造成残影和乱码。
 */

#include "../include/graphics.h"
#include <stdarg.h>

/* ==================== 帧缓冲 ==================== */

/** @brief 单个字符单元（Unicode 字符 + 颜色属性） */
typedef struct {
    WCHAR ch;     // 宽字符
    WORD  attr;   // 颜色属性
} Cell;

/** @brief 帧缓冲 —— 整个屏幕的内存映像 */
static Cell g_fb[CONSOLE_HEIGHT][CONSOLE_WIDTH];

/** @brief 控制台输出句柄（单缓冲，不交换） */
static HANDLE g_hOut = NULL;

/* ==================== 内部辅助 ==================== */

/**
 * @brief 清空帧缓冲（空格 + 白字黑底）
 */
static void fb_clear(void)
{
    for (int row = 0; row < CONSOLE_HEIGHT; row++) {
        for (int col = 0; col < CONSOLE_WIDTH; col++) {
            g_fb[row][col].ch   = L' ';
            g_fb[row][col].attr = CON_WHITE;
        }
    }
}

/**
 * @brief 在帧缓冲中放置一个 UTF-8 字符（自动转 WCHAR）
 *
 * 注意：简易实现假定输入是 1-3 字节 UTF-8（● ┌─┐ 等在 3 字节内），
 * 多字节字符需逐字节解析。为了可靠，改用已知的 WCHAR 直接传入。
 */
static void fb_put_wchar(int col, int row, ConsoleColor color, WCHAR wc)
{
    if (col < 0 || col >= CONSOLE_WIDTH)  return;
    if (row < 0 || row >= CONSOLE_HEIGHT) return;

    g_fb[row][col].ch   = wc;
    g_fb[row][col].attr = color;
}

/**
 * @brief 在帧缓冲中放置一个 UTF-8 字符串
 *
 * 逐个字符写入，非 ASCII 字符暂时用 * 代替（稳健做法）。
 * 对于 ● ┌─┐ 等特殊字符，使用 fb_put_wchar 直接传入 WCHAR。
 */
static void fb_puts(int col, int row, ConsoleColor color, const char* str)
{
    if (str == NULL) return;
    int x = col;
    while (*str && x < CONSOLE_WIDTH) {
        unsigned char c = (unsigned char)*str;

        if (c < 0x80) {
            // ASCII 单字节
            WCHAR wc;
            MultiByteToWideChar(CP_UTF8, 0, str, 1, &wc, 1);
            fb_put_wchar(x, row, color, wc ? wc : L' ');
            str += 1;
            x += 1;
        } else if (c >= 0xC0 && c < 0xE0) {
            // 双字节 UTF-8
            WCHAR wc;
            MultiByteToWideChar(CP_UTF8, 0, str, 2, &wc, 1);
            fb_put_wchar(x, row, color, wc ? wc : L'?');
            str += 2;
            x += 1;
        } else if (c >= 0xE0 && c < 0xF0) {
            // 三字节 UTF-8 (● 等)
            WCHAR wc;
            MultiByteToWideChar(CP_UTF8, 0, str, 3, &wc, 1);
            fb_put_wchar(x, row, color, wc ? wc : L'?');
            str += 3;
            x += 1;
        } else if (c >= 0xF0) {
            // 四字节 UTF-8（极少使用）
            WCHAR wc;
            MultiByteToWideChar(CP_UTF8, 0, str, 4, &wc, 1);
            fb_put_wchar(x, row, color, wc ? wc : L'?');
            str += 4;
            x += 1;
        } else {
            // 无效字节，跳过
            str++;
            x++;
        }
    }
}

/**
 * @brief 在帧缓冲中格式化输出字符串
 */
static void fb_printf(int col, int row, ConsoleColor color,
                       const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    fb_puts(col, row, color, buf);
}

/** @brief 上一次已知的缓冲区宽度（用于检测缩放） */
static SHORT g_prevBufW = 0;
static SHORT g_prevBufH = 0;

/**
 * @brief 将帧缓冲写入控制台
 *
 * 只在检测到控制台尺寸变化时才清空全缓冲区，平时直接覆盖写入120×45区域。
 * 这样避免每帧清屏→写内容的两步闪烁。
 */
static void fb_flush(void)
{
    if (g_hOut == NULL) return;

    // 检测缓冲区尺寸是否改变（用户缩放窗口）
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(g_hOut, &csbi)) {
        if (csbi.dwSize.X != g_prevBufW || csbi.dwSize.Y != g_prevBufH) {
            // 尺寸变了 → 清空整个缓冲区，消除乱码残留
            g_prevBufW = csbi.dwSize.X;
            g_prevBufH = csbi.dwSize.Y;
            DWORD total = (DWORD)csbi.dwSize.X * csbi.dwSize.Y;
            DWORD done  = 0;
            COORD zero  = { 0, 0 };
            FillConsoleOutputCharacterW(g_hOut, L' ', total, zero, &done);
            FillConsoleOutputAttribute(g_hOut, CON_WHITE, total, zero, &done);
        }
    }

    // 写入120×45帧缓冲（直接覆盖，无闪烁）
    SMALL_RECT region = { 0, 0, CONSOLE_WIDTH - 1, CONSOLE_HEIGHT - 1 };
    COORD bufSize = { CONSOLE_WIDTH, CONSOLE_HEIGHT };
    COORD bufPos  = { 0, 0 };

    WriteConsoleOutputW(g_hOut, (const CHAR_INFO*)g_fb,
                        bufSize, bufPos, &region);
}

/* ==================== 窗口管理 ==================== */

void graphics_init(void)
{
    g_hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // UTF-8 代码页
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    SetConsoleTitleW(L"无人机编队灯光秀模拟系统");

    // 设置缓冲区/窗口尺寸
    COORD sz = { CONSOLE_WIDTH, CONSOLE_HEIGHT };
    SetConsoleScreenBufferSize(g_hOut, sz);

    SMALL_RECT rect = { 0, 0, CONSOLE_WIDTH - 1, CONSOLE_HEIGHT - 1 };
    SetConsoleWindowInfo(g_hOut, TRUE, &rect);

    // 隐藏光标
    CONSOLE_CURSOR_INFO ci = { 1, FALSE };
    SetConsoleCursorInfo(g_hOut, &ci);

    // 锁定控制台窗口尺寸
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL) {
        // 移除可缩放边框和最大化按钮
        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
        SetWindowLongPtr(hwnd, GWL_STYLE, style);

        // 删除系统菜单的缩放/最大化项
        HMENU hmenu = GetSystemMenu(hwnd, FALSE);
        if (hmenu != NULL) {
            DeleteMenu(hmenu, SC_SIZE,     MF_BYCOMMAND);
            DeleteMenu(hmenu, SC_MAXIMIZE, MF_BYCOMMAND);
        }

        // 强制刷新窗口框架
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_FRAMECHANGED | SWP_DRAWFRAME);
    }

    // 设置缓冲区与窗口完全一致，消除滚动条
    COORD bufSz = { CONSOLE_WIDTH, CONSOLE_HEIGHT };
    SetConsoleScreenBufferSize(g_hOut, bufSz);
    SMALL_RECT winRect = { 0, 0, CONSOLE_WIDTH - 1, CONSOLE_HEIGHT - 1 };
    SetConsoleWindowInfo(g_hOut, TRUE, &winRect);

    // 清空帧缓冲并写入
    fb_clear();
    fb_flush();
}

void graphics_close(void)
{
    CONSOLE_CURSOR_INFO ci = { 1, TRUE };
    if (g_hOut) SetConsoleCursorInfo(g_hOut, &ci);

    // 定位到底部
    COORD bot = { 0, CONSOLE_HEIGHT - 1 };
    if (g_hOut) {
        SetConsoleCursorPosition(g_hOut, bot);
        printf("\n");
    }
}

void graphics_clear(void)
{
    fb_clear();
}

void graphics_flush(void)
{
    fb_flush();
}

/* ==================== 绘制元素 ==================== */

void graphics_draw_stage(const SafetyZone* zone)
{
    if (zone == NULL) return;

    int x0 = STAGE_LEFT;
    int y0 = STAGE_TOP;
    int x1 = STAGE_LEFT + STAGE_COLS - 1;
    int y1 = STAGE_TOP  + STAGE_ROWS - 1;

    // 双线外框 - 四个角
    fb_put_wchar(x0, y0, CON_CYAN, L'\x2554');   // ╔
    fb_put_wchar(x1, y0, CON_CYAN, L'\x2557');   // ╗
    fb_put_wchar(x0, y1, CON_CYAN, L'\x255A');   // ╚
    fb_put_wchar(x1, y1, CON_CYAN, L'\x255D');   // ╝

    // 双线横线
    for (int x = x0 + 1; x < x1; x++) {
        fb_put_wchar(x, y0, CON_CYAN, L'\x2550');  // ═
        fb_put_wchar(x, y1, CON_CYAN, L'\x2550');
    }

    // 双线竖线
    for (int y = y0 + 1; y < y1; y++) {
        fb_put_wchar(x0, y, CON_CYAN, L'\x2551');  // ║
        fb_put_wchar(x1, y, CON_CYAN, L'\x2551');
    }

    // 舞台标题（左上角）
    fb_puts(x0 + 3, y0, CON_YELLOW, " 表演区域 ");

    // 坐标刻度标记（上下边框每10格一个刻度）
    for (int x = x0 + 10; x < x1; x += 10) {
        fb_put_wchar(x, y0, CON_CYAN, L'\x2566');  // ╦
        fb_put_wchar(x, y1, CON_CYAN, L'\x2569');  // ╩
    }
    // 左右边框刻度
    for (int y = y0 + 10; y < y1; y += 10) {
        fb_put_wchar(x0, y, CON_CYAN, L'\x2560');  // ╠
        fb_put_wchar(x1, y, CON_CYAN, L'\x2563');  // ╣
    }

    // 舞台内部画淡色网格点（每5格一个·），帮助定位
    for (int y = y0 + 5; y < y1; y += 5) {
        for (int x = x0 + 5; x < x1; x += 5) {
            fb_put_wchar(x, y, CON_GRAY, L'\x00B7');  // ·
        }
    }

    // 中心十字参考线（淡色）
    int cx = x0 + STAGE_COLS / 2;
    int cy = y0 + STAGE_ROWS / 2;
    for (int x = x0 + 1; x < x1; x++) {
        if (x % 10 == 0) continue; // 不覆盖网格点
        fb_put_wchar(x, cy, CON_GRAY, L'\x2504');  // ┄ 细横线
    }
    for (int y = y0 + 1; y < y1; y++) {
        if (y % 10 == 0) continue;
        fb_put_wchar(cx, y, CON_GRAY, L'\x2506');  // ┆ 细竖线
    }
}

void graphics_draw_drone(const Drone* drone, ConsoleColor color)
{
    if (drone == NULL || !drone->is_active) return;

    // 将无人机浮点坐标映射到帧缓冲坐标
    // drone->position 的范围应在 [0, STAGE_COLS) × [0, STAGE_ROWS)
    int cx = STAGE_LEFT + (int)(drone->position.x + 0.5f);
    int cy = STAGE_TOP  + (int)(drone->position.y + 0.5f);

    if (cx < STAGE_LEFT || cx >= STAGE_LEFT + STAGE_COLS) return;
    if (cy < STAGE_TOP  || cy >= STAGE_TOP  + STAGE_ROWS) return;

    // 用 ● 绘制无人机
    fb_put_wchar(cx, cy, color, L'\x25CF');  // ●
}

void graphics_draw_all_drones(Drone* fleet[], int count, int delta_ms)
{
    if (fleet == NULL) return;

    for (int i = 0; i < count; i++) {
        if (fleet[i] == NULL) continue;
        ConsoleColor c = drone_get_current_color(fleet[i], delta_ms);
        graphics_draw_drone(fleet[i], c);
    }
}

/* ==================== 覆盖层 ==================== */

void graphics_draw_warnings(const SafetyResult* result)
{
    if (result == NULL) return;

    int row = STAGE_TOP + STAGE_ROWS;  // 表演区下方第一行（行42）
    int px  = STAGE_LEFT;

    if (result->boundary_violations > 0) {
        // 越界警告 - 红色背景效果
        fb_put_wchar(px, row, CON_RED, L'\x26A0');  // ⚠
        fb_printf(px + 2, row, CON_RED, "越界警告: ");
        int pos = 0;
        char buf[80] = {0};
        for (int i = 0; i < result->boundary_violations && i < 5; i++) {
            pos += snprintf(buf + pos, sizeof(buf) - pos,
                           "D#%d ", result->boundary_ids[i]);
        }
        if (result->boundary_violations > 5)
            snprintf(buf + pos, sizeof(buf) - pos, "...共%d架", result->boundary_violations);
        fb_printf(px + 16, row, CON_RED, "%s", buf);
        row++;
    }

    if (result->distance_violations > 0) {
        // 碰撞警告 - 黄色
        fb_put_wchar(px, row, CON_YELLOW, L'\x26A0');  // ⚠
        fb_printf(px + 2, row, CON_YELLOW, "碰撞警告: ");
        char buf[80] = {0};
        int pos = 0;
        for (int i = 0; i < result->distance_violations && i < 3; i++) {
            pos += snprintf(buf + pos, sizeof(buf) - pos,
                           "#%d-#%d  ",
                           result->pair_a[i], result->pair_b[i]);
        }
        if (result->distance_violations > 3)
            snprintf(buf + pos, sizeof(buf) - pos, "...共%d对", result->distance_violations);
        fb_printf(px + 16, row, CON_YELLOW, "%s", buf);
    }
}

/* ==================== 顶部标题栏 ==================== */

void graphics_draw_title_bar(SimState state, int elapsed_ms)
{
    int sec = elapsed_ms / 1000;
    int min = sec / 60;
    sec %= 60;

    // 标题栏背景行
    for (int x = 0; x < CONSOLE_WIDTH; x++) {
        fb_put_wchar(x, 0, CON_CYAN, L'\x2500');  // ─ 整条横线
    }

    // 左侧标题
    fb_printf(2, 0, CON_YELLOW, " 无人机编队灯光秀模拟系统 ");

    // 右侧状态信息
    char status_str[40];
    switch (state) {
    case STATE_IDLE:    snprintf(status_str, sizeof(status_str), "[待命]");    break;
    case STATE_RUNNING: snprintf(status_str, sizeof(status_str), "[运行中]");  break;
    case STATE_PAUSED:  snprintf(status_str, sizeof(status_str), "[已暂停]");  break;
    case STATE_REPLAY:  snprintf(status_str, sizeof(status_str), "[回放中]");  break;
    }
    fb_printf(CONSOLE_WIDTH - 30, 0, CON_GREEN, " %02d:%02d %s ", min, sec, status_str);

    // 标题栏底部分隔
    for (int x = 0; x < CONSOLE_WIDTH; x++) {
        fb_put_wchar(x, 1, CON_GRAY, L'\x2500');
    }
}

/* ==================== 底部状态栏 ==================== */

void graphics_draw_bottom_bar(int drone_count, int active_count,
                              const char* hint_text)
{
    int row = CONSOLE_HEIGHT - 1;

    // 底部分隔线（细线）
    for (int x = 0; x < CONSOLE_WIDTH; x++) {
        fb_put_wchar(x, row, CON_GRAY, L'\x2500');
    }

    // 在分隔线上叠加文字
    fb_printf(2, row, CON_WHITE, " 总:%d ", drone_count);

    // 活跃数用绿色
    fb_printf(14, row, CON_GREEN, "活跃:%d ", active_count);

    // 右侧：提示文字
    if (hint_text != NULL) {
        int hint_len = (int)strlen(hint_text);
        fb_puts(CONSOLE_WIDTH - hint_len - 4, row, CON_YELLOW, hint_text);
    }
}

void graphics_draw_panel(Drone* fleet[], int count, SimState state,
                         const Formation* formation, int elapsed_ms,
                         float sim_speed, LightColor light_color,
                         LightMode light_mode, int has_warning)
{
    int px = PANEL_LEFT;
    int py = STAGE_TOP;
    int pw = PANEL_WIDTH;  // 面板内容宽度

    int sec = elapsed_ms / 1000;
    int min = sec / 60;
    sec %= 60;

    // ── 面板顶部边框 ──
    fb_put_wchar(px, py, CON_CYAN, L'\x2554');              // ╔
    for (int i = 1; i < pw - 1; i++)
        fb_put_wchar(px + i, py, CON_CYAN, L'\x2550');      // ═
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2557');     // ╗
    py++;

    // ── 面板标题行 ──
    fb_put_wchar(px, py, CON_CYAN, L'\x2551');              // ║
    fb_printf(px + (pw - 16) / 2, py, CON_YELLOW, "无人机灯光秀模拟");
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');     // ║
    py++;

    // 标题分隔
    fb_put_wchar(px, py, CON_CYAN, L'\x2560');              // ╠
    for (int i = 1; i < pw - 1; i++)
        fb_put_wchar(px + i, py, CON_CYAN, L'\x2550');      // ═
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2563');     // ╣
    py++;

    // ── 状态显示 ──
    fb_put_wchar(px, py, CON_CYAN, L'\x2551');              // ║
    fb_printf(px + 2, py, CON_WHITE, "状态 ");
    // 状态指示器图标
    switch (state) {
    case STATE_IDLE:
        fb_put_wchar(px + 9, py, CON_YELLOW, L'\x25CB');    // ○
        fb_printf(px + 11, py, CON_YELLOW, "待命中...");    break;
    case STATE_RUNNING:
        fb_put_wchar(px + 9, py, CON_GREEN, L'\x25CF');     // ●
        fb_printf(px + 11, py, CON_GREEN, "运行中");        break;
    case STATE_PAUSED:
        fb_put_wchar(px + 9, py, CON_YELLOW, L'\x25AE');    // ▮
        fb_printf(px + 11, py, CON_YELLOW, "已暂停");       break;
    case STATE_REPLAY:
        fb_put_wchar(px + 9, py, CON_CYAN, L'\x25C0');      // ◀
        fb_printf(px + 11, py, CON_CYAN, "回放中");         break;
    }
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');     // ║
    py++;

    // ── 基本参数区 ──
    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 2, py, CON_WHITE, "时间 %02d:%02d", min, sec);
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 2, py, CON_WHITE, "数量 %d 架", count);
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 2, py, CON_WHITE, "速度 %.2fx", (double)sim_speed);
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    // 分隔线
    fb_put_wchar(px, py, CON_CYAN, L'\x2560');
    for (int i = 1; i < pw - 1; i++)
        fb_put_wchar(px + i, py, CON_CYAN, L'\x2500');
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2563');
    py++;

    // ── 编队信息 ──
    if (formation != NULL) {
        fb_put_wchar(px, py, CON_CYAN, L'\x2551');
        fb_printf(px + 2, py, CON_YELLOW, "当前编队");
        fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
        py++;

        fb_put_wchar(px, py, CON_CYAN, L'\x2551');
        fb_printf(px + 2, py, CON_WHITE, "%-20s", formation->name);
        fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
        py++;

        const char* pn = "未知";
        switch (formation->pattern) {
        case PAT_CIRCLE:   pn = "圆形";    break;
        case PAT_SQUARE:   pn = "正方形";  break;
        case PAT_TRIANGLE: pn = "三角形";  break;
        case PAT_DIAMOND:  pn = "菱形";    break;
        case PAT_STAR:     pn = "五角星";  break;
        case PAT_PENTAGON: pn = "五边形";  break;
        case PAT_HEXAGON:  pn = "六边形";  break;
        case PAT_HEART:    pn = "心形";    break;
        case PAT_SPIRAL:   pn = "螺旋";    break;
        case PAT_LINE:     pn = "直线";    break;
        case PAT_ARROW:    pn = "箭头";    break;
        case PAT_CROSS:    pn = "十字";    break;
        case PAT_ARC:      pn = "弧形";    break;
        case PAT_GRID:     pn = "网格";    break;
        case PAT_RANDOM:   pn = "随机散布"; break;
        case PAT_TEXT:     pn = formation->display_text; break;
        default:           break;
        }
        fb_put_wchar(px, py, CON_CYAN, L'\x2551');
        fb_printf(px + 2, py, CON_WHITE, "图案 %s", pn);
        fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
        py++;

        // 分隔线
        fb_put_wchar(px, py, CON_CYAN, L'\x2560');
        for (int i = 1; i < pw - 1; i++)
            fb_put_wchar(px + i, py, CON_CYAN, L'\x2500');
        fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2563');
        py++;
    }

    // ── 灯光设置 ──
    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 2, py, CON_YELLOW, "灯光设置");
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    // 颜色指示器
    const char* cname = "未知";
    ConsoleColor ccol = CON_WHITE;
    switch (light_color) {
    case COLOR_RED:    cname = "红色"; ccol = CON_RED;    break;
    case COLOR_GREEN:  cname = "绿色"; ccol = CON_GREEN;  break;
    case COLOR_BLUE:   cname = "蓝色"; ccol = CON_BLUE;   break;
    case COLOR_WHITE:  cname = "白色"; ccol = CON_WHITE;  break;
    case COLOR_YELLOW: cname = "黄色"; ccol = CON_YELLOW; break;
    case COLOR_CYAN:   cname = "青色"; ccol = CON_CYAN;   break;
    case COLOR_PURPLE: cname = "紫色"; ccol = CON_PURPLE; break;
    case COLOR_ORANGE: cname = "橙色"; ccol = CON_ORANGE; break;
    default:           break;
    }
    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 2, py, CON_WHITE, "颜色 ");
    fb_put_wchar(px + 8, py, ccol, L'\x25CF');  // ● 彩色圆点
    fb_printf(px + 10, py, ccol, "%s", cname);
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    // 模式
    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 2, py, CON_WHITE, "模式 ");
    switch (light_mode) {
    case LIGHT_STEADY:
        fb_printf(px + 8, py, CON_GREEN, "常亮");   break;
    case LIGHT_BLINK:
        fb_printf(px + 8, py, CON_YELLOW, "闪烁");  break;
    case LIGHT_OFF:
        fb_printf(px + 8, py, CON_GRAY, "关闭");    break;
    }
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    // 分隔线
    fb_put_wchar(px, py, CON_CYAN, L'\x2560');
    for (int i = 1; i < pw - 1; i++)
        fb_put_wchar(px + i, py, CON_CYAN, L'\x2500');
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2563');
    py++;

    // ── 安全状态 ──
    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 2, py, CON_YELLOW, "安全状态");
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    if (has_warning) {
        fb_put_wchar(px + 2, py, CON_RED, L'\x26A0');   // ⚠
        fb_printf(px + 4, py, CON_RED, " 发现警告!");
    } else {
        fb_put_wchar(px + 2, py, CON_GREEN, L'\x2714');  // ✔
        fb_printf(px + 4, py, CON_GREEN, " 全部正常");
    }
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    // 分隔线
    fb_put_wchar(px, py, CON_CYAN, L'\x2560');
    for (int i = 1; i < pw - 1; i++)
        fb_put_wchar(px + i, py, CON_CYAN, L'\x2500');
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2563');
    py++;

    // ── 操作键（紧凑两列布局） ──
    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 2, py, CON_YELLOW, "操作键");
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    // 左列
    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 1, py, CON_WHITE, "S    开始  P  暂停");
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 1, py, CON_WHITE, "Q    停止  ESC 退出");
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 1, py, CON_WHITE, "<->  图案  ^v  调速");
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 1, py, CON_WHITE, "C    换色  B  闪烁");
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    fb_put_wchar(px, py, CON_CYAN, L'\x2551');
    fb_printf(px + 1, py, CON_WHITE, "T    文字  H  历史");
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x2551');
    py++;

    // ── 面板底部边框 ──
    fb_put_wchar(px, py, CON_CYAN, L'\x255A');              // ╚
    for (int i = 1; i < pw - 1; i++)
        fb_put_wchar(px + i, py, CON_CYAN, L'\x2550');      // ═
    fb_put_wchar(px + pw - 1, py, CON_CYAN, L'\x255D');     // ╝
}

/* ==================== 欢迎界面 ==================== */

void graphics_show_welcome(void)
{
    fb_clear();

    int cx = CONSOLE_WIDTH / 2;
    int cy = 3;

    // ═══════════ 外框 ═══════════
    // 顶部边框
    fb_put_wchar(1, cy, CON_CYAN, L'\x2554');                // ╔
    for (int i = 0; i < CONSOLE_WIDTH - 4; i++)
        fb_put_wchar(2 + i, cy, CON_CYAN, L'\x2550');        // ═
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2557'); // ╗
    cy++;

    // 空白行（上边距）
    for (int r = 0; r < 2; r++) {
        fb_put_wchar(1, cy, CON_CYAN, L'\x2551');            // ║
        for (int i = 2; i < CONSOLE_WIDTH - 2; i++)
            fb_put_wchar(i, cy, CON_BLACK, L' ');
        fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551'); // ║
        cy++;
    }

    // ── 标题区 ──
    // 装饰性无人机图标
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    fb_printf(cx - 20, cy, CON_YELLOW, "  ✈   ✈   ✈   无人机编队灯光秀模拟系统   ✈   ✈   ✈  ");
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // 英文副标题
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    fb_printf(cx - 16, cy, CON_WHITE, "Drone Formation Light Show Simulator");
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // 空白行
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    for (int i = 2; i < CONSOLE_WIDTH - 2; i++)
        fb_put_wchar(i, cy, CON_BLACK, L' ');
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // 学校信息
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    fb_printf(cx - 20, cy, CON_GREEN, "华中科技大学 · 机械科学与工程学院 · 测控专业");
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    fb_printf(cx - 14, cy, CON_GREEN, "C语言课程设计 · 2025级");
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // 空白行
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    for (int i = 2; i < CONSOLE_WIDTH - 2; i++)
        fb_put_wchar(i, cy, CON_BLACK, L' ');
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // ── 分隔线 ──
    fb_put_wchar(1, cy, CON_CYAN, L'\x2560');                // ╠
    for (int i = 0; i < CONSOLE_WIDTH - 4; i++)
        fb_put_wchar(2 + i, cy, CON_CYAN, L'\x2550');
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2563'); // ╣
    cy++;

    // ── 操作说明（三列布局） ──
    // 列标题
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    fb_printf(5, cy, CON_YELLOW, "模拟控制");
    fb_printf(35, cy, CON_YELLOW, "编队切换");
    fb_printf(65, cy, CON_YELLOW, "灯光效果");
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // 列分隔
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    for (int i = 2; i < CONSOLE_WIDTH - 2; i++)
        fb_put_wchar(i, cy, CON_GRAY, L'\x2500');
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // 操作键行1
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    fb_printf(5, cy, CON_WHITE,  "S / Enter   开始模拟");
    fb_printf(35, cy, CON_WHITE, "← →        切换图案");
    fb_printf(65, cy, CON_WHITE, "C           切换颜色");
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // 操作键行2
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    fb_printf(5, cy, CON_WHITE,  "P / Space   暂停/继续");
    fb_printf(35, cy, CON_WHITE, "T           文字编队");
    fb_printf(65, cy, CON_WHITE, "B           闪烁开关");
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // 操作键行3
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    fb_printf(5, cy, CON_WHITE,  "Q           停止模拟");
    fb_printf(35, cy, CON_WHITE, "H           历史编队");
    fb_printf(65, cy, CON_WHITE, "            8种颜色");
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // 操作键行4
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    fb_printf(5, cy, CON_WHITE,  "↑ ↓        调节速度");
    fb_printf(35, cy, CON_WHITE, "            15种图案");
    fb_printf(65, cy, CON_WHITE, "            常亮/闪烁");
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // 操作键行5
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    fb_printf(5, cy, CON_WHITE,  "ESC         退出程序");
    fb_printf(35, cy, CON_GRAY,  "支持: 圆形/方形/三角/");
    fb_printf(65, cy, CON_GRAY,  "支持: 红/绿/蓝/白/");
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // 补充说明行
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    fb_printf(5, cy, CON_GRAY,  "              ");
    fb_printf(35, cy, CON_GRAY, "菱形/星形/心形/螺旋等");
    fb_printf(65, cy, CON_GRAY, "黄/青/紫/橙");
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // 空白行
    fb_put_wchar(1, cy, CON_CYAN, L'\x2551');
    for (int i = 2; i < CONSOLE_WIDTH - 2; i++)
        fb_put_wchar(i, cy, CON_BLACK, L' ');
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x2551');
    cy++;

    // ── 底部边框 ──
    fb_put_wchar(1, cy, CON_CYAN, L'\x255A');                // ╚
    for (int i = 0; i < CONSOLE_WIDTH - 4; i++)
        fb_put_wchar(2 + i, cy, CON_CYAN, L'\x2550');
    fb_put_wchar(CONSOLE_WIDTH - 2, cy, CON_CYAN, L'\x255D'); // ╝
    cy++;

    // ── 底部提示（闪烁效果用颜色交替模拟） ──
    cy += 2;
    fb_printf(cx - 12, cy, CON_GREEN, "<< 按任意键开始模拟 >>");

    // 选题信息
    cy += 2;
    fb_printf(cx - 18, cy, CON_GRAY, "选题 18: 无人机编队灯光秀模拟 | 指导教师: 周凯波");

    fb_flush();
    _getch();
}

/* ==================== 工具函数 ==================== */

void graphics_gotoxy(int col, int row)
{
    // 帧缓冲模式下此函数无操作（位置由 fb_put_* 的参数决定）
    (void)col; (void)row;
}

void graphics_set_color(ConsoleColor color)
{
    // 帧缓冲模式下此函数无操作（颜色由 fb_put_* 的参数决定）
    (void)color;
}

void graphics_put_str(int col, int row, ConsoleColor color, const char* str)
{
    fb_puts(col, row, color, str);
}
