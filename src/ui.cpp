/**
 * @file    ui.cpp
 * @brief   用户界面交互模块实现 —— 键盘输入处理
 * @author  [队友名字]
 * @date    2026-06-08
 *
 * 使用 _kbhit() + _getch() 实现非阻塞键盘输入检测。
 * 按键映射：
 *   S / s     → 开始模拟
 *   P / p     → 暂停/继续
 *   Q / q     → 停止
 *   ← / →     → 切换图案
 *   ↑ / ↓     → 调节速度
 *   C / c     → 切换灯光颜色
 *   B / b     → 切换闪烁模式
 *   ESC       → 退出
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
    // 简化菜单：控制台直接打印选项并等待按键
    system("cls");
    printf("\n");
    printf("  ╔══════════════════════════════════════╗\n");
    printf("  ║     无人机编队灯光秀模拟系统         ║\n");
    printf("  ╠══════════════════════════════════════╣\n");
    printf("  ║                                      ║\n");
    printf("  ║  [1] 开始新模拟                      ║\n");
    printf("  ║  [2] 加载回放文件                    ║\n");
    printf("  ║  [3] 设置无人机数量                  ║\n");
    printf("  ║  [4] 帮助                            ║\n");
    printf("  ║  [5] 退出                            ║\n");
    printf("  ║                                      ║\n");
    printf("  ╚══════════════════════════════════════╝\n");
    printf("\n  请选择 (1-5): ");
    fflush(stdout);

    int ch = _getch();
    printf("%c\n\n", ch);

    if (ch >= '1' && ch <= '5') {
        return ch - '1';  // 返回 0~4
    }
    return 0;  // 默认选项0
}
