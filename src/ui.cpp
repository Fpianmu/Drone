/*
 * ui.cpp —— 用户输入模块实现
 *
 * 用 _kbhit() 检查是否有按键（非阻塞），有则用 _getch() 读取。
 * 方向键等扩展键发送两个字节（0xE0/0x00 + 键码），需要读两次。
 */


#include "../include/ui.h"

/* ==================== 按键映射常量 ==================== */

#define KEY_ESC      27
#define KEY_ENTER    13
#define KEY_SPACE    32
#define KEY_UP       72    // 方向键上（扩展键）
#define KEY_DOWN     80    // 方向键下
#define KEY_LEFT     75    // 方向键左
#define KEY_RIGHT    77    // 方向键右

/* ==================== 输入处理 ==================== */

UICmd ui_poll_input(void)
{
    // 无输入直接返回
    if (!_kbhit()) {
        return UI_CMD_NONE;
    }

    int ch = _getch();

    // 处理扩展键（方向键等发送两个字节：0xE0 或 0x00 + 键码）
    if (ch == 0xE0 || ch == 0x00) {
        ch = _getch();  // 读取实际键码

        switch (ch) {
        case KEY_UP:    return UI_CMD_SPEED_UP;
        case KEY_DOWN:  return UI_CMD_SPEED_DOWN;
        case KEY_LEFT:  return UI_CMD_PREV_PATTERN;
        case KEY_RIGHT: return UI_CMD_NEXT_PATTERN;
        default:        return UI_CMD_NONE;
        }
    }

    // 处理普通按键（大小写不敏感）
    switch (ch) {
    case 's': case 'S':     return UI_CMD_START;
    case 'p': case 'P':     return UI_CMD_PAUSE;
    case 'q': case 'Q':     return UI_CMD_STOP;
    case 'c': case 'C':     return UI_CMD_CHANGE_COLOR;
    case 'b': case 'B':     return UI_CMD_TOGGLE_BLINK;
    case 't': case 'T':     return UI_CMD_TEXT_INPUT;
    case 'h': case 'H':     return UI_CMD_HISTORY;

    case KEY_ESC:           return UI_CMD_EXIT;
    case KEY_ENTER:         return UI_CMD_START;   // 回车 = 开始
    case KEY_SPACE:         return UI_CMD_PAUSE;   // 空格 = 暂停

    default:                return UI_CMD_NONE;
    }
}

/* ==================== 输入对话框 ==================== */

int ui_input_int(const char* prompt, int min_val, int max_val)
{
    int value;
    char buffer[32];

    while (1) {
        printf("\n%s (%d-%d): ", prompt, min_val, max_val);
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            return min_val;  // 读取失败 → 返回最小值
        }

        if (sscanf(buffer, "%d", &value) == 1) {
            if (value >= min_val && value <= max_val) {
                return value;
            }
        }

        printf("输入无效，请重新输入！\n");
    }
}

int ui_confirm(const char* message)
{
    printf("\n%s (Y/N): ", message);
    fflush(stdout);

    int ch = _getch();
    printf("%c\n", ch);

    return (ch == 'y' || ch == 'Y') ? 1 : 0;
}

int ui_show_menu(void)
{
    /* 自包含菜单：设置 UTF-8 代码页，使用标准 printf 输出 */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    /* ANSI 清屏（Windows 10+ 支持） */
    printf("\033[2J\033[H");

    /* 使用 UTF-8 编码的框线字符绘制菜单 */
    printf("\n");
    printf("  \xe2\x95\x94"); for (int i = 0; i < 38; i++) printf("\xe2\x95\x90");
    printf("\xe2\x95\x97\n");

    printf("  \xe2\x95\x91                                      \xe2\x95\x91\n");

    printf("  \xe2\x95\x91   \xe6\x97\xa0\xe4\xba\xba\xe6\x9c\xba\xe7\xbc\x96\xe9\x98\x9f"
           "\xe7\x81\xaf\xe5\x85\x89\xe7\xa7\x80\xe6\xa8\xa1\xe6\x8b\x9f\xe7\xb3\xbb\xe7\xbb\x9f"
           "      \xe2\x95\x91\n");

    printf("  \xe2\x95\x91                                      \xe2\x95\x91\n");
    printf("  \xe2\x95\xa0"); for (int i = 0; i < 38; i++) printf("\xe2\x95\x90");
    printf("\xe2\x95\xa3\n");

    printf("  \xe2\x95\x91                                      \xe2\x95\x91\n");
    printf("  \xe2\x95\x91  [1] \xe5\xbc\x80\xe5\xa7\x8b\xe6\x96\xb0\xe6\xa8\xa1\xe6\x8b\x9f"
           "                      \xe2\x95\x91\n");
    printf("  \xe2\x95\x91  [2] \xe5\x8a\xa0\xe8\xbd\xbd\xe5\x9b\x9e\xe6\x94\xbe\xe6\x96\x87\xe4\xbb\xb6"
           "                    \xe2\x95\x91\n");
    printf("  \xe2\x95\x91  [3] \xe8\xae\xbe\xe7\xbd\xae\xe6\x97\xa0\xe4\xba\xba\xe6\x9c\xba\xe6\x95\xb0"
           "\xe9\x87\x8f                  \xe2\x95\x91\n");
    printf("  \xe2\x95\x91  [4] \xe5\xb8\xae\xe5\x8a\xa9"
           "                            \xe2\x95\x91\n");
    printf("  \xe2\x95\x91  [5] \xe9\x80\x80\xe5\x87\xba"
           "                            \xe2\x95\x91\n");
    printf("  \xe2\x95\x91                                      \xe2\x95\x91\n");

    printf("  \xe2\x95\x9a"); for (int i = 0; i < 38; i++) printf("\xe2\x95\x90");
    printf("\xe2\x95\x9d\n");

    printf("\n  \xe8\xaf\xb7\xe9\x80\x89\xe6\x8b\xa9 (1-5): ");
    fflush(stdout);

    int ch = _getch();
    printf("%c\n\n", ch);

    if (ch >= '1' && ch <= '5') {
        return ch - '1';  // 返回 0~4
    }
    return 0;  // 默认选项0
}
