/**
 * @file    ui.h
 * @brief   用户界面交互模块 —— 键盘/鼠标输入处理
 * @author  [队友名字]
 * @date    2026-06-08
 *
 * 本模块处理所有用户输入，并映射为系统操作：
 * - 键盘快捷键（菜单导航、启停控制、速度调节）
 * - 菜单系统（文字菜单选择）
 * - 对话框/提示确认
 *
 * 模块不持有状态，返回操作码供 Controller 处理。
 */

#ifndef UI_H
#define UI_H

#include "common.h"

/* ==================== 用户操作码枚举 ==================== */

/** @brief 用户操作枚举 —— UI 层返回的操作指令 */
typedef enum {
    UI_CMD_NONE,            // 无操作
    UI_CMD_START,           // 开始模拟
    UI_CMD_PAUSE,           // 暂停/继续
    UI_CMD_STOP,            // 停止模拟
    UI_CMD_SPEED_UP,        // 加速
    UI_CMD_SPEED_DOWN,      // 减速
    UI_CMD_NEXT_PATTERN,    // 切换下一个图案
    UI_CMD_PREV_PATTERN,    // 切换上一个图案
    UI_CMD_CHANGE_COLOR,    // 切换灯光颜色
    UI_CMD_TOGGLE_BLINK,    // 开关闪烁模式
    UI_CMD_SAVE,            // 保存轨迹
    UI_CMD_LOAD,            // 加载轨迹
    UI_CMD_REPLAY,          // 回放模式
    UI_CMD_TEXT_INPUT,      // 输入文字编队
    UI_CMD_HISTORY,         // 切换历史编队
    UI_CMD_EXIT,            // 退出程序
} UICmd;

/* ==================== 输入处理 ==================== */

/**
 * @brief 检查用户输入（非阻塞）
 *
 * 每帧调用，用 _kbhit() 检测键盘输入。
 * 返回对应的操作码；无输入时返回 UI_CMD_NONE。
 *
 * @return 用户触发的操作码
 */
UICmd ui_poll_input(void);

/**
 * @brief 获取用户输入的整数
 *
 * 用于设置无人机数量等需要用户输入数值的场景。
 * 阻塞等待直到用户输入有效值。
 *
 * @param prompt   提示文字
 * @param min_val  最小值
 * @param max_val  最大值
 * @return 用户输入的整数值
 */
int ui_input_int(const char* prompt, int min_val, int max_val);

/**
 * @brief 显示确认对话框
 * @param message 提示信息
 * @return 1=确认, 0=取消
 */
int ui_confirm(const char* message);

/**
 * @brief 绘制主菜单并获取选择项
 * @return 选中的菜单索引（0开始）
 */
int ui_show_menu(void);

#endif // UI_H
