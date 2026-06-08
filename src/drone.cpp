/**
 * @file    drone.cpp
 * @brief   无人机实体模块实现
 * @author  [你的名字]
 * @date    2026-06-08
 */

#include "../include/drone.h"

/* ==================== 颜色映射表 ==================== */

/**
 * @brief 将 LightColor 枚举值映射为 Windows 控制台颜色属性（WORD）
 *
 * 索引为 LightColor 枚举的值，便于 O(1) 查表。
 * 在 SetConsoleTextAttribute(hConsole, color) 中使用。
 */
static const ConsoleColor g_color_table[] = {
    CON_BLACK,          // COLOR_OFF    —— 黑色（熄灭）
    CON_RED,            // COLOR_RED    —— 红色
    CON_GREEN,          // COLOR_GREEN  —— 绿色
    CON_BLUE,           // COLOR_BLUE   —— 蓝色
    CON_WHITE,          // COLOR_WHITE  —— 白色
    CON_YELLOW,         // COLOR_YELLOW —— 黄色
    CON_CYAN,           // COLOR_CYAN   —— 青色
    CON_PURPLE,         // COLOR_PURPLE —— 紫色
    CON_ORANGE,         // COLOR_ORANGE —— 橙色
};

#define COLOR_TABLE_SIZE (sizeof(g_color_table) / sizeof(g_color_table[0]))

/* ==================== 辅助函数 ==================== */

/**
 * @brief 将 LightColor 转为控制台颜色属性（ConsoleColor）
 * @param color 颜色枚举
 * @return 控制台颜色属性 WORD
 */
static ConsoleColor lightcolor_to_console(LightColor color)
{
    int idx = (int)color;
    if (idx < 0 || idx >= (int)COLOR_TABLE_SIZE) {
        return CON_GRAY;      // 无效颜色 → 灰色兜底
    }
    return g_color_table[idx];
}

/* ==================== 生命周期管理 ==================== */

Drone* drone_create(int id, float x, float y, float height)
{
    // 在堆上分配无人机内存
    Drone* drone = (Drone*)malloc(sizeof(Drone));
    if (drone == NULL) {
        return NULL;    // 内存分配失败
    }

    // 初始化基本属性
    drone->id       = id;
    drone->position.x = x;
    drone->position.y = y;
    drone->height   = height;
    drone->is_active = 1;       // 默认活跃

    // 初始化灯光状态（默认白灯常亮）
    drone->light.color          = COLOR_WHITE;
    drone->light.mode           = LIGHT_STEADY;
    drone->light.blink_interval = BLINK_INTERVAL_MS;
    drone->light.blink_timer    = 0;
    drone->light.is_visible     = 1;        // 初始可见

    return drone;
}

void drone_destroy(Drone* drone)
{
    if (drone != NULL) {
        free(drone);
    }
}

Drone** drone_create_fleet(int count, int start_id)
{
    if (count <= 0 || count > MAX_DRONE_COUNT) {
        return NULL;
    }

    // 为无人机指针数组分配内存
    Drone** fleet = (Drone**)malloc(sizeof(Drone*) * count);
    if (fleet == NULL) {
        return NULL;
    }

    // 逐架创建无人机，初始位置设在地面中央
    for (int i = 0; i < count; i++) {
        fleet[i] = drone_create(
            start_id + i,
            (float)(STAGE_COLS  / 2),   // X 居中
            (float)(STAGE_ROWS / 2),    // Y 居中
            0.0f                         // 初始高度 0
        );
        if (fleet[i] == NULL) {
            // 创建失败则回滚：释放已创建的部分
            for (int j = 0; j < i; j++) {
                drone_destroy(fleet[j]);
            }
            free(fleet);
            return NULL;
        }
    }

    return fleet;
}

void drone_destroy_fleet(Drone** fleet, int count)
{
    if (fleet == NULL) return;

    for (int i = 0; i < count; i++) {
        drone_destroy(fleet[i]);
    }
    free(fleet);
}

/* ==================== 位置操作 ==================== */

void drone_set_position(Drone* drone, float x, float y, float height)
{
    if (drone == NULL) return;

    drone->position.x = x;
    drone->position.y = y;
    drone->height     = height;
}

void drone_move(Drone* drone, float dx, float dy, float dh)
{
    if (drone == NULL) return;

    drone->position.x += dx;
    drone->position.y += dy;
    drone->height     += dh;

    // 高度不允许为负
    if (drone->height < 0.0f) {
        drone->height = 0.0f;
    }
}

void drone_get_display_pos(const Drone* drone, int* x, int* y)
{
    if (drone == NULL || x == NULL || y == NULL) return;

    // 浮点坐标四舍五入转整数（用于屏幕显示）
    *x = (int)(drone->position.x + 0.5f);
    *y = (int)(drone->position.y + 0.5f);
}

/* ==================== 灯光操作 ==================== */

void drone_set_light_color(Drone* drone, LightColor color)
{
    if (drone == NULL) return;
    drone->light.color = color;
}

void drone_set_light_mode(Drone* drone, LightMode mode)
{
    if (drone == NULL) return;

    drone->light.mode = mode;

    // 模式变更时重置闪烁计时器和可见性
    if (mode == LIGHT_STEADY) {
        drone->light.is_visible  = 1;
        drone->light.blink_timer = 0;
    } else if (mode == LIGHT_BLINK) {
        drone->light.is_visible  = 1;      // 闪烁从"亮"开始
        drone->light.blink_timer = 0;
    } else {
        // LIGHT_OFF
        drone->light.is_visible  = 0;
        drone->light.blink_timer = 0;
    }
}

void drone_set_blink_interval(Drone* drone, int interval_ms)
{
    if (drone == NULL) return;

    // 闪烁间隔不小于 50ms（太快无意义）
    if (interval_ms < 50) {
        interval_ms = 50;
    }
    drone->light.blink_interval = interval_ms;
}

void drone_light_onoff(Drone* drone, int on)
{
    if (drone == NULL) return;

    if (on) {
        drone->light.mode = LIGHT_STEADY;
        drone->light.is_visible = 1;
    } else {
        drone->light.mode = LIGHT_OFF;
        drone->light.is_visible = 0;
    }
}

/* ==================== 状态查询 ==================== */

int drone_is_active(const Drone* drone)
{
    if (drone == NULL) return 0;
    return drone->is_active;
}

ConsoleColor drone_get_current_color(Drone* drone, int delta_ms)
{
    if (drone == NULL) return CON_BLACK;

    // 根据灯光模式处理闪烁逻辑
    switch (drone->light.mode) {
    case LIGHT_OFF:
        // 灯光关闭 → 始终返回黑色
        return CON_BLACK;

    case LIGHT_STEADY:
        // 常亮 → 直接返回颜色表中的颜色
        return lightcolor_to_console(drone->light.color);

    case LIGHT_BLINK:
        // 闪烁模式：累加计时器，周期性切换亮/灭
        drone->light.blink_timer += delta_ms;

        if (drone->light.blink_timer >= drone->light.blink_interval) {
            // 满一个周期：切换到相反状态
            drone->light.is_visible = !drone->light.is_visible;
            drone->light.blink_timer -= drone->light.blink_interval;
        }

        // 根据当前可见性返回颜色或黑色
        if (drone->light.is_visible) {
            return lightcolor_to_console(drone->light.color);
        } else {
            return CON_BLACK;
        }

    default:
        return CON_BLACK;
    }
}

/* ==================== 工具函数：颜色名称获取 ==================== */

/**
 * @brief 根据颜色枚举返回对应的中文字符串（供 UI 显示用）
 * @param color 颜色枚举
 * @return 中文字符串指针（静态常量）
 */
const char* color_to_name(LightColor color)
{
    switch (color) {
    case COLOR_RED:    return "红色";
    case COLOR_GREEN:  return "绿色";
    case COLOR_BLUE:   return "蓝色";
    case COLOR_WHITE:  return "白色";
    case COLOR_YELLOW: return "黄色";
    case COLOR_CYAN:   return "青色";
    case COLOR_PURPLE: return "紫色";
    case COLOR_ORANGE: return "橙色";
    default:           return "未知";
    }
}
