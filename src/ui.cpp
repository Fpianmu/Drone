/*
 * ui.cpp —— 键盘 + 鼠标输入模块
 *
 * 键盘：_kbhit() + _getch() 非阻塞检测
 * 鼠标：ReadConsoleInput 读 MOUSE_EVENT，记录点击坐标
 */
#include "../include/ui.h"

#define KEY_ESC   27
#define KEY_ENTER 13
#define KEY_SPACE 32
#define KEY_UP    72
#define KEY_DOWN  80
#define KEY_LEFT  75
#define KEY_RIGHT 77

MouseClick g_mouse_click = {0, 0, 0};

// 检查鼠标事件，有左键/右键点击时填入 g_mouse_click 并返回 UI_CMD_MOUSE_CLICK
static UICmd poll_mouse(void)
{
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD events;
    INPUT_RECORD rec;
    // 只看队列第一个事件，仅当是鼠标事件时才读取，键盘事件留给 _kbhit
    if (PeekConsoleInputW(hIn, &rec, 1, &events) && events > 0
        && rec.EventType == MOUSE_EVENT) {
        ReadConsoleInputW(hIn, &rec, 1, &events);
        MOUSE_EVENT_RECORD* me = &rec.Event.MouseEvent;
        if (me->dwEventFlags == 0) {
            g_mouse_click.col   = me->dwMousePosition.X;
            g_mouse_click.row   = me->dwMousePosition.Y;
            g_mouse_click.button = (me->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) ? 1
                                : (me->dwButtonState & RIGHTMOST_BUTTON_PRESSED) ? 2 : 0;
            if (g_mouse_click.button > 0)
                return UI_CMD_MOUSE_CLICK;
        }
    }
    return UI_CMD_NONE;
}

UICmd ui_poll_input(void)
{
    // 键盘优先
    if (_kbhit()) {
        int ch = _getch();
        if (ch == 0xE0 || ch == 0x00) {
            ch = _getch();
            switch (ch) {
            case KEY_UP:    return UI_CMD_SPEED_UP;
            case KEY_DOWN:  return UI_CMD_SPEED_DOWN;
            case KEY_LEFT:  return UI_CMD_PREV_PATTERN;
            case KEY_RIGHT: return UI_CMD_NEXT_PATTERN;
            default:        return UI_CMD_NONE;
            }
        }
        switch (ch) {
        case 's': case 'S':     return UI_CMD_START;
        case 'p': case 'P':     return UI_CMD_PAUSE;
        case 'q': case 'Q':     return UI_CMD_STOP;
        case 'c': case 'C':     return UI_CMD_CHANGE_COLOR;
        case 'b': case 'B':     return UI_CMD_TOGGLE_BLINK;
        case 't': case 'T':     return UI_CMD_TEXT_INPUT;
        case 'h': case 'H':     return UI_CMD_HISTORY;
        case 'i': case 'I':     return UI_CMD_IMAGE;
        case 'e': case 'E':     return UI_CMD_FX_NEXT;
        case KEY_ESC:           return UI_CMD_EXIT;
        case KEY_ENTER:         return UI_CMD_START;
        case KEY_SPACE:         return UI_CMD_PAUSE;
        default:                return UI_CMD_NONE;
        }
    }

    // 没键盘输入时才检查鼠标
    return poll_mouse();
}

int ui_input_int(const char* prompt, int min_val, int max_val)
{
    int value; char buf[32];
    while (1) {
        printf("\n%s (%d-%d): ", prompt, min_val, max_val);
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) return min_val;
        if (sscanf(buf, "%d", &value) == 1 && value >= min_val && value <= max_val)
            return value;
        printf("Invalid input!\n");
    }
}

int ui_confirm(const char* message)
{
    printf("\n%s (Y/N): ", message); fflush(stdout);
    int ch = _getch(); printf("%c\n", ch);
    return (ch == 'y' || ch == 'Y') ? 1 : 0;
}

int ui_show_menu(void)
{
    system("cls");
    printf("\n  ======================================\n");
    printf("  Drone Formation Light Show Simulator\n");
    printf("  ======================================\n\n");
    printf("  [1] Start\n  [2] Load Replay\n  [3] Set Drones\n  [4] Help\n  [5] Exit\n");
    printf("\n  Select (1-5): "); fflush(stdout);
    int ch = _getch(); printf("%c\n\n", ch);
    return (ch >= '1' && ch <= '5') ? ch - '1' : 0;
}
