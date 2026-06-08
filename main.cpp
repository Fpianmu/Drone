/**
 * @file    main.cpp
 * @brief   无人机编队灯光秀模拟系统 —— 程序入口
 *
 * @note    华中科技大学 机械科学与工程学院
 *          测控专业 2025 级 C 语言课程设计
 *          选题：18. 无人机编队灯光秀模拟
 *
 * @author  [你的名字]     —— 核心仿真引擎（drone, formation, trajectory, safety, light）
 * @author  [队友名字]     —— 图形渲染与交互（graphics, ui, file_io, controller）
 * @date    2026-06-08
 *
 * 编译环境：GCC / MinGW-w64（无需额外图形库）
 * 编译命令：g++ -std=c++11 -o drone_show.exe main.cpp src/drone.cpp src/light.cpp
 *           src/formation.cpp src/trajectory.cpp src/safety.cpp
 *           src/graphics.cpp src/ui.cpp src/file_io.cpp src/controller.cpp -I include
 * 渲染方案：Windows 控制台字符画（帧缓冲 + WriteConsoleOutputW）
 *
 * 项目结构：
 *   include/    —— 公共头文件
 *   src/        —— 源文件实现
 *   main.cpp    —— 程序入口
 *
 * 文档参考：
 *   commands.txt —— 课程设计任务书
 */

#include "include/controller.h"

/**
 * @brief 程序入口函数
 *
 * 流程：
 *   1. 初始化随机数种子
 *   2. 创建控制器（含所有子系统初始化）
 *   3. 显示欢迎界面 → 进入主循环
 *   4. 退出时清理资源
 */
int main(void)
{
    // 初始化随机数种子（供图案生成器使用）
    srand((unsigned int)time(NULL));

    // 创建控制器（内部自动创建无人机编队、安全区域、默认编队）
    Controller* ctrl = controller_create();
    if (ctrl == NULL) {
        printf("ERROR: 控制器初始化失败！\n");
        printf("请检查控制台窗口是否正常。\n");
        printf("按任意键退出...\n");
        _getch();
        return 1;
    }

    // 进入主循环（欢迎界面 → 事件循环 → 清理）
    controller_run(ctrl);

    // 清理所有资源
    controller_destroy(ctrl);

    return 0;
}
