/*
 * ui.h —— 用户输入模块
 *
 * 用 _kbhit() + _getch() 实现非阻塞按键检测。
 * 每帧在主循环中调用 ui_poll_input()，有按键就返回对应的操作码。
 */

#ifndef UI_H
#define UI_H

#include "common.h"

// 用户操作码 —— ui_poll_input 的返回值
typedef enum {
    UI_CMD_NONE,         // 无操作
    UI_CMD_START,        // 开始模拟
    UI_CMD_PAUSE,        // 暂停/继续
    UI_CMD_STOP,         // 停止
    UI_CMD_SPEED_UP,     // 加速
    UI_CMD_SPEED_DOWN,   // 减速
    UI_CMD_NEXT_PATTERN, // 下一个图案
    UI_CMD_PREV_PATTERN, // 上一个图案
    UI_CMD_CHANGE_COLOR, // 切换灯光颜色
    UI_CMD_TOGGLE_BLINK, // 闪烁开关
    UI_CMD_SAVE,         // 保存轨迹
    UI_CMD_LOAD,         // 加载轨迹
    UI_CMD_REPLAY,       // 回放模式
    UI_CMD_TEXT_INPUT,   // 输入文字编队
    UI_CMD_HISTORY,      // 切换历史编队
    UI_CMD_IMAGE,        // 加载 BMP 图片编队
    UI_CMD_EXIT,         // 退出
} UICmd;

// 检查有没有按键（非阻塞），有就返回对应命令，没有返回 UI_CMD_NONE
UICmd ui_poll_input(void);

// 阻塞等待用户输入一个整数（在 [min_val, max_val] 范围内）
int ui_input_int(const char* prompt, int min_val, int max_val);

// Y/N 确认
int ui_confirm(const char* message);

// 主菜单
int ui_show_menu(void);

#endif
