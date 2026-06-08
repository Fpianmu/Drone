/**
 * @file    graphics.cpp
 * @brief   控制台字符画渲染 —— 内存帧缓冲方案（零闪烁，零残影）
 * @author  [队友名字]
 * @date    2026-06-08
 *
 * 架构：
 *   1. 维护一个 CHAR_INFO 二维帧缓冲（120×45），完全脱离控制台绘制
 *   2. 所有绘制操作只修改帧缓冲中的字符和颜色
 *   3. 每帧结束时，用 WriteConsoleOutputW 一次性将整个帧缓冲写入控制台
 *   4. 不存在双缓冲切换、不存在 printf 指向错误 —— 一个 WriteConsoleOutput 搞定一切
 *
 * 为什么之前有白线：
 *   printf 写 stdout → 写到原始缓冲区 → 双缓冲交换后两个缓冲区内容错乱
 *   帧缓冲方案完全绕过了这个问题：统一在内存中绘制，一次输出。
 */

#include "../include/graphics.h"

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

    // 四个角
    fb_put_wchar(x0, y0, CON_WHITE, L'\x250C');   // ┌
    fb_put_wchar(x1, y0, CON_WHITE, L'\x2510');   // ┐
    fb_put_wchar(x0, y1, CON_WHITE, L'\x2514');   // └
    fb_put_wchar(x1, y1, CON_WHITE, L'\x2518');   // ┘

    // 横线
    for (int x = x0 + 1; x < x1; x++) {
        fb_put_wchar(x, y0, CON_WHITE, L'\x2500');  // ─
        fb_put_wchar(x, y1, CON_WHITE, L'\x2500');
    }

    // 竖线
    for (int y = y0 + 1; y < y1; y++) {
        fb_put_wchar(x0, y, CON_WHITE, L'\x2502');  // │
        fb_put_wchar(x1, y, CON_WHITE, L'\x2502');
    }

    // 舞台内部画淡色网格点（每5格一个·），帮助定位
    for (int y = y0 + 5; y < y1; y += 5) {
        for (int x = x0 + 5; x < x1; x += 5) {
            fb_put_wchar(x, y, CON_GRAY, L'\x00B7');  // ·
        }
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

    int row = STAGE_TOP + STAGE_ROWS + 1;

    if (result->boundary_violations > 0) {
        char buf[128] = "!! [越界] ";
        int  pos = (int)strlen(buf);
        for (int i = 0; i < result->boundary_violations && i < 5; i++) {
            pos += snprintf(buf + pos, sizeof(buf) - pos,
                           "#%d ", result->boundary_ids[i]);
        }
        fb_puts(STAGE_LEFT, row, CON_RED, buf);
        row++;
    }

    if (result->distance_violations > 0) {
        char buf[128] = "!! [碰撞] ";
        int  pos = (int)strlen(buf);
        for (int i = 0; i < result->distance_violations && i < 3; i++) {
            pos += snprintf(buf + pos, sizeof(buf) - pos,
                           "#%d-#%d  ",
                           result->pair_a[i], result->pair_b[i]);
        }
        fb_puts(STAGE_LEFT, row, CON_YELLOW, buf);
    }
}

void graphics_draw_panel(Drone* fleet[], int count, SimState state,
                         const Formation* formation, int elapsed_ms)
{
    int px = PANEL_LEFT;
    int py = STAGE_TOP + 1;
    int sec = elapsed_ms / 1000;
    int min = sec / 60;
    sec %= 60;

    // 标题
    fb_puts(px, py, CON_CYAN, "无人机灯光秀模拟"); py++;
    fb_puts(px, py, CON_CYAN, "==================");  py += 2;

    // 状态
    fb_printf(px, py, CON_WHITE, "状态: ");  // 先写标签
    switch (state) {
    case STATE_IDLE:    fb_printf(px + 7, py, CON_GREEN, "待命中...");       break;
    case STATE_RUNNING: fb_printf(px + 7, py, CON_GREEN, ">>>> 运行中 <<<<");  break;
    case STATE_PAUSED:  fb_printf(px + 7, py, CON_GREEN, "|| 已暂停");       break;
    case STATE_REPLAY:  fb_printf(px + 7, py, CON_GREEN, "<< 回放中 >>");    break;
    }
    py += 2;

    fb_printf(px, py, CON_WHITE, "时间: %02d:%02d", min, sec); py++;
    fb_printf(px, py, CON_WHITE, "无人机: %d 架", count);      py++;
    fb_printf(px, py, CON_WHITE, "速度: 1.0x");                  py += 2;

    // 编队
    if (formation != NULL) {
        fb_puts(px, py, CON_CYAN, "当前编队"); py++;
        fb_puts(px, py, CON_WHITE, formation->name); py += 2;

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
        fb_printf(px, py, CON_WHITE, "图案: %s", pn); py += 2;
    }

    // 操作键
    fb_puts(px, py, CON_YELLOW, "── 操作键 ──");          py++;
    fb_puts(px, py, CON_WHITE,  "S      开始模拟");        py++;
    fb_puts(px, py, CON_WHITE,  "P      暂停/继续");       py++;
    fb_puts(px, py, CON_WHITE,  "Q      停止");             py++;
    fb_puts(px, py, CON_WHITE,  "< >    切换图案(15种)");  py++;
    fb_puts(px, py, CON_WHITE,  "^ v    调节速度");        py++;
    fb_puts(px, py, CON_WHITE,  "C      切换灯光颜色");    py++;
    fb_puts(px, py, CON_WHITE,  "B      闪烁开关");        py++;
    fb_puts(px, py, CON_WHITE,  "T      输入文字编队");    py++;
    fb_puts(px, py, CON_WHITE,  "H      历史编队(5次)");  py++;
    fb_puts(px, py, CON_WHITE,  "ESC    退出");
}

/* ==================== 欢迎界面 ==================== */

void graphics_show_welcome(void)
{
    fb_clear();

    int cx = CONSOLE_WIDTH / 2;
    int cy = 7;

    // 标题
    fb_put_wchar(cx - 16, cy, CON_CYAN, L'\x2554');  // ╔
    for (int i = 0; i < 32; i++)
        fb_put_wchar(cx - 15 + i, cy, CON_CYAN, L'\x2550');  // ═
    fb_put_wchar(cx + 16, cy, CON_CYAN, L'\x2557');  // ╗
    cy++;

    fb_put_wchar(cx - 16, cy, CON_CYAN, L'\x2551');  // ║
    fb_printf(cx - 4, cy, CON_YELLOW, "无人机编队灯光秀模拟系统");
    fb_put_wchar(cx + 16, cy, CON_CYAN, L'\x2551');  // ║
    cy++;

    fb_put_wchar(cx - 16, cy, CON_CYAN, L'\x255A');  // ╚
    for (int i = 0; i < 32; i++)
        fb_put_wchar(cx - 15 + i, cy, CON_CYAN, L'\x2550');  // ═
    fb_put_wchar(cx + 16, cy, CON_CYAN, L'\x255D');  // ╝
    cy += 3;

    // 操作说明
    static const char* help[] = {
        "  S      开始模拟",
        "  P      暂停/继续",
        "  Q      停止",
        "  <- ->  切换图案（圆形/方形/三角/菱形/星形/心形...共15种）",
        "  ^  v   加快/减慢飞行速度",
        "  C      切换灯光颜色（红/绿/蓝/白/黄/青/紫/橙）",
        "  B      闪烁模式开关",
        "  T      输入文字编队（英文/中文，≤5字）",
        "  ESC    退出",
    };
    for (int i = 0; i < 9; i++) {
        fb_puts(cx - 26, cy + i, CON_WHITE, help[i]);
    }

    fb_printf(cx - 12, cy + 12, CON_GREEN, "按任意键开始...");

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
